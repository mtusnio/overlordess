//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "basegrenade_shared.h"
#endif

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
#define CWeaponSMG1 C_WeaponSMG1
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SMG1_GRENADE_DAMAGE 100.0f
#define SMG1_GRENADE_RADIUS 250.0f

ConVar eo_smg_burst("eo_smg_burst", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_smg_burst_speed("eo_smg_burst_speed", "0.10", FCVAR_REPLICATED | FCVAR_CHEAT);

#define FASTEST_REFIRE_RATE 0.15f

class CWeaponSMG1 : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS( CWeaponSMG1, CHL2MPMachineGun );

	CWeaponSMG1();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void	Precache( void );
	void	AddViewKick( void );

	void	PrimaryAttack(void);
	void	SecondaryAttack( void );

	virtual void ItemPostFrame();

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual int GetClassWeapon() const { return CLASS_ASSAULT; };

	virtual void Equip( CBaseCombatCharacter *pOwner );

	bool	Holster(CBaseCombatWeapon *pSwitchingTo);
	bool	Reload( void );

	float	GetFireRate( void ) { return 0.415f; }
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void );

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();


protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	
private:
	CWeaponSMG1( const CWeaponSMG1 & );

	CNetworkVar(int, m_iShots);
	CNetworkVar(float, m_flLastPrimaryAttack);
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSMG1, DT_WeaponSMG1 )

BEGIN_NETWORK_TABLE( CWeaponSMG1, DT_WeaponSMG1 )
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_iShots)),
	SendPropFloat(SENDINFO(m_flLastPrimaryAttack)),
#else
	RecvPropInt(RECVINFO(m_iShots)),
	RecvPropFloat(RECVINFO(m_flLastPrimaryAttack)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSMG1 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_smg1, CWeaponSMG1 );
PRECACHE_WEAPON_REGISTER(weapon_smg1);


acttable_t	CWeaponSMG1::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SMG1,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SMG1,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SMG1,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SMG1,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SMG1,					false },
};


IMPLEMENT_ACTTABLE(CWeaponSMG1);


//=========================================================
CWeaponSMG1::CWeaponSMG1( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;
	m_iShots = 0;
	m_flLastPrimaryAttack = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::Precache( void )
{
#ifndef CLIENT_DLL
	//UTIL_PrecacheOther("grenade_emp");
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSMG1::Equip( CBaseCombatCharacter *pOwner )
{
	m_fMaxRange1 = 1400;

	BaseClass::Equip( pOwner );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponSMG1::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}


const Vector& CWeaponSMG1::GetBulletSpread()
{
	static const Vector cone = VECTOR_CONE_1DEGREES;
	static const Vector burst = VECTOR_CONE_5DEGREES;

	if(m_iShots > 0)
		return burst;

	return cone;
}

bool CWeaponSMG1::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if(m_iShots > 0)
		return false;

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponSMG1::Reload( void )
{
	if(m_iShots > 0)
		return false;

	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound( RELOAD );
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG1::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

void CWeaponSMG1::PrimaryAttack()
{
	if(m_iShots > 0)
		return;

	BaseClass::PrimaryAttack();

	m_flLastPrimaryAttack = gpGlobals->curtime;
	m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
}
void CWeaponSMG1::SecondaryAttack()
{
	if(m_iShots > 0)
		return;

	BaseClass::PrimaryAttack();

	m_iShots++;

	m_flNextSecondaryAttack = gpGlobals->curtime + eo_smg_burst_speed.GetFloat() * eo_smg_burst.GetFloat() + FASTEST_REFIRE_RATE;
	m_flNextPrimaryAttack = gpGlobals->curtime + eo_smg_burst_speed.GetFloat();
}

void CWeaponSMG1::ItemPostFrame()
{
	if(m_iShots > 0 && m_iShots < eo_smg_burst.GetInt())
	{
		if((Clip1() <= 0) || m_bInReload)
		{
			m_iShots = 0;
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
			m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
		}
		else if(m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			BaseClass::PrimaryAttack();
			m_flNextPrimaryAttack = gpGlobals->curtime + eo_smg_burst_speed.GetFloat();
			m_iShots++;
		}
	}
	else if(m_iShots >= eo_smg_burst.GetInt())
	{
		m_iShots = 0;
	}

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if((m_iShots == 0) 
		&& pPlayer 
		&& ((pPlayer->m_nButtons & IN_ATTACK) == false) 
		&& (m_flLastPrimaryAttack + FASTEST_REFIRE_RATE <= gpGlobals->curtime))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime;
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponSMG1::SecondaryAttack( void )
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
	VectorScale( vecThrow, 1000.0f, vecThrow );
	
#ifndef CLIENT_DLL
	//Create the grenade
	COverlordEMP *pGrenade = (COverlordEMP*)Create( "grenade_emp", vecSrc, vec3_angle, pPlayer );
	pGrenade->SetAbsVelocity( vecThrow );

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
	pGrenade->SetThrower( GetOwner() );
	pGrenade->SetDamage( SMG1_GRENADE_DAMAGE );
	pGrenade->SetDamageRadius( SMG1_GRENADE_RADIUS );
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
}*/

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSMG1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
