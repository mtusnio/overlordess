//==============Overlord mod=====================
//	Overlord psi plasma grenade
//  Huge copy-pasta from the frag grenade code
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"

#ifndef CLIENT_DLL
#include "grenade_frag.h"
#include "Sprite.h"
#endif

ConVar eo_plasma_grenade_cost("eo_plasma_grenade_cost", "75", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_plasma_grenade_limit("eo_plasma_grenade_limit", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_plasma_grenade_armtime("eo_plasma_grenade_armtime", "1.05", FCVAR_REPLICATED | FCVAR_CHEAT);

#ifdef CLIENT_DLL
#define COverlordPlasmaGrenade C_OverlordPlasmaGrenade
#endif

#include "basegrenade_shared.h"

#define PLASMA_ATTACK1 1
#define PLASMA_ATTACK2 2

#define PLASMA_DAMAGE_RADIUS 250
#define PLASMA_TIMER 2.0f

#define PLASMA_RADIUS 4.0f

#define PLASMA_MODEL "models/Weapons/w_grenade.mdl"

#define BEEP_SOUND "NPC_SScanner.DeployMine"
#define GLOW_SPRITE "sprites/light_glow03.vmt"

#define PLASMA_SIZE Vector(2, 2, 3)

class COverlordPlasma : public CBaseGrenade
{
public:
	DECLARE_CLASS(COverlordPlasma, CBaseGrenade);
	DECLARE_DATADESC();

	COverlordPlasma();
	virtual ~COverlordPlasma();

	virtual void Precache();
	virtual void Spawn();

#ifndef CLIENT_DLL
	virtual void ThinkBecomeSolid()
	{
		SetThink(NULL);

		RemoveSolidFlags(FSOLID_NOT_SOLID);
	}

	virtual void ArmedThink();

	virtual void SetGracePeriod(float flTime);

	virtual void PlasmaTouch( CBaseEntity *pOther );
#endif
	
	void SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );

	virtual bool CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_BBOX, 0, false );
		return true;
	}
private:
#ifndef CLIENT_DLL
	bool m_bTouched;
	CHandle<CSprite> m_hGlow;
	float m_flSpawnTime;
#endif
};

BEGIN_DATADESC(COverlordPlasma)

#ifndef CLIENT_DLL
DEFINE_ENTITYFUNC( PlasmaTouch ),
DEFINE_THINKFUNC( ThinkBecomeSolid ),
DEFINE_THINKFUNC( ArmedThink ),
#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS(plasma_grenade, COverlordPlasma);

COverlordPlasma::COverlordPlasma()
{
#ifndef CLIENT_DLL
	m_bTouched = false;
	m_flSpawnTime = 0.0f;
#endif
}

COverlordPlasma::~COverlordPlasma()
{
}

void COverlordPlasma::Precache()
{
	BaseClass::Precache();

	PrecacheModel(PLASMA_MODEL);
	PrecacheScriptSound(BEEP_SOUND);
	PrecacheModel(GLOW_SPRITE);
}

void COverlordPlasma::Spawn()
{
	//BaseClass::Spawn();

	Precache();

	SetModel(PLASMA_MODEL);
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_DEFAULT );
	//SetSolid( SOLID_BBOX );
	SetSolid( SOLID_VPHYSICS );

#ifndef CLIENT_DLL
	//UTIL_SetSize( this, -PLASMA_SIZE, PLASMA_SIZE);

	SetTouch( &COverlordPlasma::PlasmaTouch );
	SetThink(NULL);
	SetNextThink(TICK_NEVER_THINK);

	//CreateVPhysics();
#endif
	m_takedamage = DAMAGE_NO;
}

#ifndef CLIENT_DLL
void COverlordPlasma::ArmedThink()
{
	if(!m_hGlow)
	{
		
		Vector lightPos;
		QAngle lightAng;

		GetAttachment(LookupAttachment("fuse"), lightPos, lightAng);

		m_hGlow = CSprite::SpriteCreate(GLOW_SPRITE, lightPos, false );

		if(m_hGlow)
		{
			m_hGlow->SetAttachment( this, LookupAttachment( "fuse" ) );
			m_hGlow->SetTransparency( kRenderWorldGlow, 0, 255, 0, 255, kRenderFxNoDissipation );
			m_hGlow->SetBrightness( 160, 0.2f );
			m_hGlow->SetScale( 0.15f);
		}
	}

	bool bInRadius = false;
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive() || pPlayer->IsInvisible())
			continue;

		float dist = (GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

		if(dist <= (PLASMA_DAMAGE_RADIUS))
		{
			bInRadius = true;
			if(dist <= (PLASMA_DAMAGE_RADIUS/2))
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

				if((tr.m_pEnt == pPlayer) || (tr.fraction >= 1.0f))
				{
					Detonate();
					SetNextThink(TICK_NEVER_THINK);
					SetThink(NULL);
					return;
				}
			}
		}
	}

	if(m_hGlow)
	{
		if(bInRadius)
		{
			m_hGlow->SetColor(255, 0, 0);
		}
		else
		{
			m_hGlow->SetColor(0, 255, 0);
		}
	}
	

	SetNextThink(gpGlobals->curtime + 0.1f);
}

void COverlordPlasma::SetGracePeriod(float flTime)
{
	SetThink(&COverlordPlasma::ThinkBecomeSolid);
	SetNextThink(gpGlobals->curtime + flTime);

	AddSolidFlags(FSOLID_NOT_SOLID);
}

void COverlordPlasma::PlasmaTouch(CBaseEntity * pOther)
{
	if(m_bTouched)
		return;

	if(pOther)
	{
		
		if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) )
			return;

		//DevMsg("PlasmaTouch...\n");

		if(pOther->IsPlayer() || pOther->IsNPC())
			return;

		//DevMsg("Touched an entity\n");

		// Check for nearest surface
		Vector dir = GetAbsVelocity();
		VectorNormalize(dir);

		trace_t tr;
		UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + dir * MAX_TRACE_LENGTH, -PLASMA_SIZE, PLASMA_SIZE, 
		MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

		Vector normal = tr.plane.normal;

		QAngle angle;
		VectorAngles(normal, angle);

		SetAbsAngles(angle);

		//Freeze our object
		SetLocalAngularVelocity( QAngle(0, 0, 0) );
		SetAbsVelocity( Vector(0, 0, 0) );
		AddFlag(FL_FROZEN);

		//SetMoveType( MOVETYPE_NONE, MOVECOLLIDE_DEFAULT );

		m_bTouched = true;

		EmitSound(BEEP_SOUND);

		m_takedamage = DAMAGE_YES;
		SetHealth(10);
		SetMaxHealth(10);
		SetCollisionGroup( COLLISION_GROUP_NONE );

		SetSolid(SOLID_VPHYSICS);
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		SetThink(&COverlordPlasma::ArmedThink);
		SetNextThink(gpGlobals->curtime + eo_plasma_grenade_armtime.GetFloat());
	}
}
#endif

void COverlordPlasma::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

#ifndef CLIENT_DLL
COverlordPlasma * Plasmagrenade_create( const Vector &position, const QAngle &angles, const Vector &velocity, 
									   const QAngle &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned )
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	COverlordPlasma *pGrenade = static_cast<COverlordPlasma*>(
		CBaseEntity::Create( "plasma_grenade", position, angles, pOwner )
		);

	if(!pGrenade)
		return NULL;
	
	//pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
/*	pGrenade->SetGracePeriod(0.1f);
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

	return pGrenade;*/

	if ( pGrenade )
	{
		pGrenade->SetLocalAngularVelocity( angVelocity );
		pGrenade->SetAbsVelocity( velocity );
		pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
		pGrenade->SetGracePeriod(0.05f);
	}

	return pGrenade;
	//return Fraggrenade_Create(position, angles, velocity, angVelocity, pOwner, timer, combineSpawned);
}
#endif

class COverlordPlasmaGrenade : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordPlasmaGrenade, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	COverlordPlasmaGrenade();
	virtual ~COverlordPlasmaGrenade();
	virtual bool		  IsOverlordWeapon() const { return true; };

	virtual const char * GetPrimaryMode() const { return "Lob the PSI grenade"; }; 
	virtual const char * GetSecondaryMode() const { return "Roll the PSI grenade"; }; 
	virtual const char * GetCostLabel() const;

	virtual void ItemPostFrame();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	virtual bool Deploy();
	virtual bool Holster(CBaseCombatWeapon * pSwitchingTo);
	virtual void Drop(const Vector &vecVelocity);
	virtual bool Reload();


#endif

private:

	virtual void UpdateGrenadeList();
#ifndef CLIENT_DLL
	virtual void AddGrenadeToList(COverlordPlasma * pGrenade);
	virtual void ThrowGrenade();
	virtual void RollGrenade();
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CUtlVector<CHandle<COverlordPlasma>> m_Grenades;
#endif

	CNetworkVar(int, m_iGrenades);
	CNetworkVar(bool, m_bDrawBackFinished);
	CNetworkVar(bool, m_bDraw);
	CNetworkVar(short, m_iAttack);
};


acttable_t	COverlordPlasmaGrenade::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(COverlordPlasmaGrenade);


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordPlasmaGrenade, DT_OverlordPlasmaGrenade )

BEGIN_NETWORK_TABLE( COverlordPlasmaGrenade, DT_OverlordPlasmaGrenade )
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_iGrenades)),
	SendPropBool(SENDINFO(m_bDrawBackFinished)),
	SendPropBool(SENDINFO(m_bDraw)),
	SendPropInt(SENDINFO(m_iAttack)),
#else
	RecvPropInt(RECVINFO(m_iGrenades)),
	RecvPropBool(RECVINFO(m_bDrawBackFinished)),
	RecvPropBool(RECVINFO(m_bDraw)),
	RecvPropInt(RECVINFO(m_iAttack)),
#endif	
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordPlasmaGrenade )

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_plasmagrenade, COverlordPlasmaGrenade );
PRECACHE_WEAPON_REGISTER(weapon_plasmagrenade);

COverlordPlasmaGrenade::COverlordPlasmaGrenade()
{
	m_bDraw = false;
	m_bDrawBackFinished = false;
	m_iAttack = NULL;
	m_iGrenades = 0;
}

COverlordPlasmaGrenade::~COverlordPlasmaGrenade()
{
}

const char * COverlordPlasmaGrenade::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "%i", eo_plasma_grenade_cost.GetInt());

	return label;
}

void COverlordPlasmaGrenade::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	
	if (!pOwner)
		return;

	if(m_bDrawBackFinished)
	{
		if (!(pOwner->m_nButtons & IN_ATTACK) && !(pOwner->m_nButtons & IN_ATTACK2))
		{
			if(m_iAttack == PLASMA_ATTACK1)
				SendWeaponAnim(ACT_VM_THROW);
			else if(m_iAttack == PLASMA_ATTACK2)
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

void COverlordPlasmaGrenade::PrimaryAttack()
{
	if(m_bDraw)
		return;

	if(Clip1() <= 0)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	UpdateGrenadeList();

	if(m_iGrenades >= eo_plasma_grenade_limit.GetInt())
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.75f;
		pOwner->EmitSound("HL2Player.UseDeny");
		return;
	}

	GET_OVERLORD_DATA_ASSERT(pData);

	if(pData->GetPower() < eo_plasma_grenade_cost.GetInt())
	{
		return;
	}

	SendWeaponAnim(ACT_VM_PULLBACK_HIGH);
	
	m_flTimeWeaponIdle		= FLT_MAX;
	m_flNextPrimaryAttack	= FLT_MAX;
	m_iAttack = PLASMA_ATTACK1;
}

void COverlordPlasmaGrenade::SecondaryAttack()
{
	if(m_bDraw)
		return;

	if(Clip1() <= 0)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	UpdateGrenadeList();

	if(m_iGrenades >= eo_plasma_grenade_limit.GetInt())
	{
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.75f;
		pOwner->EmitSound("HL2Player.UseDeny");
		return;
	}

	GET_OVERLORD_DATA_ASSERT(pData);

	if(pData->GetPower() < eo_plasma_grenade_cost.GetInt())
	{
		return;
	}

	SendWeaponAnim(ACT_VM_PULLBACK_LOW);
	
	m_flTimeWeaponIdle		= FLT_MAX;
	m_flNextSecondaryAttack	= FLT_MAX;
	m_iAttack = PLASMA_ATTACK2;
}

void COverlordPlasmaGrenade::UpdateGrenadeList()
{
#ifndef CLIENT_DLL
	for(int i = 0; i < m_Grenades.Count(); i++)
	{
		if(!m_Grenades[i])
		{
			m_Grenades.Remove(i);
		}
	}

	m_iGrenades = m_Grenades.Count();
#endif
}

#ifndef CLIENT_DLL
void COverlordPlasmaGrenade::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	bool bThrown = false;
	switch(pEvent->event)
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_bDrawBackFinished = true;
			break;
		case EVENT_WEAPON_THROW:
			{
				bThrown = true;
				//m_iClip1--;

				if(m_iAttack == PLASMA_ATTACK1)
					ThrowGrenade();
				else
					RollGrenade();
				break;
			}
		case EVENT_WEAPON_THROW2:
			{
				bThrown = true;
				//m_iClip1--;

				if(m_iAttack == PLASMA_ATTACK1)
					ThrowGrenade();
				else
					RollGrenade();
				break;
			}
		case EVENT_WEAPON_THROW3:
			{
				bThrown = true;
				//m_iClip1--;

				if(m_iAttack =- PLASMA_ATTACK1)
					ThrowGrenade();
				else
					RollGrenade();
				break;
			}
		default:
			BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
			break;
	}

	if(bThrown)
	{
		GetOverlordData()->HandlePowerEvent(EVENT_WEAPONUSED, eo_plasma_grenade_cost.GetInt());
	}
}

bool COverlordPlasmaGrenade::Deploy()
{
	m_bDrawBackFinished = false;

	return BaseClass::Deploy();
}

bool COverlordPlasmaGrenade::Holster(CBaseCombatWeapon * pSwitchingTo)
{
	m_bDrawBackFinished = false;

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordPlasmaGrenade::Drop(const Vector &vecVelocity)
{
	m_bDrawBackFinished = false;

	BaseClass::Drop(vecVelocity);
}

bool COverlordPlasmaGrenade::Reload()
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

void COverlordPlasmaGrenade::AddGrenadeToList(COverlordPlasma * pGrenade)
{
	UpdateGrenadeList();
	m_Grenades.AddToTail(pGrenade);
}

void COverlordPlasmaGrenade::ThrowGrenade()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	Vector	vecEye = pPlayer->RealEyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	
	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 700 + Vector( 0, 0, 55 );
	COverlordPlasma *pGrenade = Plasmagrenade_create( vecSrc, vec3_angle, vecThrow, QAngle(200,random->RandomInt(-600,600),0), pPlayer, PLASMA_TIMER, false );

	if ( pGrenade )
	{
		AddGrenadeToList(static_cast<COverlordPlasma*>(pGrenade));
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( PLASMA_DAMAGE_RADIUS );
	}
	WeaponSound( WPN_DOUBLE );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_bDraw = true;
}

void COverlordPlasmaGrenade::RollGrenade()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecSrc );
	vecSrc.z += PLASMA_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition( pPlayer, pPlayer->WorldSpaceCenter(), vecSrc );

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vecFacing * 350;
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	QAngle rotSpeed(0,0,720);

	vecSrc.z += 54;
	COverlordPlasma *pGrenade = Plasmagrenade_create( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, PLASMA_TIMER, false );

	if ( pGrenade )
	{
		AddGrenadeToList(static_cast<COverlordPlasma*>(pGrenade));
		pGrenade->SetDamage( GetHL2MPWpnData().m_iPlayerDamage );
		pGrenade->SetDamageRadius( PLASMA_DAMAGE_RADIUS );
	}


	WeaponSound( SPECIAL1 );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_bDraw = true;

	m_iGrenades++;
}

void COverlordPlasmaGrenade::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(PLASMA_RADIUS+2,PLASMA_RADIUS+2,PLASMA_RADIUS+2), Vector(PLASMA_RADIUS+2,PLASMA_RADIUS+2,PLASMA_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}
#endif