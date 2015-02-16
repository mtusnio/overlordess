//==============Overlord mod=====================
//	Psion's shield
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#define KINESIS 0
#define SHIELD 1
#define MODE_SIZE 1

#define SELF 0
#define FRIEND 1
#define TARGET_SIZE 1

#ifdef CLIENT_DLL
#define COverlordPsion C_OverlordPsion
#define COverlordPsishield C_OverlordPsishield
#endif

#define KINESIS_RANGE 750
#define KINESIS_FORCE 180
#define KINESIS_SECONDARY KINESIS_FORCE

class COverlordPsishield : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordPsishield, CBaseEntity);

};

LINK_ENTITY_TO_CLASS(prop_psishield, COverlordPsishield);

class COverlordPsion : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordPsion, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	COverlordPsion();
	virtual ~COverlordPsion() { };

	#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
	#endif

	virtual PlayerClass_t GetClassWeapon() const { return CLASS_PSION; };

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual bool		  IsMainClassWeapon() const { return true; };

	virtual const char * GetPrimaryMode() const { return "Psychokinesis"; };
	virtual const char * GetSecondaryMode() const { return "Shield"; }; 
#if 0
	virtual void HandleFireOnEmpty() { return; };
	virtual bool Reload() { ReleaseKinesis(); return false; };
#endif
private:
#if 0
	virtual void KinesisAttack();
	virtual void ShieldAttack();
	
	virtual void ReleaseKinesis();
	EHANDLE m_pTargeted;
#endif


};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordPsion, DT_OverlordPsion)

BEGIN_NETWORK_TABLE(COverlordPsion, DT_OverlordPsion)

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( COverlordPsion)


END_PREDICTION_DATA()


LINK_ENTITY_TO_CLASS(weapon_psion, COverlordPsion);


PRECACHE_WEAPON_REGISTER(weapon_psion);

#ifndef CLIENT_DLL

acttable_t	COverlordPsion::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PISTOL,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL,				false },
};

IMPLEMENT_ACTTABLE(COverlordPsion);

#endif

COverlordPsion::COverlordPsion()
{
#if 0
	m_pTargeted = NULL;
#endif
}

void COverlordPsion::PrimaryAttack()
{

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
}

void COverlordPsion::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;
}

#if 0

void COverlordPsion::KinesisAttack()
{
	if(!GetOwner())
		return;

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
		start = GetOwner()->EyePosition() + forward;
		end = start + forward * MAX_TRACE_LENGTH;

		UTIL_TraceLine(start, end, MASK_SHOT|CONTENTS_GRATE, GetOwner() , COLLISION_GROUP_NONE, &tr);

		/*if(tr.fraction == 1.0)
			return;*/

		if(tr.m_pEnt == m_pTargeted)
			return;

		Vector force;
		force = tr.endpos - m_pTargeted->GetAbsOrigin();
		//force.Negate();
		
		AngularImpulse impulse = RandomAngularImpulse(0.0f, 500.0f);
		
		pPhys->ApplyForceCenter(force * KINESIS_FORCE);
		pPhys->ApplyTorqueCenter(impulse);
		//ApplyLocalAngularVelocityImpulse(impulse);
	}
	else if(!m_pTargeted)
	{
		Vector start, forward, end;
		trace_t tr;

		start = GetOwner()->EyePosition();
		AngleVectors(GetOwner()->EyeAngles(), &forward);
		VectorMA(start, KINESIS_RANGE, forward, end);

		UTIL_TraceLine(start, end, MASK_SHOT|CONTENTS_GRATE, GetOwner() , COLLISION_GROUP_NONE, &tr);

		if(tr.fraction == 1.0)
			return;

		if(!tr.m_pEnt || tr.m_pEnt->GetMoveType() != MOVETYPE_VPHYSICS)
			return;

		if(!tr.m_pEnt->VPhysicsGetObject())
			return;

		m_pTargeted = tr.m_pEnt;

		GetOwner()->EmitSound("HL2Player.FlashLightOn" );

	}
	// Add owner entity at the end
	if(m_pTargeted && m_pTargeted->GetOwnerEntity() == NULL)
	{
		m_pTargeted->SetOwnerEntity(GetOwner());
	}
}

void COverlordPsion::ShieldAttack()
{

}

void COverlordPsion::ReleaseKinesis()
{
	if(!m_pTargeted)
		return;
	
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;

		// Add owner entity at the end
	if(m_pTargeted->GetOwnerEntity() == pPlayer)
	{
		m_pTargeted->SetOwnerEntity(NULL);
	}
	m_pTargeted = NULL;

	// Emit client-side sound
	pPlayer->EmitSound("HL2Player.FlashLightOn");
}

#endif
