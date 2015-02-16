//==============Overlord mod=====================
// Teleporter weapon
//	
//===============================================



#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#ifndef CLIENT_DLL
#include "overlord_spawner.h"
#endif

#ifdef CLIENT_DLL
#define COverlordBlinker C_OverlordBlinker
#endif

#define FAIL_SOUND "WallHealth.Deny"

ConVar eo_blinker_teleport_time("eo_blinker_teleport_time", "3.0", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_HIDDEN);
ConVar eo_blinker_cooldown_time("eo_blinker_cooldown_time", "300", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_HIDDEN);
ConVar eo_blinker_teleport_slowdown("eo_blinker_teleport_slowdown", "175", FCVAR_CHEAT | FCVAR_REPLICATED | FCVAR_HIDDEN);

#define BLINK_TIME eo_blinker_teleport_time.GetFloat()
#define BLINK_COOLDOWN eo_blinker_cooldown_time.GetFloat();

class COverlordBlinker : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordBlinker, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();


	DECLARE_ACTTABLE();

	COverlordBlinker();
	virtual ~COverlordBlinker() { };

	virtual void Precache();

	virtual void PrimaryAttack();

	virtual PlayerClass_t GetClassWeapon() const { return CLASS_ALL; };

	virtual bool	HasAnyAmmo( void ) { return true; }
#ifndef CLIENT_DLL
	virtual void ItemPostFrame();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
#endif
	virtual bool ShouldDrawCrosshair() { return false; };
	virtual bool UsesAmmo() const { return false; };
private:
#ifndef CLIENT_DLL
	int m_InitialHealth;
	//Vector m_vTeleport;
	float m_flCooldownEnd;
	float m_flTeleportTime;
#endif
};


IMPLEMENT_NETWORKCLASS_ALIASED(OverlordBlinker, DT_OverlordBlinker)

BEGIN_NETWORK_TABLE( COverlordBlinker, DT_OverlordBlinker)
#ifndef CLIENT_DLL

#else

#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordBlinker )


END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_blinker, COverlordBlinker);


PRECACHE_WEAPON_REGISTER( weapon_blinker );



acttable_t	COverlordBlinker::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SLAM,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SLAM,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SLAM,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SLAM,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SLAM,					false },
};

IMPLEMENT_ACTTABLE(COverlordBlinker);


COverlordBlinker::COverlordBlinker()
{
#ifndef CLIENT_DLL
	m_InitialHealth = 0;
	//m_vTeleport = Vector(0, 0, 0);
	m_flCooldownEnd = 0.0f;
	m_flTeleportTime = 0.0f;
	m_InitialHealth = 0;
#endif
}

void COverlordBlinker::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(FAIL_SOUND);
}

void COverlordBlinker::PrimaryAttack()
{
#ifndef CLIENT_DLL
	if(m_flCooldownEnd > gpGlobals->curtime)
		return;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	pPlayer->AddSlowdown(eo_blinker_teleport_slowdown.GetInt(), -1.0f, this, true);

	m_flNextPrimaryAttack = gpGlobals->curtime + BLINK_TIME + 0.5f;
	m_flTeleportTime = gpGlobals->curtime + BLINK_TIME;
	m_InitialHealth = pPlayer->GetHealth();
	/*Vector forw;
	AngleVectors(pPlayer->RealEyeAngles(), &forw);

	Vector end = pPlayer->RealEyePosition() + forw * MAX_TRACE_LENGTH;

	trace_t tr;
	UTIL_TraceLine(pPlayer->RealEyePosition(), end, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr);

	CBasePlayer * pHit = ToBasePlayer(tr.m_pEnt);
	if(pHit && pHit->IsAlive() && pHit->IsRebel())
	{
		m_vTeleport = pHit->GetAbsOrigin();
		m_flNextPrimaryAttack = gpGlobals->curtime + BLINK_TIME + 0.5f;
		m_flTeleportTime = gpGlobals->curtime + BLINK_TIME;
		return;
	}


	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;*/
#endif
}

#ifndef CLIENT_DLL
bool COverlordBlinker::Holster(CBaseCombatWeapon * pSwitchingTo)
{
	if(m_flTeleportTime != 0.0f)
		return false;

	if(m_flCooldownEnd >= gpGlobals->curtime)
		engine->ClientCommand(GetOwner()->edict(), "hide_progress_bar");

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordBlinker::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	float perc = 0.0f;
	if(m_flTeleportTime != 0.0f)
	{
		perc = 1.0f - (m_flTeleportTime - gpGlobals->curtime)/BLINK_TIME;

		if(pPlayer->GetHealth() < m_InitialHealth)
		{
			m_flTeleportTime = 0.0f;
			EmitSound(FAIL_SOUND);
			m_flCooldownEnd = 0.0f;
			pPlayer->RemoveSlowdown(this);
			return;
		}

		if(m_flTeleportTime <= gpGlobals->curtime)
		{
			m_flTeleportTime = 0.0f;
			m_flCooldownEnd = gpGlobals->curtime + BLINK_COOLDOWN;
			/*GetOwner()->Teleport(&m_vTeleport, NULL, &vec3_origin);*/
			COverlordSpawner * pSpawner = COverlordSpawner::GetLastUsed();

			if(pSpawner && !pSpawner->GetObjectOnTop())
			{
				Vector tele = pSpawner->GetSpawnSpot();
				QAngle angle = pSpawner->GetAbsAngles();
				pPlayer->Teleport(&tele, &angle, &vec3_origin);
			}
			else
			{
				EmitSound(FAIL_SOUND);
				m_flCooldownEnd = 0.0f;
			}

			pPlayer->RemoveSlowdown(this);
		}
	}
	else if(m_flCooldownEnd != 0.0f && m_flCooldownEnd >= gpGlobals->curtime)
	{
		perc = 1.0f - (m_flCooldownEnd - gpGlobals->curtime)/BLINK_COOLDOWN;
	}

	if(perc > 0.0f)
	{
		char command[32];
		Q_snprintf(command, ARRAYSIZE(command), "progress_bar %f", perc);
		engine->ClientCommand(GetOwner()->edict(), command);
	}
}

#endif

