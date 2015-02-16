//==============Overlord mod=====================
//	Operative's sniper rifle
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordSniper C_OverlordSniper
#endif

#ifndef CLIENT_DLL
extern ConVar hl2_normspeed;
#endif

#define SNIPER_SPEED 150

class COverlordSniper : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( COverlordSniper, CBaseHL2MPCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	COverlordSniper();
	virtual ~COverlordSniper();

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;

		if(m_bZoomed)
		{
			CBasePlayer * pOwner = ToBasePlayer(GetOwner());

			if(pOwner)
			{
				//Vector velocity = pOwner->GetAbsVelocity();
				//if(velocity.Length() > 1.0f)
				//	cone = VECTOR_CONE_6DEGREES;
				//else
					cone = Vector(0, 0, 0);
			}
			else
			{
				cone = Vector(0, 0, 0);
			}
		}
		else
		{
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}

	virtual PlayerClass_t GetClassWeapon() const { return CLASS_STEALTH; };

	virtual float GetFireRate() { return 0.275f; };

	// Zoom in
	virtual void SecondaryAttack();

	virtual bool Reload();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo);
	virtual void Drop(const Vector &vecVelocity);


private:
	void Zoom(bool bZoom);

	CNetworkVar(bool, m_bZoomed);
};


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordSniper, DT_OverlordSniper )

BEGIN_NETWORK_TABLE( COverlordSniper, DT_OverlordSniper )
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bZoomed)),
#else
	RecvPropBool(RECVINFO(m_bZoomed)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( COverlordSniper )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sniper, COverlordSniper );
PRECACHE_WEAPON_REGISTER(weapon_sniper);

acttable_t	COverlordSniper::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_CROSSBOW,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_CROSSBOW,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_CROSSBOW,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_CROSSBOW,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_CROSSBOW,					false },
};

IMPLEMENT_ACTTABLE(COverlordSniper);

COverlordSniper::COverlordSniper()
{
	m_bZoomed = false;
}

COverlordSniper::~COverlordSniper()
{
	Zoom(false);
}

void COverlordSniper::SecondaryAttack()
{
	Zoom(!m_bZoomed);

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}

void COverlordSniper::Zoom(bool bZoom)
{
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	if ( !bZoom )
	{
		if ( pPlayer->SetFOV( this, 0, 0.2f ) )
		{
			pPlayer->RemoveSlowdown(this);

			m_bZoomed = false;
		}
	}
	else
	{
		if ( pPlayer->SetFOV( this, 20, 0.1f ) )
		{
			pPlayer->AddSlowdown(SNIPER_SPEED, -1.0f, this);

			m_bZoomed = true;
		}
	}
#endif
}

bool COverlordSniper::Reload()
{
	if(m_bZoomed)
		Zoom(false);

	return BaseClass::Reload();
}

bool COverlordSniper::Holster(CBaseCombatWeapon * pSwitchingTo)
{
	Zoom(false);

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordSniper::Drop(const Vector &vecVelocity)
{
	Zoom(false);

	BaseClass::Drop(vecVelocity);
}