//==============Overlord mod=====================
//	Disabler weapon for the Operative
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbase_machinegun.h"
#include "overlord_dynamictraps.h"

#ifndef CLIENT_DLL
#include "overlord_emp.h"
#endif

#ifdef CLIENT_DLL
#define COverlordDisabler C_OverlordDisabler
#endif

ConVar eo_disabler_disable_shots("eo_disabler_disable_shots", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_disabler_disable_length("eo_disabler_disable_length", "2.15", FCVAR_REPLICATED | FCVAR_CHEAT);

#define DISABLE_SOUND "coast.combine_apc_shutdown"

class COverlordDisabler : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS(COverlordDisabler, CHL2MPMachineGun);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();
	DECLARE_ACTTABLE();

	COverlordDisabler();
	virtual ~COverlordDisabler();

	virtual void Precache();

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;
		
		cone = VECTOR_CONE_1DEGREES;

		return cone;
	}

#ifndef CLIENT_DLL
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
#endif

	virtual float GetFireRate() { return 0.165f; };
private:
#ifndef CLIENT_DLL
	int m_nShots;
	CHandle<COverlordTrap> m_hTrap;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordDisabler, DT_OverlordDisabler )

BEGIN_NETWORK_TABLE( COverlordDisabler, DT_OverlordDisabler )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( COverlordDisabler )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_disabler, COverlordDisabler );
PRECACHE_WEAPON_REGISTER( weapon_disabler );


acttable_t COverlordDisabler::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PISTOL,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PISTOL,					false },
};

IMPLEMENT_ACTTABLE( COverlordDisabler );

COverlordDisabler::COverlordDisabler()
{
#ifndef CLIENT_DLL
	m_nShots = 0;
	m_hTrap = NULL;
#endif
}

COverlordDisabler::~COverlordDisabler()
{
}

void COverlordDisabler::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther("grenade_emp");
#endif

	PrecacheScriptSound(DISABLE_SOUND);
}

#ifndef CLIENT_DLL
void COverlordDisabler::PrimaryAttack()
{
	// Base class condition needs to be used here as well
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		BaseClass::PrimaryAttack();
		return;
	}

	BaseClass::PrimaryAttack();

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	// Increment our shots ONLY if they've hit the same trap
	// as before
	
	// Launching a seperate trace here, might want to link it with
	// the fire bullets method later on
	Vector forw, end, start;
	start = pPlayer->Weapon_ShootPosition();
	AngleVectors(GetAbsAngles(), &forw);
	VectorMA(start, MAX_TRACE_LENGTH, forw, end);

	trace_t tr;
	UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), end, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	// We hit something! Quick, check!
	COverlordTrap * pTrap = COverlordTrap::EntityToTrap(tr.m_pEnt);
	if(pTrap)
	{
		// New trap, restart...
		if(pTrap != m_hTrap)
		{
			m_nShots = 0;
			m_hTrap = pTrap;
		}
		else
		{
			m_nShots++;

			if(m_nShots >= eo_disabler_disable_shots.GetInt())
			{
				// Now disable... and restart the shots
				if(pTrap->CanDisableTrap())
				{
					m_nShots = 0;
					pTrap->EmitSound(DISABLE_SOUND);
					pTrap->DisableTrap(gpGlobals->curtime + eo_disabler_disable_length.GetFloat());
				}
			}
		}
	}
}

void COverlordDisabler::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	//Must have ammo
	if ( ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 ) || ( pPlayer->GetWaterLevel() == 3 ) )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if( m_bInReload )
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( WPN_DOUBLE );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 1500.0f, vecThrow );
	
#ifndef CLIENT_DLL
	//Create the grenade
	COverlordEMP *pGrenade = (COverlordEMP*)Create( "grenade_emp", vecSrc, vec3_angle, pPlayer );
	pGrenade->SetAbsVelocity( vecThrow );

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
	pGrenade->SetThrower( GetOwner() );
	pGrenade->SetDamage( 0 );
	pGrenade->SetDamageRadius( eo_emp_radius.GetFloat() );
#endif

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}
#endif