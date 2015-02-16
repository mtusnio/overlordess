//==============Overlord mod=====================
//	Base class for all modules
//	
//===============================================

#include "cbase.h"
#include "overlord_basemodule.h"
#include "hl2mp_gamerules.h"
#include "overlord_data.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEFAULT_THINK 0.05f
#define DENY_SOUND "HL2Player.UseDeny"

ConVar eo_modules_neverdeactivate("eo_modules_neverdeactivate", "0", FCVAR_CHEAT | FCVAR_REPLICATED);

BEGIN_DATADESC(COverlordBaseModule)
	
	DEFINE_KEYFIELD(m_flLength, FIELD_FLOAT, "ActTime"), 
	DEFINE_KEYFIELD(m_iCost, FIELD_INTEGER, "ModuleCost"),
	DEFINE_KEYFIELD(m_flCooldown, FIELD_FLOAT, "CooldownTime"),
	DEFINE_KEYFIELD(m_ModuleName, FIELD_STRING, "ModuleName"),
	DEFINE_KEYFIELD(m_ModuleDescription, FIELD_STRING, "ModuleDescription"),
	DEFINE_KEYFIELD(m_szGlowEntityName, FIELD_STRING, "GlowEntity"),

	DEFINE_INPUTFUNC(FIELD_VOID, "InputActivate", InputActivate), 
	DEFINE_INPUTFUNC(FIELD_VOID, "InputDeactivate", InputDeactivate), 
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable), 
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable), 

	DEFINE_OUTPUT(m_OnActivated, "OnActivated"), 
	DEFINE_OUTPUT(m_OnDeactivated, "OnDeactivated"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(COverlordBaseModule, DT_OverlordBaseModule)

	SendPropFloat( SENDINFO( m_flLength ) ),
	SendPropInt( SENDINFO(m_iCost) ),
	SendPropBool(SENDINFO(m_bEnabled) ),
	SendPropBool(SENDINFO(m_bActivated) ),
	SendPropFloat(SENDINFO(m_flCooldown) ),
	SendPropFloat(SENDINFO(m_flDeactTime) ),
	SendPropStringT( SENDINFO(m_ModuleName) ),
	SendPropStringT( SENDINFO(m_ModuleDescription) ),
	SendPropEHandle( SENDINFO(m_GlowEntity)),
	SendPropInt( SENDINFO(m_spawnflags)),

END_SEND_TABLE()


COverlordBaseModule::COverlordBaseModule()
{
	SetActivated(false);
	//m_bEnabled = true;
	m_flDeactTime = 0.0f;
	m_GlowEntity = NULL;
}

COverlordBaseModule::~COverlordBaseModule()
{

}

void COverlordBaseModule::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(DENY_SOUND);

	PrecacheModel("models/props_combine/breenglobe.mdl");
}

void COverlordBaseModule::Spawn()
{
	BaseClass::Spawn();

	Precache();

	// If we do not keep it this way we will end up with traps randomly enabling and disabling themself
	if(HasSpawnFlags(SF_START_MODULE_DISABLED) == true)
		m_bEnabled = false;
	else
		m_bEnabled = true;
		
	SetSolid(SOLID_BBOX);

	AddSolidFlags( FSOLID_NOT_SOLID );

	AddSolidFlags( FSOLID_TRIGGER );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetRenderMode(kRenderTransAlpha);
	SetRenderColor(255, 127, 80, 160);
	SetModel("models/props_combine/breenglobe.mdl");

	SetThink(&COverlordBaseModule::Think);

	if(GetDuration() <= 0.0f)
	{
		SetNextThink(TICK_NEVER_THINK);
	}
	else
		SetNextThink(gpGlobals->curtime + DEFAULT_THINK);
}

void COverlordBaseModule::Activate()
{
	BaseClass::Activate();

	if(m_szGlowEntityName)
	{
		m_GlowEntity = gEntList.FindEntityByName(gEntList.FirstEnt(), m_szGlowEntityName);
	}
}

void COverlordBaseModule::Think()
{
	if(GetDuration() <= 0.0f)
		return;

	if(!eo_modules_neverdeactivate.GetBool() && ShouldDeactivate())
		Deactivate();

	SetNextThink(gpGlobals->curtime + DEFAULT_THINK);
}


void COverlordBaseModule::Deactivate()
{
	SetActivated(false);
	FireOnDeactivated();
}

void COverlordBaseModule::DenyActivation()
{
	GET_OVERLORD_DATA_ASSERT(pData);
	
	CBasePlayer * pPlayer = pData->GetOverlord();
	
	pPlayer->EmitAmbientSound(pPlayer->entindex(), pPlayer->EyePosition(), DENY_SOUND);

	// Send a hint about the reasons
	// Priorities:
	// 1. Module is activated
	// 2. Module is disabled
	// 3. Module is cooling down
	// 4. Not enough power

	bool sendhint = true;
	char hint[256];

	if(IsActivated())
	{
		Q_strncpy(hint, "This module has already been activated!", ARRAYSIZE(hint));
	}
	else if(!IsEnabled())
	{
		Q_strncpy(hint, "This module is disabled!", ARRAYSIZE(hint));
	}
	else if(IsCoolingDown())
	{
		const int time = RemainingCooldownTime();
		if(time > 1)
			Q_snprintf(hint, ARRAYSIZE(hint) , 
			"The module is cooling down, it will be ready in: %i seconds", time);
		else
			Q_snprintf(hint, ARRAYSIZE(hint) , 
			"The module is cooling down, it will be ready in: %i second", time);
	}
	else if(pData->GetPower() < GetModuleCost())
	{
		const int power = GetModuleCost() - pData->GetPower();

		Q_snprintf(hint, ARRAYSIZE(hint), 
		"The module requires %i more power", power);
	}
	else
	{
		// We do not want do send unknown hint
		sendhint = false;
		Warning("No reason found for the inability to enable a module\n");
	}

	if(sendhint)
	{
		IGameEvent * event = gameeventmanager->CreateEvent("eo_customhint");
		if(event)
		{	
			event->SetInt("userid", pPlayer->GetUserID());
			event->SetString("hint", hint);
			gameeventmanager->FireEvent(event);
		}

	}
}

bool COverlordBaseModule::CanActivate() const
{
	if(IsActivated())
		return false;

	if(!IsEnabled())
		return false;

	if(IsCoolingDown())
		return false;

	COverlordData * pData = GET_OVERLORD_DATA;

	if(pData->GetPower() < GetModuleCost())
		return false;

	return true;
}

void COverlordBaseModule::InputActivate(inputdata_t &inputData)
{
	if(!CanActivate())
	{
		DenyActivation();
		return;
	}

	GET_OVERLORD_DATA_ASSERT(pData);

	pData->HandlePowerEvent(EVENT_MODULE, m_iCost);

	SetActivated(true);

	m_flDeactTime = gpGlobals->curtime + GetDuration();

	FireOnActivated();

	// Deactivate immediately
	if(!ShouldNeverDeactivate() && GetDuration() == 0.0f)
		Deactivate();

}

void COverlordBaseModule::InputDisable(inputdata_t &inputData)
{
	//m_bEnabled = false;
	
	if(IsActivated())
		Deactivate();

	m_bEnabled = false;
}

void COverlordBaseModule::InputEnable(inputdata_t &inputData)
{
	m_bEnabled = true;
}

void COverlordBaseModule::InputDeactivate(inputdata_t &inputData)
{
	if(!IsActivated())
		return;

	Deactivate();
}


