//==============Overlord mod=====================
// Backstabbing knife
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#define COverlordKnife C_OverlordKnife
#endif

#define CHARGE_SOUND "Jeep.GaussCharge"
#define ATTACK_SOUND "AlyxEMP.Charge"

ConVar eo_stealther_knife_max_damage("eo_stealther_knife_max_damage", "100", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stealther_knife_min_damage("eo_stealther_knife_min_damage", "5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stealther_knife_charge_time("eo_stealther_knife_charge_time", "1.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stealther_knife_hold_time("eo_stealther_knife_hold_time", "1.75", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordKnife : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS(COverlordKnife, CBaseHL2MPBludgeonWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	COverlordKnife();
	virtual ~COverlordKnife();

	virtual void Precache();

	float		GetRange( void ) { return 62; };
	//float		GetFireRate( void ) { return m_bBackstab ? 2.1f : 0.2f; };
	virtual float GetDamageForActivity(Activity hitActivity);

	virtual void PrimaryAttack();

	// No secondary
	virtual void SecondaryAttack() { };

	virtual bool		  IsClassWeapon() const { return true; };
	virtual	PlayerClass_t GetClassWeapon() const { return CLASS_STEALTH; };

	virtual void ItemPostFrame();

	virtual bool Holster(CBaseCombatWeapon * pSwitchingTo);
private:
	float m_flCharged;

	float m_flStartedCharging;
	float m_flFinishedCharging;
};
IMPLEMENT_NETWORKCLASS_ALIASED(OverlordKnife, DT_OverlordKnife)

BEGIN_NETWORK_TABLE( COverlordKnife, DT_OverlordKnife)

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordKnife )

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_knife, COverlordKnife);
					 

PRECACHE_WEAPON_REGISTER( weapon_knife );


acttable_t	COverlordKnife::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(COverlordKnife);


COverlordKnife::COverlordKnife()
{
	m_flCharged = 0.0f;

	m_flStartedCharging = 0.0f;
	m_flFinishedCharging = 0.0f;
}

COverlordKnife::~COverlordKnife()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(pPlayer)
		pPlayer->StopSound(CHARGE_SOUND);
}

void COverlordKnife::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(CHARGE_SOUND);
	PrecacheScriptSound(ATTACK_SOUND);
}

float COverlordKnife::GetDamageForActivity(Activity hitActivity)
{
	float damage = eo_stealther_knife_min_damage.GetFloat();
	
	if(m_flFinishedCharging != 0.0f)
	{
		damage = eo_stealther_knife_max_damage.GetFloat();
		return damage;
	}

	float fraction = (gpGlobals->curtime - m_flStartedCharging)/eo_stealther_knife_charge_time.GetFloat();

	damage += (eo_stealther_knife_max_damage.GetFloat() - eo_stealther_knife_min_damage.GetFloat()) * fraction;

	return damage;
}

void COverlordKnife::PrimaryAttack()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	if(m_flStartedCharging == 0.0f)
	{
		m_flStartedCharging = gpGlobals->curtime;

		// Send weapon anim here
		pPlayer->EmitSound(CHARGE_SOUND);

	}
	else
	{
		if(((m_flStartedCharging + eo_stealther_knife_charge_time.GetFloat()) <= gpGlobals->curtime) 
			&& m_flFinishedCharging == 0.0f)
		{
			m_flFinishedCharging = gpGlobals->curtime;

			// Send weapon anim here
		}
		else if((m_flFinishedCharging + eo_stealther_knife_hold_time.GetFloat()) <= gpGlobals->curtime
			&& m_flFinishedCharging != 0.0f)
		{
			BaseClass::PrimaryAttack();

			pPlayer->EmitSound(ATTACK_SOUND);
			pPlayer->StopSound(CHARGE_SOUND);

			m_flFinishedCharging = 0.0f;
			m_flStartedCharging = 0.0f;

			m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
			return;
		}
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;

}


void COverlordKnife::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if(m_flStartedCharging != 0.0f)
	{
		CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

		if(pPlayer)
		{
			if(!(pPlayer->m_nButtons & IN_ATTACK))
			{
				BaseClass::PrimaryAttack();

				pPlayer->EmitSound(ATTACK_SOUND);
				pPlayer->StopSound(CHARGE_SOUND);

				m_flFinishedCharging = 0.0f;
				m_flStartedCharging = 0.0f;

				m_flNextPrimaryAttack = gpGlobals->curtime + 0.6f;
			}
		}
	}
}

bool COverlordKnife::Holster(CBaseCombatWeapon * pSwitchingTo)
{
	m_flCharged = 0.0f;

	m_flStartedCharging = 0.0f;
	m_flFinishedCharging = 0.0f;

	return BaseClass::Holster(pSwitchingTo);
}