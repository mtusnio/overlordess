//==============Overlord mod=====================
//	Dynamic traps defense systems
//===============================================

#include "cbase.h"
#include "overlord_dynamictraps_defenses.h"
#include "overlord_dynamictraps.h"
#include "particle_parse.h"

#ifndef CLIENT_DLL
#include "team.h"
#include "hl2mp_gamerules.h"
#include "gib.h"
#include "grenade_frag.h"
#include "basegrenade_shared.h"
#endif

ConVar eo_trapsdefense_smoke_chance("eo_trapsdefense_smoke_chance", "21", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_gas_chance("eo_trapsdefense_gas_chance", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_chance("eo_trapsdefense_vortex_chance", "16", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_trapsdefense_spikes_chance("eo_trapsdefense_spikes_chance", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_chance("eo_trapsdefense_reversevortex_chance", "22", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_chance("eo_trapsdefense_grenades_chance", "27", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_randomtrap_chance("eo_trapsdefense_randomtrap_chance", "20", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_slowdown_chance("eo_trapsdefense_slowdown_chance", "15", FCVAR_REPLICATED | FCVAR_CHEAT);

// We keep the projectile quite high to assure we got some mobility in our traps...
ConVar eo_trapsdefense_projectile_chance("eo_trapsdefense_projectile_chance", "90", FCVAR_REPLICATED | FCVAR_CHEAT);

namespace
{
	// We need this array for some calculations
	// The order must be just like the enum's!
	ConVar * s_ConVars[] = 
	{
		&eo_trapsdefense_smoke_chance,
		&eo_trapsdefense_gas_chance,
		&eo_trapsdefense_vortex_chance,
		&eo_trapsdefense_projectile_chance,
		//&eo_trapsdefense_spikes_chance,
		&eo_trapsdefense_reversevortex_chance,
		&eo_trapsdefense_grenades_chance,
		&eo_trapsdefense_randomtrap_chance,
		&eo_trapsdefense_slowdown_chance,
	};
}

void CreateSmoke(const Vector & pos, const QAngle & angle);
void CreateGas(const Vector & pos, const QAngle & angle);
void CreateVortex(const Vector & pos, const QAngle & angle);
void CreateSpikes(const Vector & pos, const QAngle & angle);
void CreateReverseVortex(const Vector & pos, const QAngle & angle);
void CreateProjectile(const Vector & pos, const QAngle & angle, COverlordTrap * pTrap);
void CreateGrenades(const Vector & pos, const QAngle & angle);
void CreateRandomTrap(const Vector & pos, const QAngle & angle, COverlordTrap * pTrap);
void CreateSlowdown(const Vector & pos, const QAngle & angle);

void CreateSelectedType(DefenseType finalType, const Vector & pos, const QAngle & angle, COverlordTrap * pTrap)
{
	switch(finalType)
	{
	case SMOKE:
		CreateSmoke(pos, angle);
		break;
	case GAS:
		CreateGas(pos, angle);
		break;
	case VORTEX:
		CreateVortex(pos, angle);
		break;
	//case SPIKES:
	//	CreateSpikes(pos, angle);
	//	break;
	case PROJECTILE:
		CreateProjectile(pos, angle, pTrap);
		break;
	case REVERSE_VORTEX:
		CreateReverseVortex(pos, angle);
		break;
	case GRENADES:
		CreateGrenades(pos, angle);
		break;
	case RANDOMTRAP:
		CreateRandomTrap(pos, angle, pTrap);
		break;
	case SLOWDOWN:
		CreateSlowdown(pos, angle);
		break;
	}
}

DefenseType SelectRandomDefense()
{
	DefenseType finalType = RANDOM;

	int fullChance = 0;

	// Fill the chance integer
	for(int i = 0; i < ARRAYSIZE(s_ConVars); i++)
	{
		ConVar * var = s_ConVars[i];

		if(!var)
			continue;	

		fullChance += var->GetInt();
	}

	int rand = random->RandomInt(0, fullChance);

	// Min-max to make it truly random...
	int max = 0;
	int min = 0;
	for(int i = 0; i < ARRAYSIZE(s_ConVars); i++)
	{
		ConVar * var = s_ConVars[i];

		if(!var)
			continue;

		max += var->GetInt();
		
		if((rand <= max) && (rand >= min))
		{
			if((RANDOM + 1 + i) > (MAX_TYPES - 1))
				break;

			// random is always the first one
			finalType = static_cast<DefenseType>(RANDOM + 1 + i);

			min += var->GetInt();

			break;
		}
	}

	return finalType;
}

void CreateTrapDefense(const Vector & pos, const QAngle & angle, COverlordTrap * pTrap, const DefenseType type)
{
	DefenseType finalType = type;
	if(type == RANDOM)
	{
		finalType = SelectRandomDefense();
	}
	// Now is the time to begin the test proper
	if(finalType != RANDOM)
	{
		CreateSelectedType(finalType, pos, angle, pTrap);
	}
	
	
}

//===================================================================================================
//===================================================================================================
//===================================================================================================

#ifndef CLIENT_DLL

#define GAS_PARTICLE "GasSmoke"

ConVar eo_trapsdefense_gas_range("eo_trapsdefense_gas_range", "292", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_gas_damage("eo_trapsdefense_gas_damage", "9", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_gas_damage_interval("eo_trapsdefense_gas_damage_interval", "0.8", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_gas_lifetime("eo_trapsdefense_gas_lifetime", "10.0", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordDefenseGas : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordDefenseGas, CBaseEntity);

	COverlordDefenseGas();
	virtual ~COverlordDefenseGas();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

private:
	float m_flSpawnTime;
};

LINK_ENTITY_TO_CLASS(defense_gas, COverlordDefenseGas);

COverlordDefenseGas::COverlordDefenseGas()
{
	m_flSpawnTime = 0.0f;
}

COverlordDefenseGas::~COverlordDefenseGas()
{
}

void COverlordDefenseGas::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(GAS_PARTICLE);
}

void COverlordDefenseGas::Spawn()
{
	BaseClass::Spawn();

	Precache();

	m_flSpawnTime = gpGlobals->curtime + eo_trapsdefense_gas_lifetime.GetFloat();

	SetSolid(SOLID_NONE);

	SetThink(&COverlordDefenseGas::Think);

	SetNextThink(gpGlobals->curtime + 0.1f);

	SetOwnerEntity(GetOverlordData()->GetOverlord());
}

void COverlordDefenseGas::Think()
{
	if(m_flSpawnTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		return;
	}

	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsAlive() || (!pPlayer->IsRebel() && !pPlayer->IsOverlord()))
			continue;

		float dist = (GetAbsOrigin() - pPlayer->WorldSpaceCenter()).Length();

		if(dist > eo_trapsdefense_gas_range.GetFloat())
			continue;

		trace_t tr;
		CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
		UTIL_TraceLine(GetAbsOrigin(), pPlayer->WorldSpaceCenter(), MASK_SHOT, &filter, &tr);

		if(tr.fraction >= 1.0f)
		{
			// Damage the player
			CTakeDamageInfo info(this, this, eo_trapsdefense_gas_damage.GetFloat(), DMG_NERVEGAS);
			pPlayer->OnTakeDamage(info);
		}
	}

	SetNextThink(gpGlobals->curtime + eo_trapsdefense_gas_damage_interval.GetFloat());
}

#ifndef CLIENT_DLL

// Lets us handle which entites should never be targeted for any of all the vortex types
bool CanVortexTarget(CBaseEntity * pEnt)
{
	if(!pEnt)
		return false;

	if(FClassnameIs(pEnt, "func_door"))
		return false;

	if(FClassnameIs(pEnt, "func_door_rotating"))
		return false;

	return true;
}

#endif

#define VORTEX_PARTICLE "Vortex"
#define VORTEX_SOUND "k_lab.mini_teleport_warmup"

ConVar eo_trapsdefense_vortex_lifetime("eo_trapsdefense_vortex_lifetime", "10.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_range("eo_trapsdefense_vortex_range", "512.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_damage_range("eo_trapsdefense_vortex_damage_range", "256.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_speed_multiplier("eo_trapsdefense_vortex_speed_multiplier", "0.815", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_damage_interval("eo_trapsdefense_vortex_damage_interval", "1.00", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_vortex_damage("eo_trapsdefense_vortex_damage", "7", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordDefenseVortex : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordDefenseVortex, CBaseEntity);

	COverlordDefenseVortex();
	virtual ~COverlordDefenseVortex();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();
private:
	float m_flLifeTime;
	float m_flDamageInterval;
};

LINK_ENTITY_TO_CLASS(defense_vortex, COverlordDefenseVortex);

COverlordDefenseVortex::COverlordDefenseVortex()
{
	m_flLifeTime = 0.0f;
	m_flDamageInterval = 0.0f;
}

COverlordDefenseVortex::~COverlordDefenseVortex()
{
	StopSound(VORTEX_SOUND);
}

void COverlordDefenseVortex::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(VORTEX_PARTICLE);
	PrecacheScriptSound(VORTEX_SOUND);
}

void COverlordDefenseVortex::Spawn()
{
	BaseClass::Spawn();

	Precache();

	m_flLifeTime = gpGlobals->curtime + eo_trapsdefense_vortex_lifetime.GetFloat();

	EmitSound(VORTEX_SOUND);

	SetThink(&COverlordDefenseVortex::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_flDamageInterval = gpGlobals->curtime + eo_trapsdefense_vortex_damage_interval.GetFloat();

	SetOwnerEntity(GetOverlordData()->GetOverlord());
}

void COverlordDefenseVortex::Think()
{
	if(m_flLifeTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		return;
	}

	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), GetAbsOrigin(), eo_trapsdefense_vortex_range.GetFloat(), 0);
	
	for(int i = 0; i < count; i++)
	{
		CBaseEntity * pEntity = pList[i];

		if(!pEntity)
			continue;

		float dist = (GetAbsOrigin() - pEntity->WorldSpaceCenter()).Length();

		if(dist > eo_trapsdefense_vortex_range.GetFloat())
			continue;

		if(pEntity->GetFlags() & FL_FROZEN)
			continue;

		trace_t tr;
		CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
		UTIL_TraceLine(GetAbsOrigin(), pEntity->WorldSpaceCenter(), (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE), &filter, &tr);

		if(tr.fraction >= 1.0f || tr.m_pEnt == pEntity)
		{
			// Handle any player attacks etc.
			if(pEntity->IsPlayer())
			{
				CBasePlayer * pPlayer = ToBasePlayer(pEntity);
				if(!pPlayer || !pPlayer->IsAlive() || (!pPlayer->IsRebel() && !pPlayer->IsOverlord()))
					continue;

				if((m_flDamageInterval <= gpGlobals->curtime) &&
					dist <= eo_trapsdefense_vortex_damage_range.GetFloat())
				{
					CTakeDamageInfo info(this, this, eo_trapsdefense_vortex_damage.GetFloat(), DMG_NERVEGAS);
					pPlayer->OnTakeDamage(info);
				}
			}
			else
			{
				// Must be here, otherwise might crash!!
				if(!pEntity->VPhysicsGetObject())
					continue;
			}

			if(!CanVortexTarget(pEntity))
				continue;

			Vector dir = pEntity->WorldSpaceCenter() - GetAbsOrigin();
			VectorNormalize(dir);
			//dir *= eo_trapsdefense_vortex_range.GetFloat();

			Vector end;
			VectorMA(GetAbsOrigin(), eo_trapsdefense_vortex_range.GetFloat(), dir, end); 
			//Vector end = GetAbsOrigin() + dir;
			
			Vector force = (end - pEntity->WorldSpaceCenter()) * eo_trapsdefense_vortex_speed_multiplier.GetFloat();
			force = -force;

			if(force.LengthSqr() >= pEntity->GetAbsVelocity().LengthSqr())
				pEntity->ApplyAbsVelocityImpulse(force);		
		}
	}

	if(m_flDamageInterval <= gpGlobals->curtime)
		m_flDamageInterval = gpGlobals->curtime + eo_trapsdefense_vortex_damage_interval.GetFloat();


	SetNextThink(gpGlobals->curtime + 0.07f);
}



#define REVERSE_VORTEX_PARTICLE "ReverseVortex"


#define VORTEX_SOUND "k_lab.mini_teleport_warmup"

ConVar eo_trapsdefense_reversevortex_lifetime("eo_trapsdefense_reversevortex_lifetime", "10.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_range("eo_trapsdefense_reversevortex_range", "512.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_damage_range("eo_trapsdefense_reversevortex_damage_range", "308.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_speed_multiplier("eo_trapsdefense_reversevortex_speed_multiplier", "0.80", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_damage_interval("eo_trapsdefense_reversevortex_damage_interval", "0.70", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_reversevortex_damage("eo_trapsdefense_reversevortex_damage", "9", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordDefenseReverseVortex : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordDefenseReverseVortex, CBaseEntity);

	COverlordDefenseReverseVortex();
	virtual ~COverlordDefenseReverseVortex();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

	
private:
	float m_flLifeTime;
	float m_flDamageInterval;
};

LINK_ENTITY_TO_CLASS(defense_reversevortex, COverlordDefenseReverseVortex);

COverlordDefenseReverseVortex::COverlordDefenseReverseVortex()
{
	m_flLifeTime = 0.0f;
	m_flDamageInterval = 0.0f;
}

COverlordDefenseReverseVortex::~COverlordDefenseReverseVortex()
{
	StopSound(VORTEX_SOUND);
}

void COverlordDefenseReverseVortex::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(VORTEX_PARTICLE);
	PrecacheScriptSound(VORTEX_SOUND);
}

void COverlordDefenseReverseVortex::Spawn()
{
	BaseClass::Spawn();

	Precache();

	m_flLifeTime = gpGlobals->curtime + eo_trapsdefense_reversevortex_lifetime.GetFloat();

	EmitSound(VORTEX_SOUND);

	SetThink(&COverlordDefenseVortex::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_flDamageInterval = gpGlobals->curtime + eo_trapsdefense_reversevortex_damage_interval.GetFloat();

	SetOwnerEntity(GetOverlordData()->GetOverlord());
}

void COverlordDefenseReverseVortex::Think()
{
	if(m_flLifeTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		return;
	}

	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), GetAbsOrigin(), eo_trapsdefense_reversevortex_range.GetFloat(), 0);
	
	for(int i = 0; i < count; i++)
	{
		CBaseEntity * pEntity = pList[i];

		if(!pEntity)
			continue;

		float dist = (GetAbsOrigin() - pEntity->WorldSpaceCenter()).Length();

		if(dist > eo_trapsdefense_reversevortex_range.GetFloat())
			continue;

		if(pEntity->GetFlags() & FL_FROZEN)
			continue;

		trace_t tr;
		CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
		UTIL_TraceLine(GetAbsOrigin(), pEntity->WorldSpaceCenter(), (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE), &filter, &tr);

		if(tr.fraction >= 1.0f || tr.m_pEnt == pEntity)
		{
			// Handle any player attacks etc.
			if(pEntity->IsPlayer())
			{
				CBasePlayer * pPlayer = ToBasePlayer(pEntity);
				if(!pPlayer || !pPlayer->IsAlive() || (!pPlayer->IsRebel() && !pPlayer->IsOverlord()))
					continue;

				if((m_flDamageInterval <= gpGlobals->curtime) && 
					dist <= eo_trapsdefense_reversevortex_damage_range.GetFloat())
				{
					CTakeDamageInfo info(this, this, eo_trapsdefense_reversevortex_damage.GetFloat(), DMG_NERVEGAS);
					pPlayer->OnTakeDamage(info);
				}
			}
			else
			{
				// Must be here, otherwise might crash!!
				if(!pEntity->VPhysicsGetObject())
					continue;
			}

			if(!CanVortexTarget(pEntity))
				continue;

			Vector dir = pEntity->WorldSpaceCenter() - GetAbsOrigin();
			VectorNormalize(dir);
			//dir *= eo_trapsdefense_reversevortex_range.GetFloat();

			Vector end;
			VectorMA(GetAbsOrigin(), eo_trapsdefense_reversevortex_range.GetFloat(), dir, end); 
			//Vector end = GetAbsOrigin() + dir;
			
			Vector force = (end - pEntity->WorldSpaceCenter()) * eo_trapsdefense_reversevortex_speed_multiplier.GetFloat();

			if(force.LengthSqr() >= pEntity->GetAbsVelocity().LengthSqr())
				pEntity->ApplyAbsVelocityImpulse(force);		
		}
	}

	if(m_flDamageInterval <= gpGlobals->curtime)
		m_flDamageInterval = gpGlobals->curtime + eo_trapsdefense_reversevortex_damage_interval.GetFloat();


	SetNextThink(gpGlobals->curtime + 0.07f);
}

#endif

//===================================================================================================
//===================================================================================================
//===================================================================================================

void CreateSmoke(const Vector & pos, const QAngle & angle)
{
	DispatchParticleEffect("ExplosionSmoke", pos, angle);
}

void CreateGas(const Vector & pos, const QAngle & angle)
{
#ifndef CLIENT_DLL
	CBaseEntity * pEnt = CreateEntityByName("defense_gas");

	if(pEnt)
	{
		pEnt->SetAbsOrigin(pos);
		pEnt->SetAbsAngles(angle);
		pEnt->Spawn();
		DispatchParticleEffect(GAS_PARTICLE, pos, angle);
	}
	else
		Warning("Cannot create gas defense\n");
#endif
}

void CreateVortex(const Vector & pos, const QAngle & angle)
{
#ifndef CLIENT_DLL
	CBaseEntity * pEnt = CreateEntityByName("defense_vortex");

	if(pEnt)
	{
		pEnt->SetAbsOrigin(pos);
		pEnt->SetAbsAngles(angle);
		pEnt->Spawn();
		DispatchParticleEffect(VORTEX_PARTICLE, pos, angle);
	}
	else
		Warning("Cannot create vortex defense\n");
#endif
}

#ifndef CLIENT_DLL
ConVar eo_trapsdefense_projectile_minforward("eo_trapsdefense_projectile_minforward", "200", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_maxforward("eo_trapsdefense_projectile_maxforward", "1000", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_minside("eo_trapsdefense_projectile_minside", "-1250", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_maxside("eo_trapsdefense_projectile_maxside", "1250", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_health("eo_trapsdefense_projectile_health", "60", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_activation_range("eo_trapsdefense_projectile_activation_range", "312", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_time("eo_trapsdefense_projectile_time", "1.9", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_trapsdefense_projectile_lifetime("eo_trapsdefense_projectile_lifetime", "45", FCVAR_CHEAT | FCVAR_REPLICATED);

#define PROJECTILE_MODEL "models/combine_helicopter/helicopter_bomb01.mdl"
#define PROJECTILE_THINK 0.1f
#define PROJECTILE_RANGE eo_trapsdefense_projectile_activation_range.GetFloat()
#define PROJECTILE_BREAK_SOUND "Metal_Box.Break"
#define PROJECTILE_LIFETIME eo_trapsdefense_projectile_lifetime.GetFloat()

class COverlordDefenseProjectile : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordDefenseProjectile, CBaseAnimating);
	DECLARE_DATADESC();

	COverlordDefenseProjectile();
	virtual ~COverlordDefenseProjectile();

	virtual void Precache();
	virtual void Spawn();
	virtual bool CreateVPhysics() { VPhysicsInitNormal( SOLID_VPHYSICS, 0, false ); return true; };

	virtual void Think();

	virtual void Event_Killed(const CTakeDamageInfo &info);

	//virtual void ProjectileTouch( CBaseEntity *pOther );

	void SetGracePeriod(float time);

private:
	void ReleaseDefense();
	void CreateEffects();

	float m_flGraceEnd;
	float m_flCanBreak;
	float m_flDie;
};

BEGIN_DATADESC( COverlordDefenseProjectile )

	//DEFINE_FUNCTION( ProjectileTouch ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(defense_projectile, COverlordDefenseProjectile);

COverlordDefenseProjectile::COverlordDefenseProjectile()
{
	m_flGraceEnd = 0.0f;
	m_flCanBreak = 0.0f;
	m_flDie = 0.0f;
}

COverlordDefenseProjectile::~COverlordDefenseProjectile()
{
}

void COverlordDefenseProjectile::Precache()
{
	BaseClass::Precache();

	PrecacheModel(PROJECTILE_MODEL);

	PrecacheScriptSound(PROJECTILE_BREAK_SOUND);

	PrecacheModel("models/gibs/metal_gib1.mdl");
	PrecacheModel("models/gibs/metal_gib2.mdl");
	PrecacheModel("models/gibs/metal_gib3.mdl");
	PrecacheModel("models/gibs/metal_gib4.mdl");
	PrecacheModel("models/gibs/metal_gib5.mdl");
}

void COverlordDefenseProjectile::Spawn()
{
	Precache();

	SetModel(PROJECTILE_MODEL);

	SetMoveType(MOVETYPE_VPHYSICS);

	SetThink(&COverlordDefenseProjectile::Think);

	SetNextThink(gpGlobals->curtime + PROJECTILE_THINK);

	SetSolid(SOLID_VPHYSICS);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	CreateVPhysics();

	//SetTouch(&COverlordDefenseProjectile::ProjectileTouch);

	/*if(!VPhysicsGetObject())
	{
		Warning("No physics object for the projectile, deleting!\n");
		UTIL_Remove(this);
		return;
	}*/

	m_iHealth = m_iMaxHealth = eo_trapsdefense_projectile_health.GetInt();

	m_takedamage = DAMAGE_YES;

	m_flCanBreak = gpGlobals->curtime + eo_trapsdefense_projectile_time.GetFloat();

	m_flDie = gpGlobals->curtime + PROJECTILE_LIFETIME;

	SetOwnerEntity(GetOverlordData()->GetOverlord());
}

void COverlordDefenseProjectile::Think()
{
	if(m_flGraceEnd <= gpGlobals->curtime)
	{
		m_flGraceEnd = 0.0f;

		RemoveSolidFlags(FSOLID_NOT_SOLID);
		m_takedamage = DAMAGE_YES;
	}

	if(m_flDie <= gpGlobals->curtime)
	{
		ReleaseDefense();
		return;
	}

	if(m_flCanBreak <= gpGlobals->curtime)
	{

		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		if(pRebels)
		{
			for(int i = 0; i < pRebels->GetNumPlayers(); i++)
			{
				CBasePlayer * pPlayer = pRebels->GetPlayer(i);

				if(!pPlayer)
					continue;

				if(pPlayer->IsDead() || pPlayer->IsInvisible())
					continue;

				if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= PROJECTILE_RANGE)
				{
					trace_t tr;
					UTIL_TraceLine(WorldSpaceCenter(), pPlayer->EyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

					// Do it again, this time aim for another location
					if(tr.m_pEnt != pPlayer)
						UTIL_TraceLine(GetAbsOrigin(), pPlayer->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

					if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
					{
						ReleaseDefense();
					}
				}
			}
		}
	}

	SetNextThink(gpGlobals->curtime + PROJECTILE_THINK);
}

void COverlordDefenseProjectile::Event_Killed(const CTakeDamageInfo &info)
{
	ReleaseDefense();

	BaseClass::Event_Killed(info);
}

/*void COverlordDefenseProjectile::ProjectileTouch(CBaseEntity * pOther)
{
	if(m_flGraceEnd > gpGlobals->curtime)
		return;

	if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) )
		return;

	ReleaseDefense();

	UTIL_Remove(this);
}*/

void COverlordDefenseProjectile::SetGracePeriod(float time)
{
	m_flGraceEnd = gpGlobals->curtime + time;

	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	SetNextThink(m_flGraceEnd);
}

void COverlordDefenseProjectile::ReleaseDefense()
{
	CreateEffects();

	// Init it as a projectile
	DefenseType type = PROJECTILE;

	// Make sure it's not a projectile!
	for(int i = 0; i < 128; i++)
	{
		type = SelectRandomDefense();

		if(type != PROJECTILE)
			break;
	}

	// If the chances for projectile are high even with 128 iterations
	// we might not get a different defense. That's why we just use
	// usual random function
	int i = 0;
	while((type == PROJECTILE) && i < 64)
	{
		i++;
		type = static_cast<DefenseType>(random->RandomInt(RANDOM+1, MAX_TYPES-1));
	}

	CreateSelectedType(type, WorldSpaceCenter(), QAngle(0, 0, 0), NULL);

	UTIL_Remove(this);
}

void COverlordDefenseProjectile::CreateEffects()
{
	EmitSound(PROJECTILE_BREAK_SOUND);

	CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib1.mdl" );
	CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib2.mdl" );
	CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib3.mdl" );
	CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib4.mdl" );
	CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib5.mdl" );
}
#endif

void CreateProjectile(const Vector & pos, const QAngle & angle, COverlordTrap * pTrap)
{
#ifndef CLIENT_DLL
	if(!pTrap)
		return;

	COverlordDefenseProjectile * pDefense = static_cast<COverlordDefenseProjectile*>(CreateEntityByName("defense_projectile"));

	if(pDefense)
	{
		// Get plane and velocity here before we spawn the projectile!
		Vector start = pTrap->WorldSpaceCenter();

		Vector dir = pTrap->GetAbsOrigin() - start;
		VectorNormalize(dir);

		Vector end;

		VectorMA(start, MAX_TRACE_LENGTH, dir, end);

		trace_t tr;
		UTIL_TraceLine(start, end, MASK_SHOT_HULL, pTrap, COLLISION_GROUP_NONE, &tr);

		Vector forward, up, right;
		// Use some treshold for moving traps
		if((tr.endpos - tr.startpos).Length() <= 64)
		{
			QAngle temp;
			VectorAngles(tr.plane.normal, temp);
			// Use temp angle to convert it to the up/right/forward vectors
			AngleVectors(temp, &forward, &right, &up);
		}
		else
		{
			AngleVectors(pTrap->GetAbsAngles(), &forward, &right, &up);
		}

		Vector velocity = forward * random->RandomInt(eo_trapsdefense_projectile_minforward.GetInt(), 
			eo_trapsdefense_projectile_maxforward.GetInt()) +

			right * random->RandomInt(eo_trapsdefense_projectile_minside.GetInt(), 
			eo_trapsdefense_projectile_maxside.GetInt()) + 

			up * random->RandomInt(eo_trapsdefense_projectile_minside.GetInt(), 
			eo_trapsdefense_projectile_maxside.GetInt());


		// Now do the spawn
		pDefense->SetAbsOrigin(pos);
		pDefense->SetAbsAngles(angle);

		pDefense->Spawn();

		pDefense->ApplyAbsVelocityImpulse(velocity);

		pDefense->SetGracePeriod(0.01f);
	}

#endif
}

void CreateSpikes(const Vector & pos, const QAngle & angle)
{
}

void CreateReverseVortex(const Vector & pos, const QAngle & angle)
{
#ifndef CLIENT_DLL
	CBaseEntity * pEnt = CreateEntityByName("defense_reversevortex");

	if(pEnt)
	{
		pEnt->SetAbsOrigin(pos);
		pEnt->SetAbsAngles(angle);
		pEnt->Spawn();
		DispatchParticleEffect(REVERSE_VORTEX_PARTICLE, pos, angle);
	}
	else
		Warning("Cannot create vortex defense\n");
#endif
}

ConVar eo_trapsdefense_grenades_min_forward("eo_trapsdefense_grenades_min_forward", "150", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_max_forward("eo_trapsdefense_grenades_max_forward", "480", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_min_side("eo_trapsdefense_grenades_min_side", "-740", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_max_side("eo_trapsdefense_grenades_max_side", "740", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_number_min("eo_trapsdefense_grenades_number_min", "2", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_number_max("eo_trapsdefense_grenades_number_max", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_grenades_timer("eo_trapsdefense_grenades_timer", "3.0", FCVAR_REPLICATED | FCVAR_CHEAT);

void CreateGrenades(const Vector & pos, const QAngle & angle)
{
#ifndef CLIENT_DLL
	int grenades = random->RandomInt(eo_trapsdefense_grenades_number_min.GetInt(), eo_trapsdefense_grenades_number_max.GetInt());
	for(int i = 0; i < grenades; i++)
	{
		Vector spin;
		spin.x = random->RandomFloat( -1000.0, 1000.0 );
		spin.y = random->RandomFloat( -1000.0, 1000.0 );
		spin.z = random->RandomFloat( -1000.0, 1000.0 );

		Vector velocity;
		velocity.x = random->RandomFloat( eo_trapsdefense_grenades_min_side.GetFloat(), 
			eo_trapsdefense_grenades_max_side.GetFloat() );
		velocity.y = random->RandomFloat( eo_trapsdefense_grenades_min_side.GetFloat(), 
			eo_trapsdefense_grenades_max_side.GetFloat() );
		velocity.z = random->RandomFloat( eo_trapsdefense_grenades_min_forward.GetFloat(), 
			eo_trapsdefense_grenades_max_forward.GetFloat() );

		CBasePlayer * pOverlord = GetOverlordData()->GetOverlord();

		CBaseGrenade * pGrenade = Fraggrenade_Create(pos, angle, velocity, spin, pOverlord, eo_trapsdefense_grenades_timer.GetFloat(), false);
		
		// Ov is our owner!
		if(pGrenade)
		{
			pGrenade->SetOwnerEntity(GetOverlordData()->GetOverlord());
		}
	
	}
#endif
}

ConVar eo_trapsdefense_randomtrap_range("eo_trapsdefense_randomtrap_range", "1024", FCVAR_REPLICATED | FCVAR_CHEAT);

#define BUILD_SOUND "Town.d1_town_01_ball_zap1"
#define RANDOM_TRAP_PARTICLE "RandomTrapSpawned"

void CreateRandomTrap(const Vector & pos, const QAngle & angle, COverlordTrap * pTrap)
{
#ifndef CLIENT_DLL
	if(!pTrap)
		return;

	COverlordData & data = *GetOverlordData();

	// Create a new defense
	if(data.GetMaxTraps() <= data.GetTrapsAmount())
	{
		// Just use the default function, it will launch itself again
		// if random trap is selected again
		CreateTrapDefense(pos, angle, pTrap);
		return;
	}

	CTraceFilterNoNPCsOrPlayer filter(pTrap, COLLISION_GROUP_NONE);
	trace_t tr;
	int i = 0;
	do
	{
		i++;
		Vector vRandom = RandomVector(-1,1);
		VectorNormalize(vRandom);

		Vector vDest;

		VectorMA(pos, eo_trapsdefense_randomtrap_range.GetFloat(), vRandom, vDest);

		UTIL_TraceLine(pos, vDest, MASK_SHOT_HULL, &filter, &tr);
	}
	while(tr.fraction == 1.0f && i <= 32);

	// If not found, create a trap where this one used to be, otherwise just use another
	// defense
	if(tr.fraction == 1.0f)
	{
		Vector dir = pTrap->WorldSpaceCenter() - pTrap->GetAbsOrigin();
		Vector end;
		VectorNormalize(dir);

		VectorMA(pTrap->WorldSpaceCenter(), eo_trapsdefense_randomtrap_range.GetFloat(), dir, end);
	
		UTIL_TraceLine(pTrap->WorldSpaceCenter(), end, MASK_SHOT_HULL, &filter, &tr);

		if(tr.fraction == 1.0f)
		{
			CreateTrapDefense(pos, angle, pTrap);
			return;
		}
	}

	// Get our traps to be built...
	char categorised[][32] = 
	{
		"trap_addicter",
		"trap_sticky",
		"trap_pigeon",
		"trap_bomber",
		"trap_mist",
		"trap_invisibler",
		"trap_fan",
	};

	int randint = random->RandomInt(0, ARRAYSIZE(categorised));

	COverlordTrap * pBuilding = static_cast<COverlordTrap*>(CreateEntityByName(categorised[randint]));

	if(!pBuilding)
	{
		Warning("Cannot build trapdefense trap, %s!\n", categorised[randint]);
		CreateTrapDefense(pos, angle, pTrap);
		return;
	}

	Vector trapPos = tr.endpos + tr.plane.normal * pBuilding->GetNormalMultiplier();
	QAngle trapAng = pBuilding->NormalToAngle(tr.plane.normal);

	if(tr.m_pEnt && Q_stricmp(tr.m_pEnt->GetClassname(), "worldspawn"))
		pBuilding->SetParent(tr.m_pEnt);

	PrecacheParticleSystem(RANDOM_TRAP_PARTICLE);
	
	pBuilding->SetAbsOrigin(trapPos);
	pBuilding->SetAbsAngles(trapAng);

	pBuilding->Spawn();

	// Sound!
	pBuilding->PrecacheScriptSound(BUILD_SOUND);
	pBuilding->EmitSound(BUILD_SOUND);
	
	// Particle!
	//DispatchParticleEffect(RANDOM_TRAP_PARTICLE, pBuilding->WorldSpaceCenter(), pBuilding->GetAbsAngles(), pBuilding);
	DispatchParticleEffect(RANDOM_TRAP_PARTICLE, PATTACH_ABSORIGIN_FOLLOW, pBuilding);

	data.AddTrap(pBuilding);

#endif
}

ConVar eo_trapsdefense_slowdown_radius("eo_trapsdefense_slowdown_radius", "250", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_slowdown_lifetime("eo_trapsdefense_slowdown_lifetime", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_slowdown_amount("eo_trapsdefense_slowdown_amount", "100", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdefense_slowdown_time("eo_trapsdefense_slowdown_time", "3.0", FCVAR_REPLICATED | FCVAR_CHEAT);

#define SLOWDOWN_PARTICLE "SlowdownEffect"
#define SLOWDOWN_RADIUS eo_trapsdefense_slowdown_radius.GetFloat()
#define SLOWDOWN_LIEFTIME eo_trapsdefense_slowdown_lifetime.GetFloat()

#ifndef CLIENT_DLL
class COverlordSlowdown : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordSlowdown, CBaseEntity);
	
	COverlordSlowdown();
	virtual ~COverlordSlowdown() { };

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

private:
	float m_flDieTime;

};

LINK_ENTITY_TO_CLASS(overlord_slowdown, COverlordSlowdown);

COverlordSlowdown::COverlordSlowdown()
{
	m_flDieTime = 0.0f;
}

void COverlordSlowdown::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(SLOWDOWN_PARTICLE);
}

void COverlordSlowdown::Spawn()
{
	Precache();

	m_flDieTime = gpGlobals->curtime + eo_trapsdefense_slowdown_lifetime.GetFloat();
	//SetModel(NULL);

	SetThink(&COverlordSlowdown::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void COverlordSlowdown::Think()
{
	if(m_flDieTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		return;
	}
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive())
			continue;

		if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= SLOWDOWN_RADIUS)
		{
			if(!pPlayer->HasSlowdown(this))
			{
				pPlayer->AddSlowdown(eo_trapsdefense_slowdown_amount.GetInt(), 
					eo_trapsdefense_slowdown_time.GetFloat(), this,
					true);
			}
		}
	}
	SetNextThink(gpGlobals->curtime + 0.1f);
}

#endif
void CreateSlowdown(const Vector &pos, const QAngle &angle)
{
#ifndef CLIENT_DLL
	CBaseEntity * pEnt = CreateEntityByName("overlord_slowdown");

	if(pEnt)
	{
		pEnt->SetAbsOrigin(pos);
		pEnt->SetAbsAngles(angle);
		pEnt->Spawn();

		DispatchParticleEffect(SLOWDOWN_PARTICLE, pos, angle);
	}
#endif
}