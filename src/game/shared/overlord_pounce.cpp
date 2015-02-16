//==============Overlord mod=====================
//	Pounce weapon, let's the Ov pounce on
//	rebels and hit them with the blade
//===============================================


#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"

#ifdef CLIENT_DLL
#define COverlordPounce C_OverlordPounce
#endif

#define POUNCE_RANGE 256
#define OVER_POUNCE_DEVMSG

namespace
{
	enum
	{
		NONE = 0,
		HOMED,
		POUNCING,
	};
}
class COverlordPounce : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS(COverlordPounce, CBaseHL2MPBludgeonWeapon);
	DECLARE_PREDICTABLE();
#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	DECLARE_NETWORKCLASS(); 

	COverlordPounce() { };
	virtual ~COverlordPounce() { };

	virtual bool IsOverlordWeapon() { return true; };

	virtual float GetDamageForActivity(Activity hitActivity) { return 20; };
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	int    GetPounceState() const { return m_Pounce; };
#ifndef CLIENT_DLL
	virtual void ItemPostFrame();
#endif
private:
#ifndef CLIENT_DLL
	CNetworkVar(int, m_Pounce);
	CHandle<CBasePlayer> m_Target;
#else
	int m_Pounce;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordPounce, DT_OverlordPounce)

BEGIN_NETWORK_TABLE(COverlordPounce, DT_OverlordPounce)
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_Pounce)),
	//SendPropEHandle(SENDINFO(m_Target)),
#else
	RecvPropInt(RECVINFO(m_Pounce)),
	//RecvPropEHandle(RECVINFO(m_Target)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordPounce)
	DEFINE_PRED_FIELD(m_Pounce, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_pounce, COverlordPounce);
PRECACHE_WEAPON_REGISTER(COverlordPounce);

#ifndef CLIENT_DLL
acttable_t COverlordPounce::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_MELEE,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_MELEE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_MELEE,					false },
};



IMPLEMENT_ACTTABLE(COverlordPounce);

#endif

void COverlordPounce::PrimaryAttack()
{
	if(m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

#ifndef CLIENT_DLL
	if(GetPounceState() == NONE)
	{
		Vector forw;
		AngleVectors(EyeAngles(), &forw);
		Vector end;
		VectorMA(EyePosition(), POUNCE_RANGE, forw, end);
		trace_t tr;
		UTIL_TraceLine(EyePosition(), end, MASK_SHOT, GetOwner(), COLLISION_GROUP_NONE, &tr);

		CBasePlayer * pPlayer = NULL;
		if(!tr.m_pEnt || !tr.m_pEnt->IsPlayer())
		{
			UTIL_TraceHull(EyePosition(), end, -Vector(8, 8, 8), Vector(8, 8, 8), MASK_SHOT, GetOwner(), COLLISION_GROUP_NONE, &tr);
			if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
				pPlayer = ToBasePlayer(tr.m_pEnt);
		}
		else
		{
			pPlayer = ToBasePlayer(tr.m_pEnt);
		}

		if(pPlayer)
		{
#ifndef CLIENT_DLL
			m_Target = pPlayer;
#endif
			m_Pounce = HOMED;
#ifdef OVER_POUNCE_DEVMSG
			DevMsg("Homed\n");
#endif
		}
	}
	else if(GetPounceState() == HOMED)
	{
		m_Pounce = POUNCING;
#ifdef OVER_POUNCE_DEVMSG
		DevMsg("Pouncing\n");
#endif
	}
	else if(GetPounceState() == POUNCING)
	{
		BaseClass::PrimaryAttack();
		m_Pounce = NONE;
#ifdef OVER_POUNCE_DEVMSG
		DevMsg("Pounced\n");
#endif
	}
#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.65f;
}

void COverlordPounce::SecondaryAttack()
{
	if(m_flNextSecondaryAttack > gpGlobals->curtime)
		return;


	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

#ifndef CLIENT_DLL
void COverlordPounce::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if(m_Target && (GetPounceState() == HOMED || GetPounceState() == POUNCING))
	{
		DevMsg("Item post frame pounce.\n");
		// General code for autoaiming
		CBasePlayer * pOwner = ToBasePlayer(GetOwner());
 
		if(!pOwner)
		{
			DevMsg("No owner found\n");
			return;
		}
		DevMsg("Homing...\n");

		Vector dir = (m_Target->EyePosition() - Vector(0, 0, 5)) - EyePosition();
		QAngle angle;
		VectorAngles(dir, angle);

		pOwner->SnapEyeAngles(angle);

		// Code for the pounce
		if(GetPounceState() == POUNCING)
		{
		}
	}
}
#endif