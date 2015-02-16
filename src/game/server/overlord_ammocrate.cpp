//==============Overlord mod=====================
// Ammo-refilling crate
//===============================================

#include "cbase.h"
#include "overlord_ammocrate.h"
#include "hl2mp_gamerules.h"
#include "ammodef.h"

ConVar eo_ammocrate_radius("eo_ammocrate_radius", "62", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_ammocrate_cooldown("eo_ammocrate_cooldown", "30", FCVAR_REPLICATED | FCVAR_CHEAT);

#define CRATE_MODEL "models/items/ammocrate_ar2.mdl"
#define RESUPPLY_RADIUS eo_ammocrate_radius.GetFloat()

BEGIN_DATADESC(COverlordAmmoCrate)

	DEFINE_THINKFUNC(AmmoThink),

END_DATADESC()

LINK_ENTITY_TO_CLASS(item_overammo, COverlordAmmoCrate);

COverlordAmmoCrate::COverlordAmmoCrate()
{
	m_flOpenDelay = 0.0f;
	m_flCloseTime = 0.0f;
}

COverlordAmmoCrate::~COverlordAmmoCrate()
{
}

void COverlordAmmoCrate::Precache()
{
	PrecacheModel(CRATE_MODEL);
	PrecacheScriptSound("HL2Player.PickupWeapon");
}

void COverlordAmmoCrate::Spawn()
{
	Precache();

	SetModel(CRATE_MODEL);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);

	CreateVPhysics();

	ResetSequence( LookupSequence( "Idle" ) );
	
	SetThink(&COverlordAmmoCrate::AmmoThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void COverlordAmmoCrate::AmmoThink()
{
	StudioFrameAdvance();
	//DispatchAnimEvents( this );

	if(m_flCloseTime > gpGlobals->curtime)
	{
		// Resupply anyone near us
		CBaseEntity * pList[1024];
		int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), WorldSpaceCenter(), RESUPPLY_RADIUS, 0);

		
		CAmmoDef * def = GetAmmoDef();

		if(!def)
		{
			Warning("Can't find ammo definition.\n");
			return;
		}

		for(int i = 0; i < count; i++)
		{
			CBaseEntity * pEnt = pList[i];

			if(!pEnt || !pEnt->IsPlayer() || pEnt->GetTeamNumber() != TEAM_REBELS)
				continue;

			trace_t tr;
			UTIL_TraceLine(WorldSpaceCenter(), pEnt->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);


			// Resupply
			if(tr.m_pEnt == pEnt)
			{
				CBasePlayer * pPlayer = ToBasePlayer(pEnt);
				
				if(!ShouldOpenForPlayer(pPlayer))
					continue;

				int iClientId = pPlayer->entindex()-1;
				m_Records[iClientId].m_Player = pPlayer;
				m_Records[iClientId].m_flEndTime = gpGlobals->curtime + eo_ammocrate_cooldown.GetFloat();
				
				bool bPlaySound = false;

				if(!HasSpawnFlags(SF_NO_AMMO))
				{
					for(int j = 0; j < MAX_WEAPONS; j++)
					{
						CBaseCombatWeapon * pWeap = pPlayer->GetWeapon(j);

						if(!pWeap)
							continue;

						int primary = pWeap->GetPrimaryAmmoType();

						if(!Q_stricmp("Charge", def->GetAmmoOfIndex(primary)->pName))
							continue;

						if(primary != -1)
						{
							if(def->MaxCarry(primary) > pPlayer->GetAmmoCount(primary))
								bPlaySound = true;

							pPlayer->SetAmmoCount(def->MaxCarry(primary), primary);
						}

						int secondary = pWeap->GetSecondaryAmmoType();

						if(!Q_stricmp("Charge", def->GetAmmoOfIndex(secondary)->pName))
							continue;

						if(secondary != -1)
						{
							if(def->MaxCarry(secondary) > pPlayer->GetAmmoCount(secondary))
								bPlaySound = true;

							pPlayer->SetAmmoCount(def->MaxCarry(secondary), secondary);
						}

				
						if(pWeap->UsesClipsForAmmo1())
							pWeap->m_iClip1 = pWeap->GetMaxClip1();

						if(pWeap->UsesClipsForAmmo2())
							pWeap->m_iClip2 = pWeap->GetMaxClip2();
					}
				}
				
				//if(!HasSpawnFlags(SF_NO_HEALTH))
				//	pPlayer->SetHealth(pPlayer->GetMaxHealth());

				if(bPlaySound)
				{
					CSingleUserRecipientFilter filter(pPlayer);
					pPlayer->EmitSound(filter, pPlayer->entindex(), "HL2Player.PickupWeapon");
				}
			}
		}
	}
	else if(m_flCloseTime <= gpGlobals->curtime && m_flCloseTime != 0.0f)
	{
		// Close
		ResetSequence( LookupSequence( "Close" ) );
		m_flCloseTime = 0.0f;
		m_flOpenDelay = gpGlobals->curtime + SequenceDuration(LookupSequence("Close"));
	}
	else if(m_flCloseTime == 0.0f && m_flOpenDelay <= gpGlobals->curtime)
	{
		// Open
		CBaseEntity * pList[1024];
		int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), WorldSpaceCenter(), RESUPPLY_RADIUS, 0);

		for(int i = 0; i < count; i++)
		{
			CBaseEntity * pEnt = pList[i];

			if(!pEnt || !pEnt->IsPlayer() || pEnt->GetTeamNumber() != TEAM_REBELS)
				continue;

			if(!ShouldOpenForPlayer(ToBasePlayer(pEnt)))
				continue;

			trace_t tr;
			UTIL_TraceLine(WorldSpaceCenter(), pEnt->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			// Start opening
			if(tr.m_pEnt == pEnt)
			{
				ResetSequence( LookupSequence( "Open" ) );
				m_flCloseTime = gpGlobals->curtime + 2.0f;
				m_flOpenDelay = 0.0f;
				break;
			}
		}
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

bool COverlordAmmoCrate::ShouldOpenForPlayer(CBasePlayer * pPlayer)
{
	int iClientId = pPlayer->entindex()-1;

	if(m_Records[iClientId].m_Player)
	{
		if(m_Records[iClientId].m_flEndTime <= gpGlobals->curtime)
		{
			m_Records[iClientId].m_Player = NULL;
			m_Records[iClientId].m_flEndTime = 0.0f;
			return true;
		}
		else
			return false;
	}	

	return true;
}