//==============Overlord mod=====================
// Smoke grenade
//===============================================

#include "cbase.h"

#if 0

#include "basegrenade_shared.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
#define COverlordSmokeNade C_OverlordSmokeNade
#endif

#define SMOKEGRENADE_MODEL "models/Weapons/w_grenade.mdl"
#ifndef CLIENT_DLL
#define SMOKE_PARTICLE "SmokeGrenadeParticle"

// Our smoke grenade class for the weapon
class COverlordSmoke : public CBaseGrenade
{
public:
	DECLARE_CLASS(COverlordSmoke, CBaseGrenade);
	DECLARE_DATADESC();

	COverlordSmoke();
	virtual ~COverlordSmoke() { };

	virtual void Precache();

	virtual void Spawn()
	{
		BaseClass::Spawn();

		SetSolid( SOLID_VPHYSICS );

		SetModel(SMOKEGRENADE_MODEL);

		SetNextThink(gpGlobals->curtime + 0.1f);

		CreateVPhysics();
	}

	bool CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_BBOX, 0, false );
		return true;
	}

	virtual void		  Detonate()
	{
		SetThink(&COverlordSmoke::Think);
		SetNextThink(gpGlobals->curtime + 0.1f);
	}
	virtual void		  Think();

	void SetTimer(float delay);

	virtual void Explode( trace_t *pTrace, int bitsDamageType )
	{
		return;
	}

private:
	float m_flSmokeTime;
	int   m_iSmoked;
};

BEGIN_DATADESC(COverlordSmoke)

	

END_DATADESC()

COverlordSmoke::COverlordSmoke()
{ 
	m_flSmokeTime = 0.0f;
	m_iSmoked = 0;
}

void COverlordSmoke::Precache()
{
	BaseClass::Precache();
	PrecacheModel(SMOKEGRENADE_MODEL);
	PrecacheParticleSystem(SMOKE_PARTICLE);
}

void COverlordSmoke::Think()
{
	const int SMOKE_COUNT = 5;

	if(m_iSmoked < SMOKE_COUNT && m_flSmokeTime <= gpGlobals->curtime)
	{
		m_iSmoked++;
		m_flSmokeTime = gpGlobals->curtime + 1.5f;
		//UTIL_Smoke(GetAbsOrigin(), random->RandomInt(30, 40), 10);
		DispatchParticleEffect(SMOKE_PARTICLE, WorldSpaceCenter(), QAngle(0, 0, 0));
	}
	else if(m_iSmoked >= SMOKE_COUNT && m_flSmokeTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		return;
	}
	
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void COverlordSmoke::SetTimer(float delay)
{
	m_flSmokeTime = gpGlobals->curtime + delay;

	SetThink(&COverlordSmoke::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

// Create function
CBaseGrenade * Smokegrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned )
{
	COverlordSmoke * pGrenade = static_cast<COverlordSmoke*>(CBaseEntity::Create( "grenade_smoke", position, angles, pOwner ));
	
	if(!pGrenade)
		return NULL;

	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetTimer(3.0f);
	IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}

	return pGrenade;
}

LINK_ENTITY_TO_CLASS(grenade_smoke, COverlordSmoke);
#endif

class COverlordSmokeNade : public CWeaponFrag
{
public:
	DECLARE_CLASS(COverlordSmokeNade, CWeaponFrag);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();


	DECLARE_ACTTABLE();


	COverlordSmokeNade();
	virtual ~COverlordSmokeNade();

	virtual bool		  IsClassWeapon() const { return true; };
	virtual PlayerClass_t GetClassWeapon() const { return CLASS_STEALTH; };
};
#ifndef CLIENT_DLL

acttable_t	COverlordSmokeNade::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(COverlordSmokeNade);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordSmokeNade, DT_OverlordSmokeNade )

BEGIN_NETWORK_TABLE( COverlordSmokeNade, DT_OverlordSmokeNade )

#ifdef CLIENT_DLL

#else

#endif
	
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordSmokeNade )

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_smokenade, COverlordSmokeNade );
PRECACHE_WEAPON_REGISTER(weapon_smokenade);

COverlordSmokeNade::COverlordSmokeNade()
{
#ifndef CLIENT_DLL
	m_GrenadeCreate = &Smokegrenade_Create;
#endif
}

COverlordSmokeNade::~COverlordSmokeNade()
{
}

#endif