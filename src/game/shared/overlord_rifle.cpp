//==============Overlord mod=====================
//	Power based rifle for the Ov
//	
//===============================================

#include "cbase.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbase_machinegun.h"
#include "hl2mp_gamerules.h"

ConVar eo_rifle_powerdrain("eo_rifle_powerdrain", "3", FCVAR_REPLICATED | FCVAR_CHEAT);

#define POWER_DRAIN eo_rifle_powerdrain.GetInt()

#ifdef CLIENT_DLL
#define COverlordRifle C_OverlordRifle
#endif


class COverlordRifle : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS(COverlordRifle, CHL2MPMachineGun);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();


	COverlordRifle();
	virtual ~COverlordRifle();

#ifndef CLIENT_DLL
	virtual void ItemPostFrame();
	virtual void FireBullets(const FireBulletsInfo_t &info);
#endif
	Activity GetPrimaryAttackActivity();

	const char *GetTracerType( void ) { return "AR2Tracer"; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		
		cone = VECTOR_CONE_5DEGREES;

		return cone;
	}

	virtual float	GetFireRate( void ) { return 0.08f; }

	virtual bool		  IsOverlordWeapon() const { return true; };
	virtual bool HasAnyAmmo() { return true; };
	virtual bool CanBeSelected() { return true; };

	virtual const char * GetPrimaryMode() const { return "Fire"; }; 
	virtual const char * GetSecondaryMode() const { return "None"; }; 
	virtual const char * GetCostLabel() const;

};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordRifle, DT_OverlordRifle )

BEGIN_NETWORK_TABLE( COverlordRifle, DT_OverlordRifle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( COverlordRifle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_rifle, COverlordRifle );
PRECACHE_WEAPON_REGISTER(weapon_rifle);


acttable_t	COverlordRifle::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_AR2,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_AR2,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_AR2,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_AR2,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_AR2,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_AR2,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_AR2,					false },
};

IMPLEMENT_ACTTABLE(COverlordRifle);


COverlordRifle::COverlordRifle()
{
}

COverlordRifle::~COverlordRifle()
{
}


Activity COverlordRifle::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}
#ifndef CLIENT_DLL
void COverlordRifle::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	COverlordData * pData = GET_OVERLORD_DATA;

	// Little hack, we can shoot all the time as we use power only
	if(m_iClip1 < GetMaxClip1() && pData->GetPower() >= POWER_DRAIN)
		m_iClip1 = GetMaxClip1();
	else if(m_iClip1 > 0 && pData->GetPower() < POWER_DRAIN)
		m_iClip1 = 0;
}

void COverlordRifle::FireBullets(const FireBulletsInfo_t &info)
{
	BaseClass::FireBullets(info);

	(GET_OVERLORD_DATA)->HandlePowerEvent(EVENT_WEAPONUSED, POWER_DRAIN);
}
#endif

const char * COverlordRifle::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "%i per shot", eo_rifle_powerdrain.GetInt());

	return label;
}