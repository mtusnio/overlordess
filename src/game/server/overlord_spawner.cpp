//==============Overlord mod=====================
//	Player spawner
//	
//===============================================

#include "cbase.h"
#include "overlord_spawner.h"
#include "team.h"
#include "hl2mp_gamerules.h"
#include "sprite.h"

#define DENY_SOUND "SuitRecharge.Deny"
#define SPAWNER_MODEL "models/spawner/spawner.mdl"
#define SPAWN_VECTOR Vector(8, 0, 16)

#define HEALTH_MINIMUM 10

#define SF_INITIAL_SPAWNER 1

CHandle<COverlordSpawner> COverlordSpawner::m_LastUsed = NULL;

//ConVar eo_percentage_per_spawn("eo_percentage_per_spawn", "0.05", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_spawner_push_force("eo_spawner_push_force", "290", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_spawner_points("eo_spawner_points", "10", FCVAR_REPLICATED | FCVAR_CHEAT);

// Health, ammo etc. lost with each spawn
//#define SPAWN_PERCENTAGE eo_percentage_per_spawn.GetFloat()

ConVar eo_playtest_spawnerdeplete("eo_playtest_spawnerdeplete", "0", 			  
#ifndef OVER_PLAYTEST
								 FCVAR_HIDDEN |
#endif
								 FCVAR_REPLICATED);

BEGIN_DATADESC(COverlordSpawner)

	DEFINE_FUNCTION(Think),


	DEFINE_INPUTFUNC( FIELD_VOID, "MarkSpawner", InputMarkSpawner),
	DEFINE_KEYFIELD(m_AutomarkDistance, FIELD_INTEGER, "playerdistance"),
	//DEFINE_KEYFIELD(m_iUses, FIELD_INTEGER, "Uses"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(COverlordSpawner, DT_OverlordSpawner)
	SendPropBool(SENDINFO(m_bAlreadyMarked)),
	//SendPropInt(SENDINFO(m_iUses), 0, SPROP_UNSIGNED),
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS(over_spawner, COverlordSpawner);

COverlordSpawner::COverlordSpawner()
{
	m_bAlreadyMarked = false;
	m_flNextDelayed = 0.0f;
	m_bDelayedSpawn = false;
	m_bSetAsLast = false;
	m_iQueue = 0;
	m_hGlow = NULL;
}

COverlordSpawner::~COverlordSpawner()
{
}


void COverlordSpawner::Spawn()
{
	Precache();

	SetSolid(SOLID_VPHYSICS); 

	SetMoveType(MOVETYPE_NONE);

	SetModel(SPAWNER_MODEL);

	SetThink(&COverlordSpawner::Think);

	SetNextThink(TICK_NEVER_THINK);

	m_flNextSpawn = 0.0f;

	CreateVPhysics();
}

void COverlordSpawner::Activate()
{
	BaseClass::Activate();

	if(HasSpawnFlags(SF_INITIAL_SPAWNER) == true)
	{
		// Send a delayed event message
		m_bSetAsLast = true;
		m_bAlreadyMarked = true;
		SetNextThink(gpGlobals->curtime + 1.5f);
		SetAsLastUsed();
	}
	else
		SetNextThink(gpGlobals->curtime + 0.5f);
}

void COverlordSpawner::Precache()
{
	BaseClass::Precache();

	PrecacheModel(SPAWNER_MODEL);
	PrecacheModel("sprites/light_glow03.vmt");

	PrecacheScriptSound(DENY_SOUND);
}

void COverlordSpawner::Think()
{
	if(m_bSetAsLast)
	{
		m_bSetAsLast = false;
		//DevMsg("Spawner event sent\n");
		IGameEvent * event = gameeventmanager->CreateEvent("spawner_used");

		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetBool("mark", "true");
			gameeventmanager->FireEvent(event);
		}
		SetNextThink(TICK_NEVER_THINK);
		return;
	}

	if(!m_bAlreadyMarked)
	{
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || !pPlayer->IsAlive())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= m_AutomarkDistance)
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

				if(tr.m_pEnt == pPlayer || tr.fraction >= 1.0f)
				{
					Mark();
					SetNextThink(TICK_NEVER_THINK);
					return;
				}
			}
		}

		SetNextThink(gpGlobals->curtime + 0.25f);
		return;
	}

	if(m_pRespawn)
	{
		// Give some power for the guy
		(GET_OVERLORD_DATA)->HandlePowerEvent(EVENT_PLAYERSPAWNED);

		Vector vOrg =GetSpawnSpot();
		m_pRespawn->Teleport(&vOrg, &GetAbsAngles(), NULL);

		m_pRespawn = NULL;
		SetNextThink(TICK_NEVER_THINK);
		return;
		//m_pRespawn->SetAbsOrigin(GetAbsOrigin() + SPAWN_VECTOR);
		//m_pRespawn->SetAbsAngles(GetAbsAngles());
		/*int deaths = m_pRespawn->GetDeathsThisRound();
		// Now take care of his ammo and max health
		
#ifdef OVER_PLAYTEST
		if(!eo_playtest_spawnerdeplete.GetBool())
			goto end;
#endif*/
		// Health first
		/*{
			int maxHealth = m_pRespawn->GetMaxHealth();
			for(int i = 1; i <= deaths; i++)
			{
				maxHealth = (int)((float)maxHealth - (SPAWN_PERCENTAGE * (float)maxHealth));
			}

			if(maxHealth < HEALTH_MINIMUM)
				maxHealth = HEALTH_MINIMUM;

			m_pRespawn->SetHealth(maxHealth);
			m_pRespawn->SetMaxHealth(maxHealth);
		}


		// Now ammo for ammo
		{
			for(int i = 0; i < MAX_WEAPONS; i++)
			{
				CBaseCombatWeapon * pWeapon = m_pRespawn->GetWeapon(i);

				if(!pWeapon)
					continue;

				int ammo = pWeapon->m_iClip1;
		
				if(ammo > 0)
				{
					for(int j = 1; j <= deaths; j++)
					{
						ammo = (int)((float)ammo - (SPAWN_PERCENTAGE * (float)ammo));
					}
					if(ammo <= 0)
						ammo = 1;
				}

				pWeapon->m_iClip1 = ammo;
				pWeapon->m_iMaxClip1 = ammo;
				
				int ammo2 = pWeapon->m_iClip2;

				if(ammo2 > 0)
				{
					for(int j = 1; j <= deaths; j++)
					{
						ammo2 = (int)((float)ammo2 - (SPAWN_PERCENTAGE * (float)ammo2));
					}
					if(ammo2 <= 0)
						ammo2 = 1;
				}

				pWeapon->m_iClip2 = ammo2;
				pWeapon->m_iMaxClip2 = ammo2;

			}
		}*/
	}

	if(m_bDelayedSpawn)
	{
		if(m_flNextDelayed <= gpGlobals->curtime)
		{
			if((GET_OVERLORD_DATA)->GetSpawnsAmount() < 1)
			{
				m_bDelayedSpawn = false;
				m_flNextDelayed = 0.0f;
				m_iQueue = 0;
				SetNextThink(TICK_NEVER_THINK);
				return;
			}

			if(CBaseEntity * pEnt = GetObjectOnTop())
			{
				if(pEnt->IsPlayer())
				{
					PushPlayer();
				}
				else
				{
					UTIL_Remove(pEnt);
				}

				SetNextThink(gpGlobals->curtime + 1.0f);
				m_flNextDelayed = gpGlobals->curtime + 1.0f;
				return;
			}

			if(SpawnPlayer())
			{
				m_iQueue--;

				if(m_iQueue <= 0)
				{
					//SetNextThink(TICK_NEVER_THINK);
					m_bDelayedSpawn = false;
					m_flNextDelayed = 0.0f;
				}
				else
				{
					m_flNextDelayed = gpGlobals->curtime + 1.5f;
					//SetNextThink(gpGlobals->curtime + 1.5f);
				}
			}
			else
			{
				SetNextThink(gpGlobals->curtime + 1.0f);
				m_flNextDelayed = gpGlobals->curtime + 1.0f;
			}
			return;
		}
		else
		{
			SetNextThink(gpGlobals->curtime + 0.1f);
			return;
		}
	}

/*#ifdef OVER_PLAYTEST
end:
#endif*/
	SetNextThink(TICK_NEVER_THINK);
}

int	 COverlordSpawner::GetUses() const
{
	return (GET_OVERLORD_DATA)->GetSpawnsAmount();
}

bool COverlordSpawner::CanSpawn() const
{
	// Some quick hack, don't let it spawn players with a player standing on it
	if(GetObjectOnTop())
		return false;

	if(m_bAlreadyMarked && (GetLastUsed() != this))
		return false;

	return !((m_flNextSpawn > gpGlobals->curtime) || (GetUses() <= 0));
}

CBaseEntity * COverlordSpawner::GetObjectOnTop() const
{
	Vector origin = GetSpawnSpot();

	Vector vUp;
	VectorMA(origin, 73, Vector(0, 0, 1), vUp);

	trace_t tr;
	UTIL_TraceHull(origin, vUp, -Vector(6, 6, 6), Vector(6, 6, 6), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	return tr.m_pEnt;
}

bool COverlordSpawner::PushPlayer()
{
	CBasePlayer * pStanding = ToBasePlayer(GetObjectOnTop());
	if(pStanding)
	{
		Vector force;
		AngleVectors(GetAbsAngles(), &force);

		force *= eo_spawner_push_force.GetFloat();
		
		pStanding->ApplyAbsVelocityImpulse(force);
		return true;
	}
	return false;
}

void COverlordSpawner::SetAsLastUsed()
{
	if(COverlordSpawner::m_LastUsed)
	{
		COverlordSpawner::m_LastUsed->Unmark();
	}

	Vector lightPos;
	QAngle lightAng;

	GetAttachment(LookupAttachment("light"), lightPos, lightAng);

	m_hGlow = CSprite::SpriteCreate("sprites/light_glow03.vmt", lightPos, false );

	if(m_hGlow)
	{
		m_hGlow->SetAttachment( this, LookupAttachment( "light" ) );
		m_hGlow->SetTransparency( kRenderWorldGlow, 0, 0, 255, 255, kRenderFxNoDissipation );
		m_hGlow->SetBrightness( 255, 0.2f );
		m_hGlow->SetScale( 0.15f);
	}


	COverlordSpawner::m_LastUsed = this;
}

void COverlordSpawner::DelayedSpawn(float flDelay)
{
	if(!m_bDelayedSpawn)
	{
		m_iQueue = 1;
		m_bDelayedSpawn = true;

		m_flNextDelayed = gpGlobals->curtime + flDelay;
		SetNextThink(gpGlobals->curtime + flDelay);
	}
	else
	{
		m_iQueue++;
	}
}

void COverlordSpawner::InputMarkSpawner(inputdata_t & inputdata)
{
	Mark();
}

void COverlordSpawner::Mark()
{
	if(m_bAlreadyMarked)
		return;

	SetAsLastUsed();
	m_bAlreadyMarked = true;

	
	IGameEvent * event = gameeventmanager->CreateEvent("spawner_used");

	if(event)
	{
		event->SetInt("entindex", entindex());

		event->SetBool("mark", true);

		gameeventmanager->FireEvent(event);
	}

	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(pRebels)
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer)
				continue;

			ClientPrint(pPlayer, HUD_PRINTCENTER, "New spawner marked!");
		}
	}
	
}

void COverlordSpawner::Unmark()
{
	if(m_hGlow)
		UTIL_Remove(m_hGlow);

}

Vector COverlordSpawner::GetSpawnSpot() const
{
	Vector ret;
	EntityToWorldSpace(SPAWN_VECTOR, &ret);

	return ret;
}



bool COverlordSpawner::SpawnPlayer(CBasePlayer * pPlayer /*= NULL*/)
{
	// Search for player who died the earliest
	CTeam * pSpectators = GetGlobalTeam(TEAM_SPECTATOR);
	
	float flTime = 0.0f;

	if(!pPlayer)
	{
		for(int i = 0; i < pSpectators->GetNumPlayers(); i++)
		{
			CBasePlayer * pSpec = pSpectators->GetPlayer(i);
			
			if(!pSpec)
				continue;

			// Select the earliest time of death
			if((flTime > pSpec->GetDeathTime()) || flTime == 0.0f)
			{
				flTime = pSpec->GetDeathTime();
				pPlayer = pSpec;
			}
		}

		// Find one among rebel team
		if(!pPlayer)
		{
			CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

			if(pRebels && pRebels->GetNumPlayers() > 0)
			{
				flTime = 0.0f;
				for(int i = 0; i < pRebels->GetNumPlayers(); i++)
				{
					CBasePlayer * pRebel = pRebels->GetPlayer(i);

					if(!pRebel || pRebel->IsAlive())
						continue;

					if((flTime > pRebel->GetDeathTime()) || flTime == 0.0f)
					{
						flTime = pRebel->GetDeathTime();
						pPlayer = pRebel;
					}	
				}
			}
		}
	}


	if(pPlayer)
	{
		// Respawn the player
		pPlayer->ChangeTeam(TEAM_REBELS);
		pPlayer->ForceRespawn();

		m_pRespawn = pPlayer;

		SetNextThink(gpGlobals->curtime + 0.15f);

		(GET_OVERLORD_DATA)->DecrementSpawns();

		m_flNextSpawn = gpGlobals->curtime + 2.0f;
		
		return true;
	}

	return false;
	
}