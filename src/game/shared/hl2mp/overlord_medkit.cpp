//==============Overlord mod=====================
//	Medkit weapon
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#endif

#ifdef CLIENT_DLL
#define COverlordMedkit C_OverlordMedkit
#endif

#define SOUND_INTERVAL 1.1f
#define SOUND_DENY "SuitRecharge.Deny"
#define SOUND_LOOP "SuitRecharge.ChargingLoop"
#define SOUND_START "SuitRecharge.Start"

#define HEAL_RANGE 72

#define PRIMARY_INTERVAL 0.3f
#define SECONDARY_INTERVAL 0.3f

ConVar eo_medkit_ammousage("eo_medkit_ammousage", "10", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_medkit_self_ammo_factor("eo_medkit_self_ammo_factor", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_medkit_health_per_ammo("eo_medkit_health_per_ammo", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_points_per_healing("eo_points_per_healing", "5", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordMedkit : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordMedkit, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();


	COverlordMedkit();
	virtual ~COverlordMedkit();

	virtual void Precache();

	virtual int GetClassWeapon() const { return CLASS_ALL; };

	virtual bool		  IsMainClassWeapon() const { return false; };

	virtual const char * GetPrimaryMode() const { return "Heal"; };
	virtual const char * GetSecondaryMode() const { return "Recharge"; };

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual bool HasAnyAmmo() { return true; };
	virtual bool CanBeSelected() { return true; };

private:
	void Heal(CBasePlayer * pHeal);

#ifdef CLIENT_DLL
	float m_flSoundTime;
#endif
};


IMPLEMENT_NETWORKCLASS_ALIASED(OverlordMedkit, DT_OverlordMedkit)

BEGIN_NETWORK_TABLE(COverlordMedkit, DT_OverlordMedkit)

END_NETWORK_TABLE()




#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordMedkit )

	//DEFINE_PRED_FIELD_TOL( m_flSoundTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE )

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_medkit, COverlordMedkit);

PRECACHE_WEAPON_REGISTER(weapon_medkit);



acttable_t	COverlordMedkit::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_GRENADE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_GRENADE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_GRENADE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_GRENADE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_GRENADE,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_GRENADE,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_GRENADE,					false },
};


IMPLEMENT_ACTTABLE(COverlordMedkit);



COverlordMedkit::COverlordMedkit()
{
#ifdef CLIENT_DLL
	m_flSoundTime = 0.0f;
#endif
	m_bFiresUnderwater = true;
	m_bAltFiresUnderwater = true;
}

COverlordMedkit::~COverlordMedkit()
{
}


void COverlordMedkit::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(SOUND_DENY);
	//PrecacheScriptSound(SOUND_LOOP);
	//PrecacheScriptSound(SOUND_START);
}


void COverlordMedkit::PrimaryAttack()
{
	if(m_flNextSecondaryAttack > gpGlobals->curtime)
		return;

#ifndef CLIENT_DLL
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetPlayerOwner() );
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );
#endif

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());;

	// Find player in front of us
	Vector pos = pOwner->RealEyePosition();
	Vector end;

	Vector forw;
	AngleVectors(pOwner->EyeAngles(), &forw);
	
	VectorMA(pos, HEAL_RANGE, forw, end);

	trace_t tr;
	UTIL_TraceLine(pos, end, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

#ifndef CLIENT_DLL
	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );
#endif

	// Now we need to find the player
	CBasePlayer * pHeal = ToBasePlayer(tr.m_pEnt);

	// Ignore nonrebels
	if(!pHeal || !pHeal->IsRebel() || !pHeal->IsAlive() || pHeal->IsInvisible())
	{
		return;
	}

	Heal(pHeal);

	// Delay BOTH to prevent the both-buttons-pressed exploit!
	m_flNextPrimaryAttack = gpGlobals->curtime + PRIMARY_INTERVAL;
	m_flNextSecondaryAttack = gpGlobals->curtime + PRIMARY_INTERVAL;
}

void COverlordMedkit::SecondaryAttack()
{
	if(m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	Heal(ToBasePlayer(GetOwner()));

	// Delay BOTH to prevent both-buttons-pressed exploit!
	m_flNextPrimaryAttack = gpGlobals->curtime + SECONDARY_INTERVAL;
	m_flNextSecondaryAttack = gpGlobals->curtime + SECONDARY_INTERVAL;
}

void COverlordMedkit::Heal(CBasePlayer * pHeal)
{
	if(!HasAmmo() || !pHeal)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	// Emit sound when we can't recharge somebody
	if(Clip1() <= 0)
	{
		pOwner->EmitSound(SOUND_DENY);
		return;
	}

	// Again, deny if somebody has full health

	if(pHeal->GetHealth() >= pHeal->GetMaxHealth())
	{
		return;
	}

	// Now charge


	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	pOwner->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL
	// Decide how much ammo was used to do the healing
	int ammoUsed = 0;
	if(m_iClip1 <= ammoUsed)
		ammoUsed = m_iClip1;
	else
		ammoUsed = eo_medkit_ammousage.GetInt();

	int heal = ammoUsed;

	if(pHeal == pOwner)
		heal = int((float)heal * eo_medkit_self_ammo_factor.GetFloat());
	else
	{
		// Give us some points based on the health we give
		pOwner->IncreaseOverlordDamage(eo_points_per_healing.GetInt());
	}

	if(heal <= 0)
		heal = 1;

	m_iClip1 -= ammoUsed;
	pHeal->TakeHealth(heal, DMG_GENERIC);	
#endif

}