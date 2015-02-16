//==============Overlord mod=====================
//	Trap marker (largely based on bugbait code)
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "overlord_data.h"
#include "basegrenade_shared.h"

#ifndef CLIENT_DLL
#include "overlord_data.h"
#include "overlord_dynamictraps.h"
#include "particle_parse.h"
#endif

#ifdef CLIENT_DLL
#define COverlordMarker C_OverlordMarker
#define COverlodrMarkerGrenade C_OverlordMarkerGrenade
#endif

#define MARKER_ATTACK1 1
#define MARKER_ATTACK2 2

#define MARKER_SOUND "Weapon_Marker.Alarm"

#define MARKER_MODEL "models/weapons/w_bugbait.mdl"

ConVar eo_marker_mark_distance("eo_marker_mark_distance", "232", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_marker_length("eo_marker_length", "7.75", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_marker_traps_length("eo_marker_traps_length", "8.0", FCVAR_CHEAT | FCVAR_REPLICATED);
//ConVar eo_marker_dormant_chance("eo_marker_dormant_chance", "25.0", FCVAR_CHEAT | FCVAR_REPLICATED);


//======================================================================================================================
//												Grenade
//======================================================================================================================
class COverlordMarkerGrenade : public CBaseGrenade
{
public:
	DECLARE_CLASS(COverlordMarkerGrenade, CBaseGrenade);
	DECLARE_DATADESC();

	virtual void Spawn();

#ifndef CLIENT_DLL
	virtual void ThinkBecomeSolid();

	virtual void MarkerTouch( CBaseEntity *pOther );

	virtual void MarkTraps();
	virtual void MarkTrap(COverlordTrap * pEnt);

	virtual bool		  IsClassWeapon() const { return true; };
	virtual	PlayerClass_t GetClassWeapon() const { return CLASS_ASSAULT; };

	void		 SetGracePeriod(float flTime);
#endif
};


BEGIN_DATADESC(COverlordMarkerGrenade)

#ifndef CLIENT_DLL
DEFINE_ENTITYFUNC( MarkerTouch ),
#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS(overlord_marker_grenade, COverlordMarkerGrenade);

void COverlordMarkerGrenade::Spawn()
{
	Precache();

	SetModel(MARKER_MODEL);
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_DEFAULT );
	SetSolid( SOLID_BBOX ); 

#ifndef CLIENT_DLL
	UTIL_SetSize( this, Vector( -2, -2, -2), Vector( 2, 2, 2 ) );

	SetTouch( &COverlordMarkerGrenade::MarkerTouch );
#endif
	m_takedamage = DAMAGE_NO;
}


#ifndef CLIENT_DLL
void COverlordMarkerGrenade::ThinkBecomeSolid()
{
	SetThink(NULL);

	RemoveSolidFlags(FSOLID_NOT_SOLID);
}

void COverlordMarkerGrenade::MarkerTouch(CBaseEntity * pOther)
{
	if(pOther)
	{
		//DevMsg("MarkerTouch...\n");
		if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) )
			return;

		if(pOther->IsPlayer() || pOther->IsNPC())
			return;

		//DevMsg("Touched an entity\n");

		MarkTraps();

		UTIL_Remove(this);
	}
	else
	{
		//DevMsg("Touched without an entity\n");
	}
}

void COverlordMarkerGrenade::MarkTraps()
{
	COverlordData * pData = GetOverlordData();

	bool bFound = false;

	CTrapFilter filter(this, COLLISION_GROUP_NONE);

	Vector startPos = WorldSpaceCenter();
	for(int i = 0; i < pData->GetMaxTraps(); i++)
	{
		COverlordTrap * pTrap = pData->GetTrap(i);

		if(!pTrap)
			continue;
		// Check distance
		if((pTrap->WorldSpaceCenter() - startPos).Length() <= eo_marker_mark_distance.GetFloat())
		{
			trace_t tr;
			UTIL_TraceLine(startPos, pTrap->WorldSpaceCenter(), MASK_SHOT_HULL, &filter, &tr);

			// We use a trap filter
			if((tr.fraction >= 0.95f))
			{
				if(pTrap->IsDecaying())
					continue;

				MarkTrap(pTrap);
				bFound = true;
			}
		}
	}


	// Check for Ov now
	CBasePlayer * pOverlord = GetOverlordData()->GetOverlord();

	if(pOverlord && ( (pOverlord->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_marker_mark_distance.GetFloat()) )
	{
		bFound = true;
		GetOverlordData()->MarkOverlord(eo_marker_length.GetFloat());
	}

	if(bFound)
	{
		EmitSound(MARKER_SOUND);		
	}
}

void COverlordMarkerGrenade::MarkTrap(COverlordTrap * pEnt)
{
	if(!pEnt)
		return;

	if(pEnt->CanMarkTrap())
	{
		pEnt->MarkTrap(eo_marker_traps_length.GetFloat());

		if(pEnt->GetRemarkTime() <= gpGlobals->curtime)
		{
			pEnt->SetLastTimeMarked(gpGlobals->curtime);
			
			CBasePlayer * pPlayer = ToBasePlayer(GetThrower());

			if(pPlayer)
				pPlayer->IncreaseOverlordDamage(pEnt->GetPoints(CLASS_ASSAULT));
		}
	}

	/*CTrapFilter filter;
	Vector pos = pEnt->WorldSpaceCenter() + Vector(0, 0, 42);

	// Trace hull it for certainty
	trace_t tr;
	UTIL_TraceHull(pEnt->WorldSpaceCenter(), pos, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SHOT_HULL, pEnt, 
		COLLISION_GROUP_NONE, &tr);

	// Just emit it there
	if(tr.fraction >= 1.0f)
	{
		DispatchParticleEffect(MARKER_PARTICLE, tr.endpos, QAngle(0, 0, 0));
		return;
	}

	// Look for a spot in front of us
	Vector front, forw;
	AngleVectors(GetAbsAngles(), &forw);
	front = front + forw * 8;

	UTIL_TraceHull(pEnt->GetEmitterPosition(), front, Vector(-1, -1, -1), Vector(1, 1, 1), MASK_SHOT_HULL, pEnt, 
		COLLISION_GROUP_NONE, &tr);	

	if(tr.fraction >= 1.0f)
	{
		DispatchParticleEffect(MARKER_PARTICLE, tr.endpos, QAngle(0, 0, 0));
		return;
	}


	// Just emit "inside" of us
	DispatchParticleEffect(MARKER_PARTICLE, pEnt->WorldSpaceCenter(), QAngle(0, 0, 0));*/



}

void COverlordMarkerGrenade::SetGracePeriod(float flTime)
{
	SetThink(&COverlordMarkerGrenade::ThinkBecomeSolid);
	SetNextThink(gpGlobals->curtime + flTime);

	AddSolidFlags(FSOLID_NOT_SOLID);
}

COverlordMarkerGrenade * MarkerGrenade_create(const Vector &position, const QAngle &angles, const Vector &velocity, 
											  const QAngle &angVelocity, CBaseEntity *owner)
{
	COverlordMarkerGrenade *pGrenade = static_cast<COverlordMarkerGrenade*>(
		CBaseEntity::Create( "overlord_marker_grenade", position, angles, owner )
		);
	
	if ( pGrenade )
	{
		pGrenade->SetLocalAngularVelocity( angVelocity );
		pGrenade->SetAbsVelocity( velocity );
		pGrenade->SetThrower( ToBaseCombatCharacter( owner ) );
		pGrenade->SetGracePeriod(0.1f);
	}

	return pGrenade;
}
#endif
//======================================================================================================================
//												Weapon
//======================================================================================================================

class COverlordMarker : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordMarker, CBaseHL2MPCombatWeapon);
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();


	DECLARE_ACTTABLE();


	COverlordMarker();
	virtual ~COverlordMarker();

	virtual void Precache();

	virtual int GetClassWeapon() const { return CLASS_ASSAULT; };
	//virtual int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual void ItemPostFrame();
	
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	virtual bool Deploy();
	virtual bool Holster(CBaseCombatWeapon * pSwitchingTo);
	virtual void Drop(const Vector &vecVelocity);
	virtual bool Reload();

	virtual void ThrowMarker();
	virtual void RollMarker();
#endif
private:
	CNetworkVar(bool, m_bDrawBackFinished);
	CNetworkVar(bool, m_bDraw);
	CNetworkVar(short, m_iAttack);
};


IMPLEMENT_NETWORKCLASS_ALIASED(OverlordMarker, DT_OverlordMarker)

BEGIN_NETWORK_TABLE(COverlordMarker, DT_OverlordMarker)
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bDrawBackFinished)),
	SendPropBool(SENDINFO(m_bDraw)),
	SendPropInt(SENDINFO(m_iAttack)),
#else
	RecvPropBool(RECVINFO(m_bDrawBackFinished)),
	RecvPropBool(RECVINFO(m_bDraw)),
	RecvPropInt(RECVINFO(m_iAttack)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordMarker)

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_marker, COverlordMarker );
PRECACHE_WEAPON_REGISTER( weapon_marker );

BEGIN_DATADESC( COverlordMarker )

END_DATADESC()



acttable_t	COverlordMarker::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(COverlordMarker);



COverlordMarker::COverlordMarker()
{
	m_bDraw = false;
	m_bDrawBackFinished = false;
	m_iAttack = NULL;
}

COverlordMarker::~COverlordMarker()
{
}

void COverlordMarker::Precache()
{
	BaseClass::Precache();

	PrecacheModel(MARKER_MODEL);
	PrecacheScriptSound(MARKER_SOUND);
}


void COverlordMarker::ItemPostFrame()
{	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	
	if (!pOwner)
		return;

	if(m_bDrawBackFinished)
	{
		if (!(pOwner->m_nButtons & IN_ATTACK) && !(pOwner->m_nButtons & IN_ATTACK2))
		{
			if(m_iAttack == MARKER_ATTACK1)
				SendWeaponAnim(ACT_VM_THROW);
			else if(m_iAttack == MARKER_ATTACK2)
				SendWeaponAnim( ACT_VM_THROW );

			m_flNextPrimaryAttack  = gpGlobals->curtime + SequenceDuration();
			m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			m_bDrawBackFinished = false;
		}
	}
	else
	{
		if ((pOwner->m_nButtons & IN_ATTACK ) && (m_flNextPrimaryAttack < gpGlobals->curtime))
		{
			PrimaryAttack();
		}
		else if ((pOwner->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack < gpGlobals->curtime))
		{
			SecondaryAttack();
		}
	}

	if(m_bDraw)
	{
		if (IsViewModelSequenceFinished())
		{
			Reload();
		}
	}
	WeaponIdle();
}

void COverlordMarker::PrimaryAttack()
{
	if(m_bDraw)
		return;

	if(Clip1() <= 0)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	SendWeaponAnim(ACT_VM_HAULBACK);
	
	m_flTimeWeaponIdle		= FLT_MAX;
	m_flNextPrimaryAttack	= FLT_MAX;
	m_iAttack = MARKER_ATTACK1;
}

void COverlordMarker::SecondaryAttack()
{
	if(m_bDraw)
		return;

	if(Clip1() <= 0)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	SendWeaponAnim(ACT_VM_HAULBACK);
	
	m_flTimeWeaponIdle		= FLT_MAX;
	m_flNextSecondaryAttack	= FLT_MAX;
	m_iAttack = MARKER_ATTACK2;
}

#ifndef CLIENT_DLL
void COverlordMarker::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch(pEvent->event)
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_bDrawBackFinished = true;
			break;
		case EVENT_WEAPON_THROW:
			{
				m_iClip1--;

				if(m_iAttack == MARKER_ATTACK1)
					ThrowMarker();
				else
					RollMarker();
				break;
			}
		case EVENT_WEAPON_THROW2:
			{
				m_iClip1--;

				if(m_iAttack == MARKER_ATTACK1)
					ThrowMarker();
				else
					RollMarker();
				break;
			}
		case EVENT_WEAPON_THROW3:
			{
				m_iClip1--;

				if(m_iAttack =- MARKER_ATTACK1)
					ThrowMarker();
				else
					RollMarker();
				break;
			}
		default:
			BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
			break;
	}
}

bool COverlordMarker::Deploy()
{
	m_bDrawBackFinished = false;

	return BaseClass::Deploy();
}

bool COverlordMarker::Holster(CBaseCombatWeapon * pSwitchingTo)
{
	m_bDrawBackFinished = false;

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordMarker::Drop(const Vector &vecVelocity)
{
	m_bDrawBackFinished = false;

	BaseClass::Drop(vecVelocity);
}

bool COverlordMarker::Reload()
{
	if ((m_bDraw) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		SendWeaponAnim( ACT_VM_DRAW );
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
		m_bDraw = false;
	}

	return true;
}

void COverlordMarker::ThrowMarker()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	
	if(!pPlayer)
		return;

	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	
	pPlayer->EyeVectors( &vForward, &vRight, &vUp );

	vThrowPos = pPlayer->EyePosition();

	vThrowPos += vForward * 18.0f;
	vThrowPos += vRight * 12.0f;

	pPlayer->GetVelocity( &vThrowVel, NULL );
	vThrowVel += vForward * 1000;

	/*COverlordMarkerGrenade *pGrenade =*/ MarkerGrenade_create( vThrowPos, vec3_angle, vThrowVel, QAngle(600,random->RandomInt(-1200,1200),0), pPlayer );

	m_bDraw = true;
}

void COverlordMarker::RollMarker()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	
	if(!pPlayer)
		return;

	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	
	pPlayer->EyeVectors( &vForward, &vRight, &vUp );

	vThrowPos = pPlayer->EyePosition();

	vThrowPos += vForward * 9.0f;
	vThrowPos += vRight * 6.0f;

	pPlayer->GetVelocity( &vThrowVel, NULL );
	vThrowVel += vForward * 400;

	/*COverlordMarkerGrenade *pGrenade =*/ MarkerGrenade_create( vThrowPos, vec3_angle, vThrowVel, QAngle(300,random->RandomInt(-600,600),0), pPlayer );

	m_bDraw = true;
}
#endif

