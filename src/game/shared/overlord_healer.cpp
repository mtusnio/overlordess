//==============Overlord mod=====================
//	Healing weapon
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifdef CLIENT_DLL
#define COverlordHealer C_OverlordHealer
#endif

class COverlordHealer : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS(COverlordHealer, CBaseHL2MPBludgeonWeapon);
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	DECLARE_NETWORKCLASS(); 

	COverlordHealer();
	virtual ~COverlordHealer();

	virtual void SecondaryAttack() { return; };

	virtual	float GetDamageForActivity( Activity hitActivity )	{	return	0.0f;	}
	virtual float GetFireRate() { return 1.6f; };

#ifndef CLIENT_DLL
	virtual void ItemPostFrame();

	virtual void Drop(const Vector &vecVelocity)
	{
		if(m_bFrozen)
			Unfreeze();

		BaseClass::Drop(vecVelocity);
	}
#endif
private:
	virtual void Hit(trace_t &traceHit, Activity nHitActivity);
#ifndef CLIENT_DLL
	void Unfreeze();

	bool m_bFrozen;
	float m_flUnfreeze;
#endif

};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordHealer, DT_OverlordHealer)

BEGIN_NETWORK_TABLE(COverlordHealer, DT_OverlordHealer)
#ifdef CLIENT_DLL
	
#else

#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordHealer)

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_healer, COverlordHealer);
PRECACHE_WEAPON_REGISTER(weapon_healer);

acttable_t COverlordHealer::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

IMPLEMENT_ACTTABLE(COverlordHealer);


COverlordHealer::COverlordHealer()
{
#ifndef CLIENT_DLL
	m_bFrozen = false;
	m_flUnfreeze = 0.0f;
#endif
}

COverlordHealer::~COverlordHealer()
{
#ifndef CLIENT_DLL
	if(m_bFrozen)
		Unfreeze();
#endif
}

void COverlordHealer::Hit(trace_t &traceHit, Activity nHitActivity)
{
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(traceHit.m_pEnt);

	if(pPlayer)
	{
		float ratio = pPlayer->GetHealth() / pPlayer->GetMaxHealth();

		// Below 10%, consume
		if(ratio <= 0.1f)
		{
			CBasePlayer * pOwner = ToBasePlayer(GetOwner());

			if(pOwner)
			{
				const int health = pPlayer->GetMaxHealth();
				pOwner->TakeHealth(health, DMG_GENERIC);
				
				CTakeDamageInfo info( this, this, health+1, DMG_DISSOLVE);
				pPlayer->OnTakeDamage(info);
				//pPlayer->GetBaseAnimating()->Dissolve( "", gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				
				pOwner->AddFlag(FL_FROZEN);
				m_bFrozen = true;
				m_flUnfreeze = gpGlobals->curtime + 1.5f;
			}
		}
	}
#endif
}

#ifndef CLIENT_DLL
void COverlordHealer::ItemPostFrame()
{
	if(m_bFrozen && m_flUnfreeze != 0.0f && m_flUnfreeze <= gpGlobals->curtime)
	{
		Unfreeze();
	}
	BaseClass::ItemPostFrame();
}

void COverlordHealer::Unfreeze()
{
	if(m_bFrozen)
	{
		CBasePlayer * pOwner = ToBasePlayer(GetOwner());

		if(pOwner)
		{
			pOwner->RemoveFlag(FL_FROZEN);
		}

		m_bFrozen = false;
		m_flUnfreeze = 0.0f;
	}
}
#endif