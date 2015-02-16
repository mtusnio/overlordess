//==============Overlord mod=====================
//	Hacking device
//	
//===============================================

#include "cbase.h"
#include "overlord_dynamictraps.h"
#include "overlord_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordHackDevice C_OverlordHackDevice
#endif

#ifndef CLIENT_DLL
ConVar eo_hackent_range("eo_hackent_range", "250", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_hackent_lifetime("eo_hackent_liefetime", "8", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordHackEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordHackEntity, CBaseAnimating);

	COverlordHackEntity();
	virtual ~COverlordHackEntity();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

	CBasePlayer * GetOwner() { return m_hHacker; };
	void		  SetOwner(CBasePlayer * pPlayer) { m_hHacker = pPlayer; };
private:
	void AddToList(COverlordTrap * pTrap);
	void RemoveFromList(COverlordTrap * pTrap);

	float m_flDeathTime;
	CUtlVector<CHandle<COverlordTrap>> m_Traps;
	CHandle<CBasePlayer> m_hHacker;
};

LINK_ENTITY_TO_CLASS(overlord_hackentity, COverlordHackEntity);

COverlordHackEntity::COverlordHackEntity()
{
	m_flDeathTime = 0.0f;
}

COverlordHackEntity::~COverlordHackEntity()
{
	for(int i = m_Traps.Count()-1; i >= 0; i--)
	{
		COverlordTrap * pTrap = m_Traps[i];

		if(!pTrap)
			continue;

		RemoveFromList(pTrap);
	}
}

void COverlordHackEntity::Precache()
{
	PrecacheModel("models/props_combine/breenclock.mdl");
}

void COverlordHackEntity::Spawn()
{
	Precache();

	SetModel("models/props_combine/breenclock.mdl");

	SetSolid(SOLID_VPHYSICS);
	SetMoveType(MOVETYPE_NONE);

	m_flDeathTime = gpGlobals->curtime + eo_hackent_lifetime.GetFloat();

	SetThink(&COverlordHackEntity::Think);
	SetNextThink(gpGlobals->curtime + 0.2f);
}

void COverlordHackEntity::Think()
{
	if(m_flDeathTime <= gpGlobals->curtime)
	{
		UTIL_Remove(this);
		SetNextThink(TICK_NEVER_THINK);	
		return;
	}

	COverlordData & data = *GetOverlordData();

	for(int i = 0; i < data.GetMaxTraps(); i++)
	{
		COverlordTrap * pTrap = data.GetTrap(i);

		if(!pTrap)
			continue;


		float dist = (GetAbsOrigin() - pTrap->GetAbsOrigin()).Length();

		trace_t tr;
		UTIL_TraceLine(WorldSpaceCenter(), pTrap->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
		if((dist <= eo_hackent_range.GetFloat()) && (tr.m_pEnt == pTrap))
		{
			AddToList(pTrap);
		}
		else
		{
			RemoveFromList(pTrap);
		}
	}



	SetNextThink(gpGlobals->curtime + 0.2f);
}

void COverlordHackEntity::AddToList(COverlordTrap * pTrap)
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(m_Traps.Find(pTrap) != -1)
			return;

	pTrap->AddTrapDefenseRef();
	if(!pTrap->GetLastHacker())
		pTrap->SetLastHacker(pPlayer);

	m_Traps.AddToTail(pTrap);
}

void COverlordHackEntity::RemoveFromList(COverlordTrap * pTrap)
{
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(m_Traps.Find(pTrap) == -1)
		return;

	if(pTrap->GetLastHacker() == pPlayer)
		pTrap->SetLastHacker(NULL);

	m_Traps.FindAndRemove(pTrap);
	pTrap->RemoveTrapDefenseRef();
}
#endif

class COverlordHackDevice : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordHackDevice, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

	COverlordHackDevice();
	virtual ~COverlordHackDevice();

	virtual void Precache();

	virtual void PrimaryAttack();

	virtual PlayerClass_t GetClassWeapon() const { return CLASS_HACKER; };

	virtual bool		  IsMainClassWeapon() const { return true; };
private:
	CNetworkVar(float, m_flHitTime);
};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordHackDevice, DT_OverlordHackDevice)

BEGIN_NETWORK_TABLE( COverlordHackDevice, DT_OverlordHackDevice)
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flHitTime)),
#else
	RecvPropFloat(RECVINFO(m_flHitTime)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordHackDevice )


END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_hackdevice, COverlordHackDevice);


PRECACHE_WEAPON_REGISTER( weapon_hackdevice );



acttable_t	COverlordHackDevice::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SLAM,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SLAM,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SLAM,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SLAM,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SLAM,					false },
};

IMPLEMENT_ACTTABLE(COverlordHackDevice);



COverlordHackDevice::COverlordHackDevice()
{
	m_flHitTime = 0.0f;
}

COverlordHackDevice::~COverlordHackDevice()
{

}

void COverlordHackDevice::Precache()
{
	BaseClass::Precache();

}


void COverlordHackDevice::PrimaryAttack()
{
	if(m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	Vector dir;
	AngleVectors(pPlayer->RealEyeAngles(), &dir);

	trace_t tr;
	UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), pPlayer->Weapon_ShootPosition() + dir * MAX_TRACE_LENGTH, MASK_SHOT_HULL, 
		pPlayer, COLLISION_GROUP_NONE, &tr);

	COverlordHackEntity * pEnt = static_cast<COverlordHackEntity*>(CreateEntityByName("overlord_hackentity"));

	if(!pEnt)
	{
		Warning("Couldn't create hackentity!\n");
		return;
	}

	if(tr.m_pEnt && !tr.m_pEnt->IsWorld())
		pEnt->SetParent(tr.m_pEnt);

	
	QAngle normalAngle;
	VectorAngles(tr.plane.normal, normalAngle);

	Vector forw, right, up;
	AngleVectors(normalAngle, &forw, &right, &up);

	QAngle final;
	VectorAngles(up, forw, final);

	pEnt->SetAbsAngles(final);
	pEnt->SetAbsOrigin(tr.endpos + forw * 4);
	pEnt->SetOwner(pPlayer);
	pEnt->Spawn();

	m_iClip1--;
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.45f;
}

