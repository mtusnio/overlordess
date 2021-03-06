//######Overlord#######
//	Psychokinesis 
//	weapon
//#####################

#include "cbase.h"
#include "in_buttons.h"
#include "baseplayer_shared.h"
#include "particle_parse.h"
#include "gameeventlistener.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "particles_new.h"
#else
	#include "hl2mp_player.h"
#endif

#include "hl2mp_gamerules.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordShield C_OverlordShield
#endif

#define SHIELD_THINK 0.1f
#define SHIELD_TYPES 2
#define SHIELD_DELAY 0.8f

#define BULLET_SHIELD_PARTICLE "BulletShield"
#define PHYSICAL_SHIELD_PARTICLE "PhysicalShield"

ConVar eo_shield_drain("eo_shield_drain", "23", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_shield_drain_time("eo_shield_drain_time", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT);

#define POWER_DRAIN eo_shield_drain.GetInt()
#define POWER_DRAIN_DELAY eo_shield_drain_time.GetFloat();

#define ACTIVATION_SOUND "NPC_FloorTurret.Activate"
//#define PSI PSI_SHIELD

class COverlordShield : public CBaseHL2MPCombatWeapon, public CGameEventListener
{
public:
	DECLARE_CLASS(COverlordShield, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	COverlordShield(void);
	virtual ~COverlordShield(void);

	virtual void Spawn();
	virtual void Precache();

	virtual void Drop(const Vector &vecVelocity);
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual void PrimaryAttack(void);
	virtual void SecondaryAttack(void);


	virtual void ItemPostFrame();
#ifdef CLIENT_DLL
	virtual int DrawModel(int flags);
	//virtual void ItemHolsterFrame();
#endif

	virtual	void		  HandleFireOnEmpty() { PrimaryAttack(); };
	
	virtual bool		  IsOverlordWeapon() const { return true; };

	virtual void          FireGameEvent( IGameEvent *event );

	virtual const char * GetPrimaryMode() const { return "180 degree bullet shield"; }; 
	virtual const char * GetSecondaryMode() const { return "Physical shield"; }; 
	virtual const char * GetCostLabel() const;
private:
	void HandleParticles();

	void DisableProtection();
	void ActivateShield(bool bActivate, unsigned short Type = BULLET_SHIELD);
	bool IsProtected() const { return m_bActivated; };

	CNetworkVar(bool, m_bActivated);

	// A little hack, we send which particles to use
	CNetworkVar(bool, m_bBullet);

	//int m_nTexture;

#ifdef CLIENT_DLL
	CNewParticleEffect * m_hEffect;
#endif
	CNetworkVar(float, m_flDrainTime);

};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordShield, DT_OverlordShield)

BEGIN_NETWORK_TABLE(COverlordShield, DT_OverlordShield)
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bActivated)),
	SendPropBool(SENDINFO(m_bBullet)),
	SendPropFloat(SENDINFO(m_flDrainTime)),
#else
	RecvPropBool(RECVINFO(m_bActivated)),
	RecvPropBool(RECVINFO(m_bBullet)),
	RecvPropFloat(RECVINFO(m_flDrainTime)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(COverlordShield)
#ifdef CLIENT_DLL
	//DEFINE_PRED_FIELD(m_bActivated, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_overshield, COverlordShield);
PRECACHE_WEAPON_REGISTER(weapon_overshield);


acttable_t COverlordShield::m_acttable[] = 
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



IMPLEMENT_ACTTABLE(COverlordShield);

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordShield)

	DEFINE_FUNCTION(Think),

END_DATADESC()
#endif

COverlordShield::COverlordShield(void)
{
	m_bActivated = false;
	m_bBullet = true;
#ifdef CLIENT_DLL
	m_hEffect = NULL;
#endif
	m_flDrainTime = 0.0f;
}

COverlordShield::~COverlordShield(void)
{
	DisableProtection();
#ifdef CLIENT_DLL
	if(m_hEffect)
	{
		m_hEffect->StopEmission();
		m_hEffect = NULL;
	}
#endif
}

void COverlordShield::Spawn()
{
	BaseClass::Spawn();

#ifdef CLIENT_DLL
	//ListenForGameEvent("shield_enabled");
	ListenForGameEvent("shield_disabled");
#endif
}

void COverlordShield::Precache()
{
	BaseClass::Precache();
	/*sprites/lgtning.vmt*/
	PrecacheParticleSystem(BULLET_SHIELD_PARTICLE);
	PrecacheParticleSystem(PHYSICAL_SHIELD_PARTICLE);
	PrecacheScriptSound(ACTIVATION_SOUND);

	//m_nTexture = PrecacheModel("sprites/laserbeam.vmt");
}


bool COverlordShield::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	DisableProtection();
		
	return BaseClass::Holster(pSwitchingTo);
}

void COverlordShield::Drop(const Vector &vecVelocity)
{
	DisableProtection();

	BaseClass::Drop(vecVelocity);
}

void COverlordShield::PrimaryAttack(void)
{	
	if(((GET_OVERLORD_DATA)->GetPower() >= POWER_DRAIN) || m_bActivated)
	{
		EmitSound(ACTIVATION_SOUND);
		ActivateShield(!m_bActivated, BULLET_SHIELD);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + SHIELD_DELAY;

}

void COverlordShield::SecondaryAttack(void)
{
	if(m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	if(((GET_OVERLORD_DATA)->GetPower() >= POWER_DRAIN) || m_bActivated)
	{
		EmitSound(ACTIVATION_SOUND);
		ActivateShield(!m_bActivated, PHYS_SHIELD);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + SHIELD_DELAY;
}

#ifdef CLIENT_DLL
void COverlordShield::HandleParticles()
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;

	if(m_bActivated)
	{
		if(m_hEffect)
			return;

		//int Type = pPlayer->GetProtection();
		const char attachment[] = "zipline";

		if(m_bBullet)
		{	
			m_hEffect = pPlayer->ParticleProp()->Create(BULLET_SHIELD_PARTICLE, PATTACH_POINT_FOLLOW, attachment);
		}
		else //if(Type & PHYS_SHIELD)
		{
			m_hEffect = pPlayer->ParticleProp()->Create(PHYSICAL_SHIELD_PARTICLE, PATTACH_POINT_FOLLOW, attachment);
		}
		//else
		//{
		//	Warning("Wrong shield type!\n");
		//}

		if(m_hEffect)
		{
			m_hEffect->SetControlPointEntity(0, pPlayer);
		}
	}
	else
	{
		if(m_hEffect)
		{
			pPlayer->ParticleProp()->StopEmission(m_hEffect);
			m_hEffect = NULL;
		}
	}
}
#endif

#ifdef CLIENT_DLL
int COverlordShield::DrawModel(int flags)
{
	HandleParticles();

	return BaseClass::DrawModel(flags);
}
#endif
void COverlordShield::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

#ifdef CLIENT_DLL
	// Only for owner.
	HandleParticles();
#else
	if(m_bActivated)
	{
		COverlordData & data = *GET_OVERLORD_DATA;

		if(data.GetPower() < POWER_DRAIN)
		{
			EmitSound(ACTIVATION_SOUND);
			DisableProtection();
			return;
		}

		if(m_flDrainTime <= gpGlobals->curtime)
		{
			data.HandlePowerEvent(EVENT_WEAPONUSED, POWER_DRAIN);
			m_flDrainTime = gpGlobals->curtime + POWER_DRAIN_DELAY;
		}
	}
#endif
}

void COverlordShield::FireGameEvent( IGameEvent *event )
{
	if(event)
	{
#ifdef CLIENT_DLL
		/*if(!Q_stricmp(event->GetName(), "shield_enabled"))
		{
			ActivateShield(true, event->GetInt("type"));
			//HandleParticles();
		}
		else*/ if(!Q_stricmp(event->GetName(), "shield_disabled"))
		{
			DisableProtection();
			//HandleParticles();
		}
#endif
	}
}

const char * COverlordShield::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "%i per second", eo_shield_drain.GetInt());

	return label;
}
void COverlordShield::ActivateShield(bool bActivate, unsigned short Type)
{
	if(bActivate == IsProtected())
		return;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	if(IsProtected())
	{
		DisableProtection();
		return;
	}

	m_flDrainTime = gpGlobals->curtime + POWER_DRAIN_DELAY;

	if(Type == BULLET_SHIELD)
		m_bBullet = true;
	else
		m_bBullet = false;
#ifndef CLIENT_DLL
	if(bActivate)
	{
		pPlayer->EnableProtection(Type);
	}
	else
	{
		pPlayer->DisableProtection(Type);
	}

	IGameEvent * event = gameeventmanager->CreateEvent("shield_enabled", true);
	if(event)
	{
		event->SetInt("userid", pPlayer->GetUserID());
		event->SetInt("type", Type);

		gameeventmanager->FireEvent(event);
	}
#endif

	m_bActivated = bActivate;
}


void COverlordShield::DisableProtection()
{
	m_bActivated = false;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	pPlayer->DisableProtection(BULLET_SHIELD);
	pPlayer->DisableProtection(PHYS_SHIELD);

#ifdef CLIENT_DLL
	if(m_hEffect)
	{
		pPlayer->ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}
#else
	IGameEvent * event = gameeventmanager->CreateEvent("shield_disabled", true);
	if(event)
	{
		event->SetInt("userid", pPlayer->GetUserID());

		gameeventmanager->FireEvent(event);
	}
#endif
}