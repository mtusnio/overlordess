//==============Overlord mod=====================
//	Invisibility
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
#define COverlordInvis C_OverlordInvis
#endif

#define SELF 0
#define FRIEND 1
#define TARGET_SIZE 1

#ifdef OVER_PLAYTEST
ConVar eo_playtest_stealthdistance("eo_playtest_stealthdistance", "800", FCVAR_REPLICATED | FCVAR_NOTIFY);
#else
ConVar eo_playtest_stealthdistance("eo_playtest_stealthdistance", "800", FCVAR_REPLICATED | FCVAR_HIDDEN);
#endif

ConVar eo_invisibility_recharge("eo_invisibility_recharge", "2.5", FCVAR_REPLICATED | FCVAR_CHEAT);

#define MINIMUM_AMMO 0
#define STEALTH_MAX_DIST eo_playtest_stealthdistance.GetInt()
#define STEALTH_WARNING  (0.65 * STEALTH_MAX_DIST)
#define INVIS_THINK		0.1f
#define INVISIBILITY_PARTICLE "InvisibilityEffect"
#define EFFECT_DELAY 4.0f

class COverlordInvis : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordInvis, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
	#ifndef CLIENT_DLL
	DECLARE_DATADESC();
	#endif

	COverlordInvis();
	virtual ~COverlordInvis();
	
	virtual void		  Precache();

#ifndef CLIENT_DLL
	virtual	void		  InvisThink(void);
#endif

	virtual const char *  GetPrimaryMode() const { return "Cloaking device"; }; 
	virtual const char *  GetSecondaryMode() const { return "Cloak teammate"; }; 

	virtual	void		  OnPickedUp( CBaseCombatCharacter *pNewOwner );

	virtual	PlayerClass_t GetClassWeapon() const { return CLASS_STEALTH; };

	virtual bool		  IsMainClassWeapon() const { return true; };

	virtual void		  PrimaryAttack();
	//virtual void		  SecondaryAttack();

	virtual void		  Drop(const Vector & vecVelocity);
	virtual	void		  HandleFireOnEmpty() { PrimaryAttack(); };
	virtual bool		  Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual void		  MakeInvisible(CBasePlayer * pPlayer, bool bExternal = false);
	virtual	void		  UnmakeInvisible();

	//const CBasePlayer *	  GetTarget() const { return m_pTarget.Get(); };
	CBasePlayer *		  GetTarget() const { return m_pTarget.Get(); };
	bool				  IsTargetOwner() const { return GetTarget() == ToBasePlayer(GetOwner()); };

#ifndef CLIENT_DLL
	virtual void		  Kill();
#endif

	bool		  IsInvisible() const { return m_bInvis; };
private:	
	virtual void SelfAttack();
	virtual void FriendAttack();

	virtual void InvisibilityEffect(CBasePlayer * pPlayer);

	void				SetTarget(CBasePlayer * pPlayer) { m_pTarget = pPlayer; };	

#ifndef CLIENT_DLL
	bool		  m_bWarned;
	float		  m_flNextClip;
	float		  m_flNextEffect;
#endif

	CNetworkVar(bool, m_bInvis);
	CNetworkHandle(CBasePlayer, m_pTarget);
};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordInvis, DT_OverlordInvis)

BEGIN_NETWORK_TABLE( COverlordInvis, DT_OverlordInvis)
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bInvis)),
	SendPropEHandle(SENDINFO(m_pTarget)),
#else
	RecvPropBool(RECVINFO(m_bInvis)),
	RecvPropEHandle(RECVINFO(m_pTarget)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordInvis )
	DEFINE_PRED_FIELD(m_bInvis, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_pTarget, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_invisibility, COverlordInvis);
					 

PRECACHE_WEAPON_REGISTER( weapon_invisibility );

acttable_t	COverlordInvis::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(COverlordInvis);

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordInvis)

	DEFINE_FUNCTION(InvisThink),

END_DATADESC()
#endif

COverlordInvis::COverlordInvis()
{
	m_bInvis = false;
#ifndef CLIENT_DLL
	m_bWarned = false;
#endif
}

COverlordInvis::~COverlordInvis()
{
	if(IsInvisible())
		UnmakeInvisible();
}

void COverlordInvis::Precache()
{
	BaseClass::Precache();

	//PrecacheParticleSystem(INVISIBILITY_PARTICLE);

	PrecacheScriptSound("HL2Player.UseDeny");
}

#ifndef CLIENT_DLL
void COverlordInvis::InvisThink(void)
{
	// The amount we want to add to the m_flNextClip
	float nextclip = 1.0f;
	
	if(GetOwner() && GetOwner()->IsAlive() && m_flNextClip <= gpGlobals->curtime)
	{

		// Standard recharge
		if(!IsInvisible() && Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			
			// Do not exceed the limit
			if(Clip1() > GetMaxClip1())
				m_iClip1 = GetMaxClip1();

			nextclip = eo_invisibility_recharge.GetFloat();
		}
		// Only when we are invisible
		else if(IsInvisible() && Clip1() > 0)
		{
			m_iClip1--;

			// So that our ammo never drops below zero
			if(Clip1() < 0)
				m_iClip1 = 0;

			nextclip = 1.0f;
		}
		m_flNextClip = gpGlobals->curtime + nextclip;
	}
		
	if(IsInvisible())
	{
		if(GetTarget() && !GetTarget()->IsInvisible())
		{
			// Runs if the target is not invisible anymore and yet the weapon is still in such state
			UnmakeInvisible();
		}
		// Unmake if our owner got owned
		else if ((!GetOwner() || !GetOwner()->IsAlive()))
		{
			UnmakeInvisible();
		}
		// Unmake if the timer is up
		else if(Clip1() <= 0)
		{
			UnmakeInvisible();
		}	
		else if(GetTarget())
		{
			// If is still invisible and effect should be played, play it
			//if(m_flNextEffect <= gpGlobals->curtime && GetTarget()->GetAbsVelocity().Length() > 1.0f)
			//{
			//	InvisibilityEffect(GetTarget());
			//}


			// Checks whether we have cloaked somebody else and makes sure we are in the 
			// wanted distance
			if(!IsTargetOwner())
			{
				CBasePlayer * pOwner = ToBasePlayer(GetOwner());
				if(!pOwner)
					return;

				//trace_t tr;
				float dist;

				Vector vecDist = GetTarget()->GetAbsOrigin() - pOwner->GetAbsOrigin();
				
				dist = vecDist.Length();

				if(!GetTarget()->IsAlive() || !GetTarget()->IsRebel())
				{
					UnmakeInvisible();
				}
				else if(dist > STEALTH_MAX_DIST)
				{
					UnmakeInvisible();
				}
				// Warn the stealth guy that his target is going too far
				else if(dist >= STEALTH_WARNING && !m_bWarned)
				{
					m_bWarned = true;
					ClientPrint(pOwner, HUD_PRINTTALK, "Your cloaked target is too far, should he move even further he will lose invisibility!");
				}
				else if(m_bWarned && dist < STEALTH_WARNING)
				{
					m_bWarned = false;
				}

		
			}
		}
	}

	SetNextThink(gpGlobals->curtime + INVIS_THINK);

}
#endif

void COverlordInvis::OnPickedUp(CBaseCombatCharacter * pNewOwner)
{
#ifndef CLIENT_DLL
	BaseClass::OnPickedUp(pNewOwner);

	m_flNextClip = gpGlobals->curtime + 2.0f;

	SetThink(&COverlordInvis::InvisThink);
	SetNextThink(gpGlobals->curtime + INVIS_THINK);
#endif
}

bool COverlordInvis::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Older version
	/*if(IsInvisible() && GetOwner())
	{
		GetOwner()->EmitSound("HL2Player.UseDeny");
		return false;
	}

	bool temp = BaseClass::Holster(pSwitchingTo);
#ifndef CLIENT_DLL
	SetThink(&COverlordInvis::InvisThink);
#endif
	return temp;*/

	if(IsInvisible())
		UnmakeInvisible();

	bool temp = BaseClass::Holster(pSwitchingTo);

#ifndef CLIENT_DLL
	SetThink(&COverlordInvis::InvisThink);
#endif
	return temp;
}


void COverlordInvis::Drop(const Vector &vecVelocity) 
{ 
	UnmakeInvisible();

	BaseClass::Drop(vecVelocity);
}

void COverlordInvis::PrimaryAttack()
{
	SelfAttack();

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
}


/*void COverlordInvis::SecondaryAttack()
{
	if(m_flNextSecondaryAttack > gpGlobals->curtime)
		return;

	FriendAttack();

	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}*/


void COverlordInvis::SelfAttack()
{
	if(IsInvisible())
	{
		UnmakeInvisible();
		return;
	}
	
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;

	MakeInvisible(pPlayer);
}

void COverlordInvis::FriendAttack(void)
{
	if(IsInvisible())
	{
		UnmakeInvisible();
		return;
	}

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	trace_t tr;
	Vector vecPos, vecDir, vecEnd;
	
	vecPos = pPlayer->Weapon_ShootPosition();
	AngleVectors(pPlayer->EyeAngles(), &vecDir);
	VectorMA(vecPos, 72, vecDir, vecEnd);

	UTIL_TraceLine(vecPos, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

	if(!tr.m_pEnt)
		return;

	if(!tr.m_pEnt->IsPlayer())
		return;

	CBasePlayer * pTarget = ToBasePlayer(tr.m_pEnt);
	
	if(!pTarget)
		return;

	MakeInvisible(pTarget, true);
}


void COverlordInvis::MakeInvisible(CBasePlayer * pPlayer, bool bExternal)
{
#ifndef CLIENT_DLL
	if(GetNextThink() == TICK_NEVER_THINK)
	{
		SetThink(&COverlordInvis::InvisThink);
		SetNextThink(gpGlobals->curtime + INVIS_THINK);
	}
#endif

	// Do not return, just disable invisibility, then activate it on another target, returning
	// should be handled in another function
#ifndef CLIENT_DLL
	if(IsInvisible())
		UnmakeInvisible();

	if(pPlayer)
		pPlayer->MakeInvisible(bExternal);

	m_bInvis = true;


	m_bWarned = false;
#endif

	//InvisibilityEffect(pPlayer);

	SetTarget(pPlayer);
}

void COverlordInvis::UnmakeInvisible()
{
	if(!IsInvisible())
		return;

#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = GetTarget();

	if(pPlayer)
		pPlayer->UnmakeInvisible();

	m_bWarned = false;
#endif

	SetTarget(NULL);

	//InvisibilityEffect(pPlayer);

	m_bInvis = false;
}

#ifndef CLIENT_DLL
void COverlordInvis::Kill()
{
	if(IsInvisible())
		UnmakeInvisible();
	

	
	BaseClass::Kill();
}
#endif

void COverlordInvis::InvisibilityEffect(CBasePlayer * pPlayer)
{
//#ifndef CLIENT_DLL
	if(!pPlayer)
		return;

	// We need to make it appear from the player's torso
	Vector org = pPlayer->WorldSpaceCenter();

	DispatchParticleEffect(INVISIBILITY_PARTICLE, org, QAngle(0, 0, 0));
#ifndef CLIENT_DLL
	m_flNextEffect = gpGlobals->curtime + EFFECT_DELAY; 
#endif
}