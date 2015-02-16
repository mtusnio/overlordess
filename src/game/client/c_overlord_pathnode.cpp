//==============Overlord mod=====================
//	Path node base class
//===============================================

#include "cbase.h"
#include "c_overlord_pathnode.h"
#include "overlord_data.h"
#include "in_buttons.h"
#include "fx_line.h"


CHandle<C_OverlordPathNode> C_OverlordPathNode::m_FirstNode = NULL;

IMPLEMENT_CLIENTCLASS(C_OverlordPathNode, DT_OverlordPathNode, COverlordPathNode);

BEGIN_RECV_TABLE(C_OverlordPathNode, DT_OverlordPathNode)

RecvPropEHandle(RECVINFO(m_NextNode)),
RecvPropEHandle(RECVINFO(m_PrevNode)),
RecvPropEHandle(RECVINFO(m_AltNode)),
RecvPropFloat(RECVINFO(m_PathLength)),
RecvPropBool(RECVINFO(m_bReached)),
RecvPropBool(RECVINFO(m_bIsAltNode)),
RecvPropInt(RECVINFO(m_spawnflags)),

END_RECV_TABLE()

C_OverlordPathNode * C_OverlordPathNode::GetFirstNode()
{
	return m_FirstNode;
}

C_OverlordPathNode * C_OverlordPathNode::GetSpawnNode()
{
	C_OverlordPathNode * pLast = GetFirstNode();
	C_OverlordPathNode * pLastSpawn = pLast;

	if(!pLast)
		return NULL;

	while(pLast->GetNextNode() && pLast->GetNextNode()->WasReached())
	{
		pLast = pLast->GetNextNode();
		if(pLast->IsSpawnNode())
			pLastSpawn = pLast;
	}

	return pLastSpawn;
}

C_OverlordPathNode * C_OverlordPathNode::GetLastNode()
{
	C_OverlordPathNode * pLast = GetFirstNode();

	if(!pLast)
	{
		Warning("CLIENT: Cannot find first node\n");
		return NULL;
	}

	while(pLast->GetNextNode() && pLast->GetNextNode()->WasReached())
		pLast = pLast->GetNextNode();

	return pLast;
}

C_OverlordPathNode::C_OverlordPathNode()
{
	m_BeamMat = NULL;
}

C_OverlordPathNode::~C_OverlordPathNode()
{
}

void C_OverlordPathNode::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if(updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}

	if(m_spawnflags & SF_FIRST_NODE)
	{
		m_FirstNode = this;
	}
	

}

void C_OverlordPathNode::ClientThink()
{
	if(ShouldShowNode())
	{
		if(!m_BeamMat)
		{
			m_BeamMat = materials->FindMaterial("sprites/selectionbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
		}

		if(m_NextNode)
		{
			DrawLine(m_NextNode, GetBeamToNodeColour(m_NextNode));
		}

		if(m_AltNode)
		{
			DrawLine(m_AltNode, GetBeamToNodeColour(m_AltNode));
		}

		SetRenderColor(GetNodeColor());
	}

	SetNextClientThink(gpGlobals->curtime + 0.3f);
}

int C_OverlordPathNode::DrawModel(int flags)
{
	if(!ShouldShowNode())
		return 0;

	return BaseClass::DrawModel(flags);
}


Color C_OverlordPathNode::GetNodeColor() const
{
	const int ALPHA = 110;
	
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return Color(0, 0, 0, ALPHA);

	if(pPlayer->IsOverlord())
	{
		if(IsSpawnNode())
		{
			if(WasReached())
				return Color(139, 90, 0, ALPHA);
			else
				return Color(238, 154, 0, ALPHA);
		}
		else if(CanSpawnNode())
		{
			if(WasReached())
				return Color(255, 69, 0, ALPHA);		
			else
				return Color(139, 37, 0, ALPHA);
		}
		else
		{
			if(WasReached())
				return Color(255, 0, 0, ALPHA);
			else
				return Color(0, 0, 255, ALPHA);
		}
	}
	else
	{
		if(IsSpawnNode())
		{
			if(WasReached())
				return Color(238, 238, 0, ALPHA);
			else
				return Color(205, 205, 0, ALPHA);
		}
		else if(CanSpawnNode())
		{
			if(WasReached())
				return Color(238, 201, 0, ALPHA);
			else
				return Color(139, 117, 0, ALPHA);	
		}
		else
		{
			if(WasReached())
				return Color(0, 255, 0, ALPHA);
			else
				return Color(0, 0, 255, ALPHA);
		}
	}
	
	return Color(0, 0, 255, ALPHA);
}


bool C_OverlordPathNode::ShouldShowNode() const
{
	// Do not draw isolated nodes
	if(!m_PrevNode && !m_NextNode)
		return false;

	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return false;

	if(pPlayer->IsRebel())
	{
		if(pPlayer->m_nButtons & IN_SHOWNODES)
			return true;

		return false;
	}
	else if(pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
	{
		return true;
	}

	C_OverlordData & data = *GetOverlordData();

	if(!data.IsInVehicle())
		return false;

	return true;
}

Color C_OverlordPathNode::GetBeamToNodeColour(C_OverlordPathNode * node) const
{
	if(node->IsAltNode() || IsAltNode())
		return Color(255, 255, 0);

	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return Color(0, 0, 0);

	if(pPlayer->IsOverlord())
	{
		if(node->WasReached())
			return Color(255, 0, 0);
		else
			return Color(0, 0, 255);
	}
	else
	{
		if(node->WasReached())
			return Color(0, 255, 0);
		else
			return Color(0, 0, 255);
	}

	return Color(0, 0, 0);
}

void C_OverlordPathNode::DrawLine(C_OverlordPathNode * node, Color clr) const
{
	FXLineData_t data;
	data.m_flDieTime = 0.3f;
	data.m_vecStart = GetAbsOrigin();
	data.m_vecEnd = node->GetAbsOrigin();
	data.m_pMaterial = m_BeamMat;
	data.m_Color.a = 255;
	data.m_Color.r = clr.r();
	data.m_Color.g = clr.g();
	data.m_Color.b = clr.b();
	data.m_flStartScale = 4;
	data.m_flEndScale = 4;
	data.m_flStartAlpha = 255;
	data.m_flEndAlpha = 255;

	data.m_vecStartVelocity = vec3_origin;
	data.m_vecEndVelocity = vec3_origin;

	FX_AddLine(data);
}
