//==============Overlord mod=====================
//	Ammo resupply kit
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordAmmoKit C_OverlordAmmoKit
#endif

#define AMMOKIT_SOUND "BaseCombatCharacter.AmmoPickup"
#define AMMOKIT_RANGE 82.0f

class COverlordAmmoKit : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordAmmoKit, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
	#endif

	COverlordAmmoKit();
	virtual ~COverlordAmmoKit();

	virtual void PrimaryAttack();

	virtual bool		  IsClassWeapon() const { return true; };
	virtual PlayerClass_t GetClassWeapon() const { return CLASS_HACKER; };
	virtual bool HasAnyAmmo() { return true; };
	virtual bool CanBeSelected() { return true; };

};


IMPLEMENT_NETWORKCLASS_ALIASED(OverlordAmmoKit, DT_OverlordAmmoKit)

BEGIN_NETWORK_TABLE( COverlordAmmoKit, DT_OverlordAmmoKit)

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordAmmoKit )


END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_ammokit, COverlordAmmoKit);


PRECACHE_WEAPON_REGISTER( weapon_ammokit );

#ifndef CLIENT_DLL

acttable_t	COverlordAmmoKit::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_RPG,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_RPG,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_RPG,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_RPG,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_RPG,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_RPG,				false },
};

IMPLEMENT_ACTTABLE(COverlordAmmoKit);

#endif

COverlordAmmoKit::COverlordAmmoKit()
{
}

COverlordAmmoKit::~COverlordAmmoKit()
{
}

void COverlordAmmoKit::PrimaryAttack()
{
	CBasePlayer * pOwner = ToBasePlayer(GetOwner());
	if(!pOwner)
		return;

	if(!HasAmmo())
		return;

	Vector vSrc = pOwner->EyePosition();
	Vector vForw;
	Vector vEnd;
	AngleVectors(EyeAngles(), &vForw);
	VectorMA(vSrc, AMMOKIT_RANGE, vForw, vEnd);
	
	trace_t tr;
	UTIL_TraceLine(vSrc, vEnd, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	// We hit a player, resupply!
	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
#ifndef CLIENT_DLL
		CBasePlayer * pPlayer = static_cast<CBasePlayer*>(tr.m_pEnt);
		
		pPlayer->GiveAmmo(2, "357", true);
		pPlayer->GiveAmmo(50, "Pistol", true);
		pPlayer->GiveAmmo(80, "SMG1", true);
		pPlayer->GiveAmmo( 1,	"grenade", true );
		pPlayer->GiveAmmo( 3, "Buckshot", true);
		pOwner->IncreaseOverlordDamage(8);
#endif
		m_iClip1--;
		EmitSound(AMMOKIT_SOUND);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.8f;
}