//==============Overlord mod=====================
//	Fast-firing minigun for the Overlordess
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_gamerules.h"

#ifdef CLIENT_DLL
#include "beam_shared.h"
#endif

#ifdef CLIENT_DLL
#define COverlordMinigun C_OverlordMinigun
#endif

ConVar eo_minigun_cost("eo_minigun_cost", "8", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_minigun_fire_time("eo_minigun_fire_time", "0.95", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_minigun_slowdown("eo_minigun_slowdown", "135", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_minigun_damage("eo_minigun_damage", "6", FCVAR_REPLICATED | FCVAR_CHEAT);

#define REVOLVE_SOUND "d3_citadel.weapon_zapper_beam_loop1"
#define SLOWDOWN_SPEED eo_minigun_slowdown.GetInt()
#define BEAM_EFFECT "sprites/laserbeam.vmt"
#define BEAM_HULL Vector(2, 2, 2)

class COverlordMinigun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordMinigun, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	COverlordMinigun();
	virtual ~COverlordMinigun();

	virtual void Precache();
	virtual void PrimaryAttack();

	virtual void ItemPostFrame();
#ifdef CLIENT_DLL
	virtual int  DrawModel(int flags);
	virtual void ViewModelDrawn(C_BaseViewModel *pViewModel);
#endif

	virtual bool CanHolster();

	virtual bool		  IsOverlordWeapon() const { return true; };
	virtual bool HasAnyAmmo() { return true; };
	virtual bool CanBeSelected() { return true; };

	virtual const char * GetPrimaryMode() const { return "Fire a beam"; }; 
	virtual const char * GetSecondaryMode() const { return "None"; }; 
	virtual const char * GetCostLabel() const;
private:
#ifdef CLIENT_DLL
	CBeam * m_Beam;

	void CreateBeam();
	void DestroyBeam();
	void DrawBeam(C_BaseViewModel * pViewModel = NULL);
#endif

#ifndef CLIENT_DLL
	float m_flNextCost;
#endif
	CNetworkVar(float, m_flEndTime);
	//CNetworkVar(float, m_flRevolveTime);
	//CNetworkVar(float, m_flLastShot);
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordMinigun, DT_OverlordMinigun )

BEGIN_NETWORK_TABLE( COverlordMinigun, DT_OverlordMinigun )
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flEndTime)),
#else
	RecvPropFloat(RECVINFO(m_flEndTime)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( COverlordMinigun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_minigun, COverlordMinigun );
PRECACHE_WEAPON_REGISTER(weapon_minigun);


acttable_t	COverlordMinigun::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SMG1,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SMG1,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SMG1,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SMG1,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SMG1,					false },
};


IMPLEMENT_ACTTABLE(COverlordMinigun);

COverlordMinigun::COverlordMinigun()
{
	m_flEndTime = 0.0f;
	m_flNextPrimaryAttack = 0.0f;
#ifndef CLIENT_DLL
	m_flNextCost = 0.0f;
#else
	m_Beam = NULL;
#endif
}

COverlordMinigun::~COverlordMinigun()
{
#ifdef CLIENT_DLL
	if(m_Beam)
	{
		delete m_Beam;
		m_Beam = NULL;
	}
#endif
	if(m_flEndTime != 0.0f)
	{
#ifdef CLIENT_DLL
		DestroyBeam();
#endif
#ifndef CLIENT_DLL
		if(GetOwner())
			GetOwner()->StopSound(REVOLVE_SOUND);
#endif
	}

}

void COverlordMinigun::Precache()
{
	BaseClass::Precache();
#ifdef CLIENT_DLL
	PrecacheModel(BEAM_EFFECT);
#else
	PrecacheScriptSound(REVOLVE_SOUND);
#endif
}


void COverlordMinigun::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
#ifndef CLIENT_DLL
	if(m_flNextCost != 0.0f && m_flNextCost <= gpGlobals->curtime)
	{
		GET_OVERLORD_DATA->HandlePowerEvent(EVENT_WEAPONUSED, eo_minigun_cost.GetInt());
		m_flNextCost = gpGlobals->curtime + 1.0f;
	}
#endif

	if(m_flEndTime != 0.0f && m_flEndTime <= gpGlobals->curtime)
	{
#ifdef CLIENT_DLL
		DestroyBeam();
#else
		CBasePlayer * pOwner = ToBasePlayer(GetOwner());

		if(pOwner)
		{
			DevMsg("Stop sound\n");
			pOwner->StopSound(REVOLVE_SOUND);
			pOwner->RemoveSlowdown(this);
		}
		m_flEndTime = 0.0f;
		m_flNextCost = 0.0f;
#endif
	}


}



void COverlordMinigun::PrimaryAttack()
{
#ifndef CLIENT_DLL
	if(m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return;

	if(GET_OVERLORD_DATA->GetPower() < eo_minigun_cost.GetInt())
	{
		return;
	}

	if(m_flEndTime == 0.0f)
	{
		GET_OVERLORD_DATA->HandlePowerEvent(EVENT_WEAPONUSED, eo_minigun_cost.GetInt());
		pPlayer->AddSlowdown(eo_minigun_slowdown.GetInt(), -1.0f, this, true);
		m_flNextCost = gpGlobals->curtime + 1.0f;
		pPlayer->EmitSound(REVOLVE_SOUND);
	}

	Vector start, forw;
	AngleVectors(GetAbsAngles(), &forw);

	Vector end = start + forw * MAX_TRACE_LENGTH;
	trace_t tr;
	UTIL_TraceHull(pPlayer->RealEyePosition(), end, -BEAM_HULL, BEAM_HULL, MASK_SHOT_HULL, 
		pPlayer, COLLISION_GROUP_NONE, &tr);
	
	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
		ToBasePlayer(tr.m_pEnt)->OnTakeDamage(CTakeDamageInfo(pPlayer, pPlayer, eo_minigun_damage.GetFloat(), DMG_PLASMA));
	}

	m_flEndTime = gpGlobals->curtime + eo_minigun_fire_time.GetFloat();

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
#endif
}

#ifdef CLIENT_DLL
int COverlordMinigun::DrawModel(int flags)
{
	if(m_flEndTime != 0.0f)
	{
		DrawBeam();
	}
	else if(m_Beam)
	{
		DestroyBeam();
	}

	return BaseClass::DrawModel(flags);
}

void COverlordMinigun::ViewModelDrawn(C_BaseViewModel *pViewModel)
{
	if(m_flEndTime != 0.0f)
	{
		DrawBeam(pViewModel);
	}

	BaseClass::ViewModelDrawn(pViewModel);
}

void COverlordMinigun::CreateBeam()
{
	if(m_Beam)
	{
		delete m_Beam;
		m_Beam = NULL;
	}

	m_Beam = CBeam::BeamCreate(BEAM_EFFECT, 1.0f);
}

void COverlordMinigun::DestroyBeam()
{
	if(m_Beam)
	{
		delete m_Beam;
		m_Beam = NULL;
	}
}

extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

void COverlordMinigun::DrawBeam(C_BaseViewModel * pViewModel)
{
	if(!GetOwner())
		return;

	if(!m_Beam)
	{
		CreateBeam();
	}

	if(!m_Beam)
	{
		Warning("Couldn't create the beam for the minigun!\n");
		return;
	}

	Vector start, end, forw;
	int attach = -1;
	QAngle angle;

	if(pViewModel)
	{
		attach = pViewModel->LookupAttachment("muzzle");
		pViewModel->GetAttachment(attach, start, angle);
		start += -forw * 16.0f;
	}
	else
	{
		attach = LookupAttachment("muzzle");
		GetAttachment(attach, start, angle);
	}

	AngleVectors(GetOwner()->GetAbsAngles(), &forw);

	m_Beam->SetStartPos(start);
	m_Beam->SetStartAttachment(attach);
	


	end = start + forw * MAX_TRACE_LENGTH;

	trace_t tr;
	UTIL_TraceHull(start, end, -BEAM_HULL, BEAM_HULL, MASK_SHOT_HULL, GetOwner(), COLLISION_GROUP_NONE, &tr);
	
	m_Beam->SetEndPos(tr.endpos);

	m_Beam->SetWidth(4);
	m_Beam->SetEndWidth(6);
	m_Beam->SetBrightness(210);
	m_Beam->SetColor( 255, 255, 255 );
	m_Beam->SetNoise(0.1f);
	m_Beam->SetScrollRate(150.0f);

	m_Beam->RelinkBeam();
	m_Beam->TurnOff();
	
	

	/*if(!m_Beam)
	{
		Warning("No minigun beam texture!\n");
		return;
	}
	Vector start, end, forw;
	

	if(pViewModel)
	{
		pViewModel->GetAttachment(pViewModel->LookupAttachment("muzzle"), start);
	}
	else
	{
		GetAttachment(LookupAttachment("muzzle"), start);
	}
	

	AngleVectors(GetOwner()->EyeAngles(), &forw);
	end = start + forw * MAX_TRACE_LENGTH;

	trace_t tr;
	UTIL_TraceLine(start, end, MASK_SHOT_HULL, GetOwner(), COLLISION_GROUP_NONE, &tr);

	color32 color;
	color.r = 255; color.b = 255; color.g = 255; color.a = 255;

	FXLineData_t data;
	data.m_flDieTime = 0.0000000000000000000001f;
	data.m_vecStart = start;
	data.m_vecEnd = tr.endpos;
	data.m_pMaterial = m_Beam;
	data.m_Color = color;
	data.m_flStartScale = 1;
	data.m_flEndScale = 1;
	data.m_flStartAlpha = 255;
	data.m_flEndAlpha = 255;

	data.m_vecStartVelocity = vec3_origin;
	data.m_vecEndVelocity = vec3_origin;

	FX_AddLine(data);*/
}
#endif
const char * COverlordMinigun::GetCostLabel() const
{
	static char cost[8];

	Q_snprintf(cost, ARRAYSIZE(cost), "%i/per second", eo_minigun_cost.GetInt());

	return cost;
}


bool COverlordMinigun::CanHolster()
{
	if(m_flEndTime != 0.0f)
		return false;

	return BaseClass::CanHolster();
}