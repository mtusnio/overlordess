//==============Overlord mod=====================
// Healing crate
//===============================================

#include "cbase.h"
#include "overlord_healcrate.h"
#include "team.h"
#include "hl2mp_gamerules.h"

#define CRATE_MODEL "models/props_combine/health_charger001.mdl"

ConVar eo_healcrate_radius("eo_healcrate_radius", "208", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_healcrate_rate("eo_healcrate_rate", "20", FCVAR_CHEAT | FCVAR_REPLICATED);

#define HEALING_SOUND "HealthKit.Touch"

LINK_ENTITY_TO_CLASS(item_overhealth, COverlordHealCrate);

BEGIN_DATADESC(COverlordHealCrate)

	DEFINE_THINKFUNC(HealThink),

	DEFINE_KEYFIELD(m_Health, FIELD_INTEGER, "health"),
	DEFINE_KEYFIELD(m_MedkitAmmo, FIELD_INTEGER, "medkit"),

END_DATADESC()

COverlordHealCrate::COverlordHealCrate()
{
}

COverlordHealCrate::~COverlordHealCrate()
{
}

void COverlordHealCrate::Precache()
{
	PrecacheModel(CRATE_MODEL);
	PrecacheScriptSound(HEALING_SOUND);
}

void COverlordHealCrate::Spawn()
{
	BaseClass::Spawn();

	Precache();

	SetModel(CRATE_MODEL);
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);

	CreateVPhysics();

	ResetSequence(LookupSequence("idle"));

	SetThink(&COverlordHealCrate::HealThink);
	SetNextThink(gpGlobals->curtime + 0.5f);

	m_MaxHealth = m_Health;
}

void COverlordHealCrate::HealThink()
{
	if(m_Health <= 0 && m_MedkitAmmo <= 0)
		return;

	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	bool bEmitSound = false;
	if(pRebels)
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || !pPlayer->IsAlive())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_healcrate_radius.GetFloat())
			{
				trace_t tr;
				UTIL_TraceLine(GetAbsOrigin(), pPlayer->RealEyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);


				if(tr.m_pEnt == pPlayer || tr.fraction >= 1.0f)
				{
					// Recharge health
					if(m_Health > 0)
					{
						int difference = pPlayer->GetMaxHealth() - pPlayer->GetHealth();
						difference = m_Health >= difference ? difference : m_Health;

						if(difference > eo_healcrate_rate.GetInt())
						{
							pPlayer->SetHealth(pPlayer->GetHealth() + eo_healcrate_rate.GetInt());
							m_Health -= eo_healcrate_rate.GetInt();
							bEmitSound = true;
						}
						else if(difference > 0)
						{
							pPlayer->SetHealth(pPlayer->GetMaxHealth());
							m_Health -= difference;
							bEmitSound = true;
						}

						
					}

					// Give ammo
					if(m_MedkitAmmo > 0)
					{
						for(int j = 0; j < MAX_WEAPONS; j++)
						{
							CBaseCombatWeapon * pWeap = pPlayer->GetWeapon(j);

							if(!pWeap)
								continue;

							if(!Q_stricmp(pWeap->GetClassname(), "weapon_medkit"))
							{
								int difference = pWeap->GetMaxClip1() - pWeap->Clip1();
								difference = m_MedkitAmmo >= difference ? difference : m_MedkitAmmo;

								if(difference > eo_healcrate_rate.GetInt())
								{
									pWeap->m_iClip1 += eo_healcrate_rate.GetInt();
									m_MedkitAmmo -= eo_healcrate_rate.GetInt();
								}
								else if(difference > 0)
								{
									pWeap->m_iClip1 = pWeap->GetMaxClip1();
									m_MedkitAmmo -= difference;
								}
					
								break;
							}
						}
					}
				}	
			}
		}
	}

	if(bEmitSound)
		EmitSound(HEALING_SOUND);

	float flCycle = (float)m_Health/(float)m_MaxHealth;
	if(m_Health <= 0)
		flCycle = 0.0f;

	SetNextThink(gpGlobals->curtime + 1.15f);
}