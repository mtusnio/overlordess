//==============Overlord mod=====================
//	Overlord's stasis weapon
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "overlord_data.h"

#include "particle_parse.h"
#include "gameeventlistener.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#endif

#ifdef CLIENT_DLL
#define COverlordStasis C_OverlordStasis
#endif

ConVar eo_stasis_cost("eo_stasis_cost", "50", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stasis_range("eo_stasis_range", "1640", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stasis_duration("eo_stasis_duration", "3.75", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stasis_damage("eo_stasis_damage", "8", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stasis_slowdown("eo_stasis_slowdown", "130", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_stasis_slowdown_duration("eo_stasis_slowdown_duration", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT);

#define STASIS_PARTICLE "Stasis"
#define STASIS_SOUND "d3_citadel.zapper_warmup"

class COverlordStasis : public CBaseHL2MPCombatWeapon, public CGameEventListener
{
public:
	DECLARE_CLASS(COverlordStasis, CBaseHL2MPCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

	COverlordStasis();
	virtual ~COverlordStasis();

	virtual void Precache();
	virtual void Spawn();

	virtual void PrimaryAttack();

#ifndef CLIENT_DLL
	virtual void StasisThink();

	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
#endif

	virtual void FireGameEvent(IGameEvent * event);

	virtual bool IsOverlordWeapon() { return true; };

	virtual const char * GetPrimaryMode() const { return "Put a player in stasis"; }; 
	virtual const char * GetSecondaryMode() const { return "None"; }; 
	virtual const char * GetCostLabel() const;

#ifndef CLIENT_DLL
	virtual void PutInStasis(CBasePlayer * pPlayer);
	virtual void RemoveFromStasis();
#endif
private:
#ifndef CLIENT_DLL
	void UpdateNextDamage() { m_flNextDamage = gpGlobals->curtime + (eo_stasis_duration.GetFloat() - 1)/((eo_stasis_damage.GetFloat() - 1)); };
#endif
	CNetworkHandle(CBasePlayer, m_hInStasis);
#ifndef CLIENT_DLL
	float m_flStatisTime;
	float m_flNextDamage;
	int	  m_iDamageCounter;
#else
	CNewParticleEffect * m_Effect;
#endif
};

BEGIN_DATADESC(COverlordStasis)
#ifndef CLIENT_DLL

//DEFINE_FUNCTION(StasisThink),

#endif
END_DATADESC()

LINK_ENTITY_TO_CLASS(weapon_stasis, COverlordStasis);

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordStasis, DT_OverlordStasis)
PRECACHE_WEAPON_REGISTER(weapon_stasis);

BEGIN_NETWORK_TABLE(COverlordStasis, DT_OverlordStasis)
#ifndef CLIENT_DLL
SendPropEHandle(SENDINFO(m_hInStasis)),
#else
RecvPropEHandle(RECVINFO(m_hInStasis)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordStasis)


END_PREDICTION_DATA()
#endif


acttable_t COverlordStasis::m_acttable[] = 
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

IMPLEMENT_ACTTABLE(COverlordStasis);


COverlordStasis::COverlordStasis()
{
	m_hInStasis = NULL;
#ifndef CLIENT_DLL
	m_flStatisTime = 0.0f;
	m_flNextDamage = 0.0f;
	m_iDamageCounter = 0;
#else
	m_Effect = NULL;
#endif
}

COverlordStasis::~COverlordStasis()
{
#ifndef CLIENT_DLL
	RemoveFromStasis();
#endif
}

void COverlordStasis::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(STASIS_PARTICLE);
	PrecacheScriptSound(STASIS_SOUND);
}

void COverlordStasis::Spawn()
{
	BaseClass::Spawn();

#ifdef CLIENT_DLL
	ListenForGameEvent("stasis_start");
	ListenForGameEvent("stasis_end");
#endif
}

void COverlordStasis::PrimaryAttack()
{
#ifndef CLIENT_DLL
	if(m_hInStasis)
		return;

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	COverlordData & data = *GetOverlordData();

	if(data.GetPower() < eo_stasis_cost.GetInt())
		return;

	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetPlayerOwner() );
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( pPlayer, pPlayer->GetCurrentCommand() );

	Vector start = pOwner->RealEyePosition();
	Vector forw;
	AngleVectors(pOwner->RealEyeAngles(), &forw);
	//Vector end = start + forw * eo_stasis_range.GetFloat();
	Vector end;
	VectorMA(start, eo_stasis_range.GetFloat(), forw, end);

	trace_t tr;
	UTIL_TraceLine(start, end, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tr);

	bool bStasised = false;
	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
		CBasePlayer * pPlayer = ToBasePlayer(tr.m_pEnt);

		if(pPlayer && pPlayer->IsRebel() && pPlayer->IsAlive())
		{
			data.HandlePowerEvent(EVENT_WEAPONUSED, eo_stasis_cost.GetInt());
			pOwner->EmitSound(STASIS_SOUND);
			PutInStasis(pPlayer);
			m_flNextPrimaryAttack = gpGlobals->curtime + eo_stasis_duration.GetFloat() + 1.5f;
			bStasised = true;
		}
	}

	if(!bStasised)
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.65f;

	

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( pPlayer );
#endif
}

void COverlordStasis::FireGameEvent(IGameEvent * event)
{
#ifdef CLIENT_DLL
	if(event)
	{
		if(!Q_stricmp("stasis_start", event->GetName()))
		{
			for(int i = 1; i <= gpGlobals->maxClients; i++)
			{
				C_BasePlayer * pPlayer = UTIL_PlayerByIndex(i);

				if(!pPlayer)
					continue;

				if(pPlayer->GetUserID() == event->GetInt("userid"))
				{
					m_Effect = pPlayer->ParticleProp()->Create(STASIS_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);
					break;
				}
			}
		}
		else if(!Q_stricmp("stasis_end", event->GetName()))
		{
			if(m_Effect)
			{
				for(int i = 1; i <= gpGlobals->maxClients; i++)
				{
					C_BasePlayer * pPlayer = UTIL_PlayerByIndex(i);

					if(!pPlayer)
						continue;

					if(pPlayer->GetUserID() == event->GetInt("userid"))
					{
						pPlayer->ParticleProp()->StopEmission(m_Effect);
						m_Effect = NULL;
						break;
					}
				}
			}
		}
	}
#endif
}

#ifndef CLIENT_DLL
void COverlordStasis::StasisThink()
{
	if(!m_hInStasis)
	{
		SetNextThink(TICK_NEVER_THINK);
		return;
	}

	if((m_flStatisTime + eo_stasis_duration.GetFloat()) <= gpGlobals->curtime)
	{
		if(m_iDamageCounter < eo_stasis_damage.GetInt())
		{
			int damage = eo_stasis_damage.GetInt() - m_iDamageCounter;
			m_hInStasis->OnTakeDamage(CTakeDamageInfo(this, this, damage, DMG_GENERIC));
		}
		RemoveFromStasis();
		return;
	}

	if((m_flNextDamage <= gpGlobals->curtime) 
		&& m_iDamageCounter < eo_stasis_damage.GetInt() 
		&& (eo_stasis_damage.GetInt() > 0))
	{
		m_iDamageCounter++;
		m_hInStasis->OnTakeDamage(CTakeDamageInfo(this, this, 1, DMG_GENERIC));
		UpdateNextDamage();
	}


	SetNextThink(gpGlobals->curtime + 0.2f);
}

bool COverlordStasis::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	bool bRet = BaseClass::Holster(pSwitchingTo);

	if(m_hInStasis.Get())
	{
		SetThink(&COverlordStasis::StasisThink);
	}

	return bRet;
}

#endif



const char * COverlordStasis::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "%i", eo_stasis_cost.GetInt());

	return label;
}

#ifndef CLIENT_DLL
void COverlordStasis::PutInStasis(CBasePlayer * pPlayer)
{
	if(!pPlayer || m_hInStasis)
		return;

	pPlayer->AddFlag(FL_FROZEN);
	pPlayer->SetInvulnerable(true);

	if(eo_stasis_damage.GetInt() > 0)
	{
		pPlayer->OnTakeDamage(CTakeDamageInfo(this, this, 1, DMG_GENERIC));
		m_iDamageCounter++;
		UpdateNextDamage();
	}

	//DispatchParticleEffect(STASIS_PARTICLE, PATTACH_ABSORIGIN_FOLLOW, pPlayer);

	m_hInStasis = pPlayer;
	m_flStatisTime = gpGlobals->curtime;
	
	// Event
	IGameEvent * event = gameeventmanager->CreateEvent("stasis_start");
	if(event)
	{
		event->SetInt("userid", pPlayer->GetUserID());
		gameeventmanager->FireEvent(event);
	}

	SetThink(&COverlordStasis::StasisThink);
	SetNextThink(gpGlobals->curtime + 0.2f);
}

void COverlordStasis::RemoveFromStasis()
{
	if(!m_hInStasis)
		return;

	// Event
	IGameEvent * event = gameeventmanager->CreateEvent("stasis_end");
	if(event)
	{
		event->SetInt("userid", m_hInStasis->GetUserID());
		gameeventmanager->FireEvent(event);
	}

	m_hInStasis->RemoveFlag(FL_FROZEN);
	m_hInStasis->SetInvulnerable(false);
	m_hInStasis->AddSlowdown(eo_stasis_slowdown.GetInt(), eo_stasis_slowdown_duration.GetFloat(),
		this);

	m_flStatisTime = 0.0f;
	m_flNextDamage = 0.0f;
	m_iDamageCounter = 0;
	m_hInStasis = NULL;

	SetNextThink(TICK_NEVER_THINK);
}

#endif