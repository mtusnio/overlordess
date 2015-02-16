//==============Overlord mod=====================
//	Path node base class
//===============================================

#include "cbase.h"
#include "overlord_pathnode.h"
#include "team.h"
#include "hl2mp_gamerules.h"
#include "sprite.h"

ConVar eo_can_spawn_time("eo_can_spawn_time", "12", FCVAR_REPLICATED | FCVAR_CHEAT);

CHandle<COverlordPathNode> COverlordPathNode::m_FirstNode = NULL;
CHandle<COverlordPathNode> COverlordPathNode::m_LastNode = NULL;
CHandle<COverlordPathNode> COverlordPathNode::m_SpawnNode = NULL;

COverlordPathNode * COverlordPathNode::GetMostPopulatedNode()
{
	COverlordPathNode * pLast = GetLastNode();

	if(!pLast)
		return NULL;

	COverlordPathNode * pPopulated = NULL;
	while(!pLast->IsSpawnNode())
	{
		if(!pPopulated || (pLast->GetPlayerCount() > pPopulated->GetPlayerCount()))	
		{
			pPopulated = pLast;
		}


		pLast = pLast->GetPreviousNode();

		if(!pLast)
			return pPopulated;
	}

	return pPopulated;
}
//==================================== Path node ====================================

BEGIN_DATADESC(COverlordPathNode)

DEFINE_KEYFIELD(m_NextLinkName, FIELD_STRING, "NextKey"),
DEFINE_KEYFIELD(m_AltLinkName, FIELD_STRING, "AltKey"),
DEFINE_KEYFIELD(m_flReachDistance, FIELD_FLOAT, "ReachDistance"),
DEFINE_KEYFIELD(m_iSpawnDistance, FIELD_INTEGER, "SpawnDistance"),
DEFINE_KEYFIELD(m_szHint, FIELD_STRING, "HintMessage"),
DEFINE_OUTPUT(m_OnPlayerReached, "OnPlayerReached"),
DEFINE_OUTPUT(m_OnFirstPlayerReached, "OnFirstPlayerReached"),
DEFINE_INPUTFUNC( FIELD_STRING, "SetNextNode", InputSetNextNode),
DEFINE_INPUTFUNC( FIELD_VOID, "SetAsReached", InputSetAsReached),

END_DATADESC()

IMPLEMENT_NETWORKCLASS(COverlordPathNode, DT_OverlordPathNode);

BEGIN_SEND_TABLE(COverlordPathNode, DT_OverlordPathNode)

SendPropEHandle(SENDINFO(m_NextNode)),
SendPropEHandle(SENDINFO(m_PrevNode)),
SendPropEHandle(SENDINFO(m_AltNode)),
SendPropFloat(SENDINFO(m_PathLength)),
SendPropBool(SENDINFO(m_bReached)),
SendPropBool(SENDINFO(m_bIsAltNode)),
SendPropInt(SENDINFO(m_spawnflags)),

END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(overlord_pathnode, COverlordPathNode);

COverlordPathNode::COverlordPathNode()
{
	m_PathLength = 0.0f;
	m_bInitialized = false;
	m_bIsAltNode = false;
	m_NextNode = NULL;
	m_PrevNode = NULL;
	m_bMarkedByInput = false;
}

COverlordPathNode::~COverlordPathNode()
{
}

void COverlordPathNode::Precache()
{
	PrecacheModel("models/node/node.mdl");
}

void COverlordPathNode::Spawn()
{
	Precache();

	SetModel("models/node/node.mdl");

	SetSolid(SOLID_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	AddEffects(EF_NOSHADOW);

	SetNextThink(TICK_NEVER_THINK);

	SetRenderMode(kRenderTransColor);
}

void COverlordPathNode::Think()
{
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(pRebels && CanReachThisNode())
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || !pPlayer->IsAlive())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= m_flReachDistance)
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->RealEyePosition(), (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_GRATE), this, COLLISION_GROUP_NONE, &tr);

				if((tr.fraction == 1.0f) || (tr.m_pEnt == pPlayer))
				{
					if(!WasReached())
					{
						COverlordPathNode * pLast = GetLastNode();

						SetAsReached(pPlayer);

						if(pLast)
							pLast->RemoveFromChain(pPlayer);
					}
					else
					{
						// He went back, remove from all forward nodes
						COverlordPathNode * pLast = GetLastNode();

						if(pLast)
							pLast->RemoveFromForwardChain(pPlayer, this);
					}
					AddToList(pPlayer);
				}
			}
		}
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

bool COverlordPathNode::CanReachThisNode() const
{
	if(m_spawnflags & SF_CAN_ALWAYS_REACH)
		return true;

	if(WasReached())
	{
		/*if(IsSpawnNode())
			return true;

		if(!m_PrevNode)
			return true;*/
		return true;
	}

	if(!WasReached() && CanOnlyMarkOnInput())
		return false;

	if(!m_PrevNode)
	{
		return true;
	}
	return m_PrevNode->CanReachThisNode();
}

void COverlordPathNode::InitializeNode(COverlordPathNode * prev, float length)
{
	m_PathLength = length;
	m_PrevNode = prev;
	
	if(!(m_spawnflags & SF_MARK_ON_INPUT))
	{
		SetThink(&COverlordPathNode::Think);
		SetNextThink(gpGlobals->curtime + 0.25f);
	}
	else
		SetNextThink(TICK_NEVER_THINK);

	if(m_NextLinkName)
	{
		CBaseEntity * pEnt = gEntList.FindEntityByName(NULL, m_NextLinkName);

		if(pEnt)
		{
			m_NextNode = static_cast<COverlordPathNode*>(pEnt);	

			float newLength = length + (GetAbsOrigin() - m_NextNode->GetAbsOrigin()).Length();
			m_NextNode->InitializeNode(this, newLength);
			m_bInitialized = true;
		}
	}

	if(m_AltLinkName)
	{
		CBaseEntity * pEnt = gEntList.FindEntityByName(NULL, m_AltLinkName);

		if(pEnt)
		{
			m_AltNode = static_cast<COverlordPathNode*>(pEnt);	
			float newLength = length + (GetAbsOrigin() - m_AltNode->GetAbsOrigin()).Length();
			m_AltNode->InitializeAltNode(this, newLength);
		}
	}
}

void COverlordPathNode::InitializeAltNode(COverlordPathNode * prev, float length)
{
	// We ran into a non-alt node, stop right here...
	if(WasInited())
		return;

	m_bIsAltNode = true;
	m_PathLength = length;
	m_PrevNode = prev;
	
	if(m_NextLinkName)
	{
		CBaseEntity * pEnt = gEntList.FindEntityByName(NULL, m_NextLinkName);

		if(pEnt)
		{
			m_NextNode = static_cast<COverlordPathNode*>(pEnt);	

			float newLength = length + (GetAbsOrigin() - m_NextNode->GetAbsOrigin()).Length();
			m_NextNode->InitializeAltNode(this, newLength);
			m_bInitialized = true;
		}
	}
}

void COverlordPathNode::RemoveFromForwardChain(CBasePlayer * pPlayer, COverlordPathNode * pNode /* = NULL */)
{
	if(!pPlayer)
		return;

	if(pNode == this)
		return;

	if(IsSpawnNode())
		return;

	RemoveFromList(pPlayer);

	if(m_PrevNode)
		m_PrevNode->RemoveFromForwardChain(pPlayer, pNode);

}

void COverlordPathNode::RemoveFromChain(CBasePlayer * pPlayer)
{
	if(IsSpawnNode())
		return;

	RemoveFromList(pPlayer);

	if(m_PrevNode)
		m_PrevNode->RemoveFromChain(pPlayer);
}

void COverlordPathNode::RespawnPlayer(CBasePlayer * pPlayer) const
{
	if(!pPlayer || GetOverlordData()->GetSpawnsAmount() <= 0)
		return;

	// Respawn the player
	GetOverlordData()->DecrementSpawns();

	pPlayer->ChangeTeam(TEAM_REBELS);
	pPlayer->ForceRespawn();
	TeleportPlayer(pPlayer);
}

void COverlordPathNode::TeleportPlayer(CBasePlayer * pPlayer) const
{
	if(!pPlayer)
		return;

	Vector dir;
	dir.Random(-1.0f, 1.0f);
	dir.z = 0;
	dir.NormalizeInPlace();

	float dist = rand() % m_iSpawnDistance;

	Vector position = WorldSpaceCenter() + dist * dir;
	pPlayer->SetAbsOrigin(position);

	if(GetNextNode())
	{
		COverlordPathNode * pNext = GetNextNode();

		dir = pNext->GetAbsOrigin() - GetAbsOrigin();
		dir.z = 0.0f;

		QAngle angle;
		VectorAngles(dir, angle);
		pPlayer->SetAbsAngles(angle);
	}
}

void COverlordPathNode::InputSetNextNode(inputdata_t & inputData)
{
	m_bMarkedByInput = true;
	COverlordPathNode * pEnt = static_cast<COverlordPathNode*>(gEntList.FindEntityByName(NULL, inputData.value.String()));

	if(pEnt)
	{
		m_NextNode = pEnt;
		float newLength = GetPathLength() + (GetAbsOrigin() - m_NextNode->GetAbsOrigin()).Length();
		pEnt->InitializeNode(this, newLength);
	}
}

void COverlordPathNode::InputSetAsReached(inputdata_t & inputData)
{
	m_bMarkedByInput = true;

	SetThink(&COverlordPathNode::Think);
	SetNextThink(gpGlobals->curtime + 0.25f);

	SetAsReached();
}

void COverlordPathNode::SetAsReached(CBasePlayer * pPlayer /* = NULL */)
{
	m_bReached = true;
		
	// Mark all the nodes back to the nearest reached
	if(m_PrevNode)
	{
		if(!m_PrevNode->WasReached())
			m_PrevNode->SetAsReached();
	}	

	m_OnPlayerReached.FireOutput(pPlayer, pPlayer);


	if(pPlayer)
		pPlayer->SetDeathNode(this);

	if(m_szHint)
		DisplayHint();

	if(IsSpawnNode())
	{
		m_SpawnNode = this;

		/* Give grace respawn time and restart the node */
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

			if(!pPlayer || pPlayer->IsOverlord() || (pPlayer->IsRebel() && pPlayer->IsAlive()))
				continue;

			pPlayer->SetDeathNode(this);
			pPlayer->SetCanSpawnTime(eo_can_spawn_time.GetFloat());
		}
	}

	if(!m_bIsAltNode)
		m_LastNode = this;
}

void COverlordPathNode::AddToList(CBasePlayer * pPlayer)
{
	if(m_PlayersPassed.HasElement(pPlayer))
		return;

	m_PlayersPassed.AddToTail(pPlayer);
}

void COverlordPathNode::RemoveFromList(CBasePlayer * pPlayer)
{
	m_PlayersPassed.FindAndRemove(pPlayer);
	
	if((GetPlayerCount() == 0) && IsLastNode())
	{
		m_bReached = false;
		m_LastNode = m_PrevNode;
	}
}

void COverlordPathNode::DisplayHint()
{
	if(!m_szHint)
		return;

	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(pRebels)
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || !pPlayer->IsAlive())
				continue;

			ClientPrint(pPlayer, HUD_PRINTCENTER, m_szHint);
		}
	}
}
//==================================== Path start ====================================
LINK_ENTITY_TO_CLASS(overlord_pathstart, COverlordPathStart);

void COverlordPathStart::Activate()
{
	BaseClass::Activate();

	// Start the first init
	InitializeNode(NULL, 0.0f);
	SetAsReached();

	m_FirstNode = this;
	m_LastNode = this;

	m_spawnflags |= SF_FIRST_NODE;
	m_spawnflags |= SF_SPAWN_NODE;
}

