//######Overlord#######
//	Psychokinesis 
//	weapon
//#####################

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "particle_parse.h"
	#include "model_types.h"
#endif

#include "gameeventlistener.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordPsychokinesis C_OverlordPsychokinesis
#endif

ConVar eo_psychokinesis_range("eo_psychokinesis_range", "1024", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_psychokinesis_force("eo_psychokinesis_force", "150", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_psychokinesis_cost("eo_psychokinesis_cost", "35", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_psychokinesis_push_distance("eo_psychokinesis_push_distance", "300", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_psychokinesis_push_force("eo_psychokinesis_push_force", "1600", FCVAR_REPLICATED | FCVAR_CHEAT);

#define KINESIS_RANGE eo_psychokinesis_range.GetFloat()
#define KINESIS_FORCE eo_psychokinesis_force.GetFloat()
#define KINESIS_SECONDARY KINESIS_FORCE;
#define PUSH_SOUND "Strider.Impact"
#define PUSH_FAIL_SOUND "NPC_Strider.Whoosh"
#define PSYCHOKINESIS_TARGET "NPC_CombineCamera.Click"

#define PSYCHOKINESIS_PARTICLE "PsychokinesisMark"



#define PUSH_VECTOR Vector(32, 32, 32)
#define PUSH_DISTANCE eo_psychokinesis_push_distance.GetFloat()
#define PUSH_COST eo_psychokinesis_cost.GetInt()

#define PUSH_FORCE eo_psychokinesis_push_force.GetFloat()

class COverlordPsychokinesis : public CBaseHL2MPCombatWeapon, public CGameEventListener
{
	
public:
	DECLARE_CLASS(COverlordPsychokinesis, CBaseHL2MPCombatWeapon);
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

	DECLARE_NETWORKCLASS(); 


	COverlordPsychokinesis(void);
	virtual ~COverlordPsychokinesis(void);

	virtual void Spawn();
	virtual void Precache();

	virtual void PrimaryAttack(void);
	virtual void SecondaryAttack(void);

	virtual	void		  HandleFireOnEmpty() { PrimaryAttack(); };

	virtual bool Reload(void);
	virtual bool Holster(CBaseCombatWeapon * pSwitchingTo);

	virtual bool		  IsOverlordWeapon() const { return true; };

	virtual void		  ItemPostFrame();

	virtual void		  FireGameEvent(IGameEvent * event);

	virtual const char * GetPrimaryMode() const { return "Select/Move object"; }; 
	virtual const char * GetSecondaryMode() const { return "Push objects/players in front"; }; 
	virtual const char * GetCostLabel() const;
private:
	void				  DisableTargeting();

	//bool m_bTargeted;
	CNetworkHandle(CBaseEntity, m_pTargeted);

#ifdef CLIENT_DLL 
	CNewParticleEffect * m_pEffect;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordPsychokinesis, DT_OverlordPsychokinesis)

BEGIN_NETWORK_TABLE(COverlordPsychokinesis, DT_OverlordPsychokinesis)
#ifndef CLIENT_DLL
	SendPropEHandle(SENDINFO(m_pTargeted)),
#else
	RecvPropEHandle(RECVINFO(m_pTargeted)),
#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(COverlordPsychokinesis)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_psychokinesis, COverlordPsychokinesis);
PRECACHE_WEAPON_REGISTER(weapon_psychokinesis);

acttable_t COverlordPsychokinesis::m_acttable[] = 
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



IMPLEMENT_ACTTABLE(COverlordPsychokinesis);


COverlordPsychokinesis::COverlordPsychokinesis(void)
{
	m_pTargeted = NULL;

#ifdef CLIENT_DLL
	m_pEffect = NULL;
#endif
}

COverlordPsychokinesis::~COverlordPsychokinesis(void)
{
	DisableTargeting();
}

void COverlordPsychokinesis::Spawn()
{
	BaseClass::Spawn();
#ifdef CLIENT_DLL
	ListenForGameEvent("psychokinesis_targeted");
	ListenForGameEvent("psychokinesis_detargeted");
#endif
}

void COverlordPsychokinesis::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound(PSYCHOKINESIS_TARGET);
	PrecacheScriptSound(PUSH_SOUND);
	PrecacheScriptSound(PUSH_FAIL_SOUND);
#ifdef CLIENT_DLL
	PrecacheParticleSystem(PSYCHOKINESIS_PARTICLE);
#endif
}

void COverlordPsychokinesis::PrimaryAttack(void)
{
	if(!GetOwner())
		return;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.4f;

	if(m_pTargeted)
	{
		IPhysicsObject * pPhys = m_pTargeted->VPhysicsGetObject();

		if(!pPhys)
		{
			m_pTargeted = NULL;
			return;
		}



		Vector start, forward, end;
		trace_t tr;

		AngleVectors(GetOwner()->EyeAngles(), &forward);
		start = GetOwner()->RealEyePosition() + forward;
		end = start + forward * MAX_TRACE_LENGTH;

		UTIL_TraceLine(start, end, MASK_SHOT_HULL, GetOwner() , COLLISION_GROUP_NONE, &tr);

		/*if(tr.fraction == 1.0)
			return;*/

		if(tr.m_pEnt == m_pTargeted)
			return;

		Vector force;
		force = tr.endpos - m_pTargeted->GetAbsOrigin();
		//force.Negate();
		
		AngularImpulse impulse = RandomAngularImpulse(0.0f, 250.0f);
		
		pPhys->ApplyForceCenter(force * KINESIS_FORCE);
		pPhys->ApplyTorqueCenter(impulse);
		//ApplyLocalAngularVelocityImpulse(impulse);
	}
	else if(!m_pTargeted)
	{
		Vector start, forward, end;
		trace_t tr;

		start = GetOwner()->RealEyePosition();
		AngleVectors(GetOwner()->EyeAngles(), &forward);
		VectorMA(start, KINESIS_RANGE, forward, end);

		UTIL_TraceLine(start, end, MASK_SHOT|CONTENTS_GRATE, GetOwner() , COLLISION_GROUP_NONE, &tr);

		if(tr.fraction == 1.0)
			return;

		if(!tr.m_pEnt || tr.m_pEnt->GetMoveType() != MOVETYPE_VPHYSICS)
			return;

		if(!(tr.m_pEnt->VPhysicsGetObject()))
			return;

		// Crash fix
#ifdef CLIENT_DLL
		if(tr.m_pEnt->IsBrushModel())
			return;
#else
		model_t *pModel = tr.m_pEnt->GetModel();
		bool bIsBrush = ( pModel && modelinfo->GetModelType( pModel ) == mod_brush );

		if(bIsBrush)
			return;
#endif
		m_pTargeted = tr.m_pEnt;

		GetOwner()->EmitSound(PSYCHOKINESIS_TARGET);

#ifndef CLIENT_DLL
		IGameEvent * event = gameeventmanager->CreateEvent("psychokinesis_targeted");
		if(event)
		{
			event->SetInt("entindex", m_pTargeted->entindex());

			gameeventmanager->FireEvent(event);
		}
#endif
	}

	// Add owner entity at the end
	if(m_pTargeted && m_pTargeted->GetOwnerEntity() == NULL)
	{
		m_pTargeted->SetOwnerEntity(this);
	}

}

void COverlordPsychokinesis::SecondaryAttack()
{
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	COverlordData & data = *GetOverlordData();

	if(data.GetPower() < PUSH_COST)
	{
		pPlayer->EmitSound(PUSH_FAIL_SOUND);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.45f;
		return;
	}

	Vector start = pPlayer->EyePosition();
	Vector end;
	Vector forw;
	AngleVectors(pPlayer->EyeAngles(), &forw);

	end = start + forw * PUSH_DISTANCE;

	Ray_t ray;
	ray.Init(start, end, -PUSH_VECTOR, PUSH_VECTOR);
	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

	bool bUsePower = false;
	for(int i = 0; i < count; i++)
	{
		if(!pList[i])
			continue;

				// Crash fix
		model_t *pModel = pList[i]->GetModel();
		bool bIsBrush = ( pModel && modelinfo->GetModelType( pModel ) == mod_brush );

		if(bIsBrush)
			continue;

		trace_t tr;
		UTIL_TraceLine(start, pList[i]->WorldSpaceCenter(), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt == pList[i] || tr.fraction >= 1.0f)
		{
			float distance = PUSH_DISTANCE - (tr.endpos - start).Length();
			Vector force = forw * distance * PUSH_FORCE;
			if(pList[i]->IsPlayer())
			{
				CBasePlayer * pPushed = ToBasePlayer(pList[i]);

				if(pPushed->IsAlive() && pPushed->IsRebel() && !(pPushed->GetFlags() & FL_FROZEN))
				{
					bUsePower = true;
					pPushed->ApplyAbsVelocityImpulse(force);
				}
			}
			else
			{
				IPhysicsObject * pObject = pList[i]->VPhysicsGetObject();

				if(pObject)
				{
					pObject->ApplyForceCenter(force);
					bUsePower = true;
				}
			}
		}

	}

	if(bUsePower)
	{
		pPlayer->EmitSound(PUSH_SOUND);
		data.HandlePowerEvent(EVENT_WEAPONUSED, PUSH_COST);
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + 1.1f;
#endif
}

void COverlordPsychokinesis::ItemPostFrame()
{
	if(m_pTargeted)
	{
		CBasePlayer * pOwner = ToBasePlayer(GetOwner());

		if(pOwner)
		{
			Vector dist = m_pTargeted->GetAbsOrigin() - pOwner->RealEyePosition();

			if(dist.Length() > KINESIS_RANGE)
				DisableTargeting();
	
		}
	}

	BaseClass::ItemPostFrame();
}

bool COverlordPsychokinesis::Reload(void)
{
	DisableTargeting();

	return false;
}

bool COverlordPsychokinesis::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	DisableTargeting();

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordPsychokinesis::FireGameEvent(IGameEvent * event)
{
#ifdef CLIENT_DLL
	if(event)
	{
		if(!Q_stricmp(event->GetName(), "psychokinesis_targeted"))
		{
			if(C_BasePlayer::GetLocalPlayer()->IsOverlord())
			{
				CBaseEntity * pEntity = ClientEntityList().GetBaseEntity(event->GetInt("entindex"));
				if(pEntity)
				{
					m_pEffect = pEntity->ParticleProp()->Create(PSYCHOKINESIS_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);
				}
				else
				{
					Warning("Cannot create psychokinesis particles\n");
				}
			}
		}
		else if(!Q_stricmp(event->GetName(), "psychokinesis_detargeted"))
		{
			if(C_BasePlayer::GetLocalPlayer()->IsOverlord())
			{
				CBaseEntity * pEntity = ClientEntityList().GetBaseEntity(event->GetInt("entindex"));
				if(pEntity)
				{
					pEntity->ParticleProp()->StopEmission(m_pEffect);
					m_pEffect = NULL;
				}
				else
				{
					Warning("Cannot disable psychokinesis particles\n");
				}
			}
		}
	}
#endif
}

const char * COverlordPsychokinesis::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "0/%i", eo_psychokinesis_cost.GetInt());

	return label;
}

void COverlordPsychokinesis::DisableTargeting()
{
	if(!m_pTargeted)
		return;
	
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;

	if(m_pTargeted->GetOwnerEntity() == pPlayer)
	{
		m_pTargeted->SetOwnerEntity(NULL);
	}

#ifndef CLIENT_DLL
	IGameEvent * event = gameeventmanager->CreateEvent("psychokinesis_detargeted");
	if(event)
	{
		event->SetInt("entindex", m_pTargeted->entindex());

		gameeventmanager->FireEvent(event);
	}
#endif

	m_pTargeted = NULL;

	// Emit client-side sound

	pPlayer->EmitSound(PSYCHOKINESIS_TARGET);

	return;
}