//==============Overlord mod=====================
//	Overlord data, previously present in
//	hl2mp_gamerules.h
//===============================================

#include "cbase.h"
#include "overlord_data.h"
#include "overlord_dynamictraps.h"
#include "overlord_powerups_shared.h"

#ifndef CLIENT_DLL
#include "overlord_area.h"
//#include "overlord_camera.h"
#include "overlord_console.h"
#include "hl2mp_player.h"
#include "team.h"
//#include "overlord_dynamictraps.h"
#include "overlord_spawner.h"
//#include "particle_parse.h"
#else
#include "c_team.h"
#include "overlord_consolemanager.h"
#include "overlord_hintsystem.h"
#endif
#include "hl2mp_gamerules.h"

#include "tier0/vprof.h"


#ifdef CLIENT_DLL
#define CTeam C_Team
#endif

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);
extern ConVar mp_restartgame;

ConVar eo_spawn_multiplier("eo_spawn_multiplier", "-1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Overrides the map's spawns multiplier if changed.");
ConVar eo_static_spawns("eo_static_spawns", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "If set to non-zero it overrides number of spawns to this amount at the beginning of the round"); 

//ConVar eo_default_power("eo_default_power", "120", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_power_per_rebel("eo_power_per_rebel", "3.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Power given by every rebel.");
ConVar eo_base_power_gain("eo_base_power_gain", "0", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_smartpowergain("eo_smartpowergain", "1", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_powergaintime("eo_powergaintime", "2.00", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_smartpowerdelay("eo_smartpowerdelay", "2.0", FCVAR_REPLICATED | FCVAR_NOTIFY);

ConVar eo_selection_type("eo_selection_type", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, 
						 "0 - percentage-based, random roll, 1 - top player becomes the Overlordess");
//ConVar eo_powerrecharge("eo_powerrecharge", "5", FCVAR_REPLICATED | FCVAR_NOTIFY);
ConVar eo_pause_limit("eo_pause_limit", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pause_delay("eo_pause_delay", "3.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pause_length("eo_pause_length", "45", FCVAR_REPLICATED | FCVAR_CHEAT);

#define SMART_PER_HUNDRED eo_smartpowerdelay.GetFloat()

//#define DEFAULT_POWER eo_default_power.GetInt()

// Power consts
#define RECHARGE_SPEED eo_powergaintime.GetFloat()
//#define RECHARGE_POWER eo_powerrecharge.GetInt()

#define OVERLORD_MOVE_TIME eo_overlord_afk_time.GetFloat()

#define SEARCH_RADIUS eo_overlord_afk_radius.GetFloat()

#define STASIS_PARTICLE "GlobalStasis"


ConVar eo_warmode_cool("eo_war_mode_cool_down", "30", FCVAR_REPLICATED | FCVAR_NOTIFY, "How long should it take (after last damage taken by the Overlordess) for the warmode to restart.");
ConVar eo_overlord_afk_time("eo_overlord_afk_time", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "How long the Overlordess may remain idle before the round restarts (use 0 to disable).");
ConVar eo_overlord_afk_radius("eo_overlord_afk_radius", "15", FCVAR_REPLICATED | FCVAR_NOTIFY);
//ConVar eo_power_maximum("eo_power_maximum", "500", FCVAR_REPLICATED | FCVAR_NOTIFY, "Maximum amount of power the Overlordess can have");
#ifdef OVER_PLAYTEST
ConVar eo_playtest_nextover("eo_playtest_nextover", "-1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Override the next Overlord with the player with this ID. Type 'status' for IDs");
#endif // OVER_PLAYTEST


ConVar eo_ferocity_max("eo_ferocity_max", "100", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_ferocity_gain("eo_ferocity_gain", "20", FCVAR_REPLICATED | FCVAR_CHEAT);

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordData, DT_OverlordData);

BEGIN_NETWORK_TABLE(COverlordData, DT_OverlordData)

	#ifndef CLIENT_DLL
		SendPropBool( SENDINFO( m_bInVehicle ) ),
		SendPropInt( SENDINFO( m_iPower ), -1, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( m_iMaxPower ), -1, SPROP_UNSIGNED ),
		//SendPropArray(SendPropStringT( SENDINFO_ARRAY( m_iszButtons )), m_iszButtons ),
		SendPropBool( SENDINFO( m_bInWarMode ) ),
		SendPropArray	( SendPropEHandle( SENDINFO_ARRAY( m_Traps ) ), m_Traps ),

		// Sending camera (hooray, no more passing some stupid vectors and angles!!)
		SendPropEHandle(SENDINFO(m_pCamera)),

		SendPropEHandle(SENDINFO(m_pOverlord)),
		SendPropFloat(SENDINFO(m_Spawns)),
		SendPropFloat(SENDINFO(m_InitialSpawns)),
		SendPropInt(SENDINFO(m_iHealthCap)),
		SendPropBool(SENDINFO(m_bCounter)),
		SendPropBool(SENDINFO(m_bInstaSpawn)),

		SendPropFloat(SENDINFO(m_flLastResourceUpdate)),

		SendPropFloat(SENDINFO(m_flSecondsPerHealth)),
		SendPropFloat(SENDINFO(m_flSpawnMultiplier)),
		SendPropInt(SENDINFO(m_iMinimumHealth)),
		SendPropInt(SENDINFO(m_iHealthCap)),
		SendPropInt(SENDINFO(m_iMaxTraps)),

		SendPropBool(SENDINFO(m_bWallSightDisabled)),
		SendPropFloat(SENDINFO(m_flWallSightDisabledEnd)),

		SendPropBool(SENDINFO(m_bOverlordMarked)),
		SendPropFloat(SENDINFO(m_flOverlordMarkedEnd)),
		SendPropInt(SENDINFO(m_PauseCount)),

		SendPropBool(SENDINFO(m_bTutorial)),
		SendPropInt( SENDINFO( m_BuildMode ), 4, SPROP_UNSIGNED ),
		SendPropInt( SENDINFO( m_Ferocity ), 8, SPROP_UNSIGNED ),
	#else
		RecvPropBool( RECVINFO( m_bInVehicle ) ),
		RecvPropInt( RECVINFO( m_iPower ) ),
		RecvPropInt( RECVINFO( m_iMaxPower ) ),
		//RecvPropArray(RecvPropString( RECVINFO( m_iszButtons[0]) ), m_iszButtons),
		RecvPropBool( RECVINFO( m_bInWarMode ) ),
		RecvPropArray	( RecvPropEHandle( RECVINFO( m_Traps[0] ) ), m_Traps ),

		// Receiving camera (hooray, no more passing some stupid vectors and angles!!)
		RecvPropEHandle(RECVINFO(m_pCamera)),
		RecvPropEHandle(RECVINFO(m_pOverlord)),
		RecvPropFloat(RECVINFO(m_Spawns)),
		RecvPropFloat(RECVINFO(m_InitialSpawns)),
		RecvPropInt(RECVINFO(m_iHealthCap)),
		RecvPropBool(RECVINFO(m_bCounter)),
		RecvPropBool(RECVINFO(m_bInstaSpawn)),

		RecvPropFloat(RECVINFO(m_flLastResourceUpdate)),
		
		RecvPropFloat(RECVINFO(m_flSecondsPerHealth)),
		RecvPropFloat(RECVINFO(m_flSpawnMultiplier)),
		RecvPropInt(RECVINFO(m_iMinimumHealth)),
		RecvPropInt(RECVINFO(m_iHealthCap)),
		RecvPropInt(RECVINFO(m_iMaxTraps)),

		RecvPropBool(RECVINFO(m_bWallSightDisabled)),
		RecvPropFloat(RECVINFO(m_flWallSightDisabledEnd)),

		RecvPropBool(RECVINFO(m_bOverlordMarked)),
		RecvPropFloat(RECVINFO(m_flOverlordMarkedEnd)),
		RecvPropInt(RECVINFO(m_PauseCount)),

		RecvPropBool(RECVINFO(m_bTutorial)),
		RecvPropInt(RECVINFO(m_BuildMode)),
		RecvPropInt(RECVINFO(m_Ferocity)),
	#endif

END_NETWORK_TABLE()

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordData)

	DEFINE_KEYFIELD(m_flSecondsPerHealth, FIELD_FLOAT, "SecondsPerHealth"),
	DEFINE_KEYFIELD(m_iMinimumHealth, FIELD_INTEGER, "StartHealth"),
	DEFINE_KEYFIELD(m_iHealthCap, FIELD_INTEGER, "HealthCap"),
	DEFINE_KEYFIELD(m_iMaxTraps, FIELD_INTEGER, "MaxTraps"),
	DEFINE_KEYFIELD(m_flSpawnMultiplier, FIELD_FLOAT, "SpawnsMultiplier"),
	DEFINE_KEYFIELD(m_iMaxPower, FIELD_INTEGER, "MaxPower"),
	DEFINE_KEYFIELD(m_iPower, FIELD_INTEGER, "StartingPower"),

	DEFINE_INPUTFUNC(FIELD_VOID, "StartHealthCount", InputStartCount ),
	DEFINE_INPUTFUNC(FIELD_VOID, "StopHealthCount", InputStopCount ),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "AddPower", InputAddPower),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "RemovePower", InputRemovePower),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableInstantSpawn", InputStartInstaSpawn),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableInstantSpawn", InputStopInstaSpawn),
	DEFINE_INPUTFUNC(FIELD_VOID, "RespawnAllPlayers", InputRespawnAllPlayers),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableWarMode", InputEnableWarMode),

END_DATADESC()
#endif

BEGIN_PREDICTION_DATA(COverlordData)

END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(overlord_data_proxy, COverlordData);

COverlordData::COverlordData()
{
	EraseData();
#ifdef CLIENT_DLL
	ListenForGameEvent("trap_decayed");
	ListenForGameEvent("trap_dormant");
#endif
}

COverlordData::~COverlordData()
{
#ifdef CLIENT_DLL
	g_HintManager.Purge();
#endif
}

void COverlordData::Spawn()
{
	BaseClass::Spawn();

	Precache();

	RestartData();

	for(int i = 0; i < g_PowerupsList.Count(); i++)
	{
		COverlordPowerup * pPowerup = g_PowerupsList[i];

		if(!pPowerup)
			continue;

		pPowerup->Init();
	}
#ifndef CLIENT_DLL
	SetCounterEnabled(HasSpawnFlags(SF_COUNT_ON_START) == true);

	// Make sure we call this here
	// to update the GET_OVERLORD_DATA thing
	GET_OVERLORD_DATA;

	SetThink(&COverlordData::Think);
	SetNextThink(gpGlobals->curtime + 0.05f);
#endif

}

void COverlordData::Activate()
{
	BaseClass::Activate();

#ifndef CLIENT_DLL
	// Select spawns again
	if(!eo_static_spawns.GetBool())
	{
		// Crude way of getting the amount of players, minus the Ov
		int count = -1;
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			if(!UTIL_PlayerByIndex(i))
				continue;

			count++;
		}

		if(count < 0)
			count = 0;

		if(eo_spawn_multiplier.GetFloat() >= 0.0f)
			SetSpawnsAmount((float)count * eo_spawn_multiplier.GetFloat());
		else
			SetSpawnsAmount((float)count * m_flSpawnMultiplier);
	}
	else if(eo_static_spawns.GetBool())
		SetSpawnsAmount(eo_static_spawns.GetInt());
	else
		SetSpawnsAmount(20);

	m_InitialSpawns = GetSpawnsAmount();
#endif
}
// Initial data

void COverlordData::RestartData()
{
	DisableWarMode();

#ifndef CLIENT_DLL
	// Disable any pauses
	if(IsPaused())
	{
		Pause();
	}

	// This way we are not left with the camera menu stuck
	if(GetCamera())
	{
		SwitchFromCamera();
	}
#endif
	EraseData();

#ifndef CLIENT_DLL
	m_OverlordThrow = random->RandomFloat(0.0f, 100.0f);
#endif
}

void COverlordData::Precache()
{

	// Currently the only way to precache all the traps and allow
	// client-side spawning
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		COverlordTrap * pTrap = static_cast<COverlordTrap*>(CreateEntityByName(COverlordTrap::GetRecord(i).entName));
		if(!pTrap)
			continue;
		pTrap->Precache();
#ifndef CLIENT_DLL
		UTIL_Remove(pTrap);
#else
		pTrap->Remove();
#endif
	}

	//PrecacheParticleSystem(STASIS_PARTICLE);

}

// Be careful! Client runs this function after a small delay and may end up changing a variable
// which is not then networked. 
void COverlordData::EraseData(void)
{
#ifndef CLIENT_DLL
	SetCamera(NULL);
	UpdateOverlord(NULL);

	m_flLastResourceUpdate = 0.0f;

	m_bInVehicle = false;
	m_bInWarMode = false;

	m_Spawns = 0.0f;

	m_bCounter = false;
	m_bInstaSpawn = false;

	m_Console = NULL;

	m_bWallSightDisabled = false;
	m_flWallSightDisabledEnd = 0.0f;

	m_bOverlordMarked = false;
	m_flOverlordMarkedEnd = 0.0f;

	m_OverlordThrow = 0.0f;

	m_NextOverlord = NULL;
	m_PauseCount = 0;
	m_Pause = 0.0f;
	m_bTutorial = false;
	m_BuildMode = BL_NONE;
	m_Ferocity = 0;
#endif
		
#ifdef CLIENT_DLL
	// Erase hint system data too
	g_HintManager.Purge();
#endif
}

#ifndef CLIENT_DLL
void COverlordData::HandlePowerEvent(powerevent_t event)
{
	// TODO: Clean this function, make it more based on some external table etc.
	if(event.iEvent == EVENT_UPDATE)
	{
		if(gpGlobals->curtime >= m_flLastResourceUpdate)
		{
			AddPower(GetCurrentPowerGain());

			m_flLastResourceUpdate = gpGlobals->curtime + GetPowerUpdateTime();
		}

	}
	else if(event.iEvent == EVENT_PLAYERSPAWNED)
	{
		//AddPower(10);
	}
	else if(event.iEvent == EVENT_PLAYERKILLED)
	{
		AddPower(event.power);
	}
	else if(event.iEvent == EVENT_MODULE)
	{
		SubtractPower(event.power);
	}
	else if(event.iEvent == EVENT_DYNAMICTRAP)
	{
		SubtractPower(event.power);
	}
	else if(event.iEvent == EVENT_WEAPONUSED)
	{
		SubtractPower(event.power);
	}
	else if(event.iEvent == EVENT_CUSTOM)
	{
		AddPower(event.power);
	}
	else if(event.iEvent == EVENT_UNREGISTERED)
	{
		AddPower(event.power);
	}


}

void COverlordData::SetPower(const int power)
{
	if(power < 0)
		m_iPower = 0;
	else if(power >= GetMaxPower())
		m_iPower = GetMaxPower();
	else
		m_iPower = power;


	if(!GetOverlord())
		return;

	CSingleUserRecipientFilter user( GetOverlord() );
	user.MakeReliable();
	UserMessageBegin( user, "ResChanged");
		WRITE_SHORT(m_iPower.Get());
	MessageEnd();

}

void COverlordData::SwitchToCamera(COverlordCamera * cam)
{
	if(!cam)
	{
		Warning("Overlord's camera does not exist!\n");
		return;
	}

	CBasePlayer * pPlayer = GetOverlord();

	if(!pPlayer)
		return;

	if(!cam->IsEnabled())
	{
		IGameEvent * event = gameeventmanager->CreateEvent("eo_customhint");

		if(event)
		{
			event->SetInt("userid", pPlayer->GetUserID());
			event->SetString("hint", "The camera cannot be accessed!");

			gameeventmanager->FireEvent(event);
		}
		return;
	}

	if(GetCamera())
	{
		SwitchFromCamera();
	}
	else
	{
		if(pPlayer->FlashlightIsOn())
			pPlayer->FlashlightTurnOff();
	}

	//pData->SetCamera(cam);

	pPlayer->AddFlag(FL_ATCONTROLS);

	pPlayer->SetDrawLocalModel(true);



	if(pPlayer->m_Local.m_bDrawViewmodel)
		pPlayer->ShowViewModel(false);
	
	pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CAMERA;

	if(cam->ShouldShowCrosshair())
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	else
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;

	cam->FireOnEnter();

	SetPosition(cam);
	SetInVehicle(true);

	IGameEvent * event = gameeventmanager->CreateEvent("camera_entered");

	if(event)
	{
		event->SetInt("entindex", cam->entindex());
		gameeventmanager->FireEvent(event);
	}

};

void COverlordData::SwitchFromCamera(void)
{
	CBasePlayer * pPlayer = GetOverlord();

	if(!pPlayer)
		return;

	COverlordCamera * cam = GetCamera();

	if(!cam)
	{
		Warning("Overlord's camera does not exist!\n");
		return;
	}

	//pData->SetCamera(cam);
	pPlayer->RemoveFlag(FL_ATCONTROLS);
	
	cam->ResetTurret();

	pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CAMERA;
	pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;

	if(!pPlayer->m_Local.m_bDrawViewmodel)
		pPlayer->ShowViewModel(true);

	cam->FireOnExit();

	SetPosition(NULL);
	SetInVehicle(false);

	IGameEvent * event = gameeventmanager->CreateEvent("camera_exited");

	if(event)
	{
		event->SetInt("entindex", cam->entindex());
		gameeventmanager->FireEvent(event);
	}

	if(m_Console)
		m_Console->UpdateCamera(cam);

	pPlayer->SetDrawLocalModel(false);

}
#endif

// Pseudo-think, it is called by HL2MPRules
void COverlordData::Think()
{
#ifndef CLIENT_DLL
	VPROF_BUDGET( "COverlordData::Think", VPROF_BUDGETGROUP_GAME );

	SetNextThink(gpGlobals->curtime + 0.025f);

	// Disable the warmode
	/*if(gpGlobals->curtime >= m_flWarModeRestart)
	{
		DisableWarMode();
	}*/

	HandlePowerEvent(EVENT_UPDATE);

	/*if(IsJailBreak() && GetJailBreakTime() <= 0.0f)
		InitJailBreak(false);
	else if(IsJailBreak())*/

	// Init counter
	if(GetOverlord() && m_bCounter)
	{
		float seconds = gpGlobals->curtime - m_flLastCount;

		if(seconds >= m_flSecondsPerHealth)
		{
			int health = GetOverlord()->GetMaxHealth();
			
			int newhealth = health + int(seconds/m_flSecondsPerHealth);

			if(newhealth > m_iHealthCap)
				newhealth = m_iHealthCap;

			GetOverlord()->SetMaxHealth(newhealth);
			GetOverlord()->SetHealth(newhealth);

			m_flLastCount = gpGlobals->curtime;
		}
	}

	if(m_bWallSightDisabled && (m_flWallSightDisabledEnd <= gpGlobals->curtime))
	{
		m_bWallSightDisabled = false;
		m_flWallSightDisabledEnd = 0.0f;
	}

	if(m_bOverlordMarked && (m_flOverlordMarkedEnd <= gpGlobals->curtime))
	{
		m_bOverlordMarked = false;
		m_flOverlordMarkedEnd = 0.0f;
	}

	// Pause the game

	if(m_Pause != 0.0f && m_Pause <= gpGlobals->curtime)
	{
		if(!IsPaused())
		{
			for(int i = 0; i < MAX_PLAYERS; i++)
			{
				CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

				if(!pPlayer)
					continue;

				pPlayer->AddFrozenRef();
			}
			m_Pause = 0.0f;
			UTIL_ClientPrintAll(HUD_PRINTCENTER, "Paused!");

			ConVar * timescale = cvar->FindVar("host_timescale");

			if(timescale)
				timescale->SetValue("0.0001");

			m_Pause = gpGlobals->curtime + eo_pause_length.GetFloat();
		}
		else
		{
			Pause();
		}
	}

	// Go over powerups
	for(int i = 0; i < g_PowerupsList.Count(); i++)
	{
		COverlordPowerup * pPowerup = g_PowerupsList[i];

		if(!pPowerup || !pPowerup->IsActive())
			continue;

		if(pPowerup->ShouldEnd())
		{
			pPowerup->StopPowerup();
		}
		else
			pPowerup->Think();
	}

#endif

}

void COverlordData::FireGameEvent( IGameEvent *event )
{
	if(event)
	{
#ifdef CLIENT_DLL
		if(!Q_stricmp(event->GetName(), "trap_decayed"))
		{
			int entindex = event->GetInt("entindex", 0);
			if(entindex)
			{
				C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					C_OverlordTrap * pTrap = C_OverlordTrap::EntityToTrap(pEnt);

					if(pTrap)
					{
						pTrap->DecayTrap();
					}
					else
					{
						Warning("Trap_decayed: Received an entity but not a trap!\n");
					}
				}
			}
		}
		else if(!Q_stricmp(event->GetName(), "trap_dormant"))
		{
			int entindex = event->GetInt("entindex", 0);
			if(entindex)
			{
				C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					C_OverlordTrap * pTrap = C_OverlordTrap::EntityToTrap(pEnt);

					if(pTrap)
					{
						pTrap->SetTrapDormant(event->GetBool("dormant"));
					}
					else
					{
						Warning("Trap_dormant: received an entity but not a trap\n");
					}
				}
				else
				{
					//Warning("Trap_dormant received, but no entity can be found\n");
				}
			}
			else
			{
				Warning("Trap_dormant received, but no entindex can be found\n");
			}
		}
#endif
	}
}

int COverlordData::GetTrapsAmount()
{
	int amount = 0;
	for(int i = 0; i < GetMaxTraps(); i++)
	{
		if(GetTrap(i))
			amount++;
	}

	return amount;
}

float COverlordData::GetFerocityPercantage() const
{
	return ((float)m_Ferocity.Get())/eo_ferocity_max.GetFloat();
}
int COverlordData::CalculateFerocity(float minutesSinceDeath)
{
	const float e = 2.718281828459045f;
	return 1.0f/pow(e, minutesSinceDeath) * eo_ferocity_gain.GetFloat();
}

#ifndef CLIENT_DLL

// Returns false if a trap hasn't been added
bool COverlordData::AddTrap(COverlordTrap * pTrap)
{
	if(!pTrap)
		return false;

	for(int i = 0; i < GetMaxTraps(); i++)
	{
		if(m_Traps[i])
			continue;

		m_Traps.Set(i, pTrap);
		return true;
	}

	return false;
}
void COverlordData::UpdateLabels()
{
	//CSingleUserRecipientFilter user( GetOverlord() );
	//user.MakeReliable();
	//UserMessageBegin( user, "UpdateCamera");
	//MessageEnd();
}

void COverlordData::DispatchNewOverlord(CBasePlayer * pOverlord)
{
	if(!pOverlord)
		return;

	pOverlord->ChangeTeam(TEAM_OVERLORD);
	pOverlord->Spawn();
	//m_pOverlord = static_cast<CBaseEntity*>(pOverlord);
}


void COverlordData::UpdateOverlord(CBasePlayer * pOverlord)
{ 
	m_pOverlord = pOverlord; 
}
#else
void COverlordData::PrintRoundStatistics()
{
}
#endif

COverlordTrap * COverlordData::GetTrap(int num)
{
	return static_cast<COverlordTrap*>(m_Traps.GetForModify(num).Get());
}
// Called only when the Overlord is killed and round should
// restart, triggers dead-overlord scene
void COverlordData::OverlordLoses()
{
	DisableWarMode();
}

void COverlordData::OverlordWins()
{
	DisableWarMode();
}

// Ok, in fact it is just a badly named function.
// This function not only enables war mode, but also
// restarts the timer every time the Overlord takes damage
void COverlordData::EnableWarMode()
{
	if(!IsInWarMode())
	{
		m_bInWarMode = true;

#ifndef CLIENT_DLL
		//SetConsolesEnabled(false);
		if(GetCamera())
		{
			SwitchFromCamera();
		}
#endif
	}
#ifndef CLIENT_DLL
	//m_flWarModeRestart = gpGlobals->curtime + eo_warmode_cool.GetInt(); 
#endif
}

void COverlordData::DisableWarMode()
{
	if(IsInWarMode())
	{
		m_bInWarMode = false;
#ifndef CLIENT_DLL
		//SetConsolesEnabled(true);
		//m_flWarModeRestart = 0;
#endif
	}
}

float COverlordData::GetPowerUpdateTime() const
{
	return GetPowerUpdateTime(GetPower());
}

float COverlordData::GetPowerUpdateTime(int currentPower) const
{
	float time = RECHARGE_SPEED;
	if(eo_smartpowergain.GetBool())
	{
		// Get amount of 100s in our power, elevate it and then multiply it
		int hundreds = ((currentPower/100 - 1) >= 0 ) ? (currentPower/100 - 1) : 0;

		time += hundreds * SMART_PER_HUNDRED;
	}
	
	return time;
}

int COverlordData::GetCurrentPowerGain() const
{
	return eo_base_power_gain.GetInt() + HL2MPRules()->GetInitialPlayers() * eo_power_per_rebel.GetFloat();
}

bool COverlordData::IsPaused() const
{
	ConVar * timescale = cvar->FindVar("host_timescale");

	if(!timescale)
		return false;

	if(timescale->GetFloat() < 1.0f)
		return true;

	return false;
}
#ifndef CLIENT_DLL
void COverlordData::Pause()
{
	if(m_PauseCount > eo_pause_limit.GetInt() && !IsPaused())
		return;

	ConVar * timescale = cvar->FindVar("host_timescale");

	if(!timescale)
		return;

	if(IsPaused())
	{
		for(int i = 0; i < MAX_PLAYERS; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

			if(!pPlayer)
				continue;

			pPlayer->RemoveFrozenRef();
		}

		timescale->SetValue(1.0f);
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "Unpaused!");
	}
	else
	{
		m_Pause = gpGlobals->curtime + eo_pause_delay.GetFloat();
		m_PauseCount++;
		char print[64];
		Q_snprintf(print, ARRAYSIZE(print), "Pause in effect in %i seconds!", eo_pause_delay.GetInt());
		UTIL_ClientPrintAll(HUD_PRINTCENTER, print);
	}
}

void COverlordData::ResetPowerups()
{
	for(int i = 0; i < g_PowerupsList.Count(); i++)
	{
		COverlordPowerup * pPowerup = g_PowerupsList[i];

		if(!pPowerup || !pPowerup->IsActive())
			continue;

		pPowerup->StopPowerup();
	}
}

void COverlordData::SetConsolesEnabled(bool bEnabled) const
{
	int i = 0;
	CBaseEntity * pCur = gEntList.FirstEnt();
	COverlordConsole * pConsole = NULL;
	while(pCur)
	{
			
		// Breaks the loop if it has gone too far,
		// protects from infinite looping
		if(i > gEntList.NumberOfEntities())
			break;

		i++;

		if(pCur->ClassMatches("func_overconsole"))
		{
			pConsole = static_cast<COverlordConsole*>(pCur);

			if(pConsole)
				pConsole->SetEnabled(bEnabled);
		}

		pCur = gEntList.NextEnt(pCur);

		pConsole = NULL;

	}
}


bool COverlordData::IsOverlordPlaying() const
{
	// Needed static variables in order to
	// preserve Overlord's status
	static bool bMoved = false;
	static CHandle<CBasePlayer> lastOverlord = NULL;
	static Vector vStartPos = vec3_origin;

	if(OVERLORD_MOVE_TIME <= 0.0f)
		return true;

	if(!HL2MPRules())
		return true;

	if(!GetOverlord())
		return true;

	// So we don't end up with overlord being notafk after he moved while restarting
	if(HL2MPRules()->IsRestarting())
		return bMoved;

	// Restarts vectors
	if(lastOverlord != GetOverlord() || !lastOverlord)
	{
		lastOverlord = GetOverlord();
		vStartPos = vec3_origin;
		bMoved = false;
	}

	// Initing the vector, so it never is uninitialized
	if(vStartPos == vec3_origin)
		vStartPos = GetOverlord()->GetAbsOrigin();

	// We return true after checking whether we should restart all vars, as 
	// the bool could have been switched to "false"
	if(bMoved)
		return true;

	// How long it is after the round has been restarted
	float gametime = HL2MPRules()->GetRoundTime();

	// This the main function, checks whether the
	// overlord has moved and whether it should return false
	if(gametime <= OVERLORD_MOVE_TIME && lastOverlord.Get())
	{
		/*CBaseEntity * list[1024];
		int count;
		count = UTIL_EntitiesInSphere(list, ARRAYSIZE(list), vStartPos, SEARCH_RADIUS, MASK_ALL);
		if(count > 0)
		{
			bool found = false;
			int i;
			CBasePlayer * pPlayer = NULL;
			for(i = 0; i < count; i++)
			{
				pPlayer = ToBasePlayer(list[i]);

				if(!pPlayer)
					continue;

				if(pPlayer->IsOverlord())
				{
					found = true;
					break;
				}
				
			}
			if(!found)
				bMoved = true;

			
		}
		else
		{
			bMoved = true;
		}

		if(bMoved)
			return true;*/

		if((GetOverlord()->GetAbsOrigin() - vStartPos).Length() > SEARCH_RADIUS)
		{
			bMoved = true;
			return true;
		}
	}
	else if(gametime > OVERLORD_MOVE_TIME && !bMoved)
	{
		return false;
	}

	return true;
}

namespace
{
	struct SelectionRecord
	{
		CHandle<CBasePlayer> m_Player;
		float m_flChance;
	};

	int SelectionCompare(const SelectionRecord * rhs, const SelectionRecord * lhs)
	{
		// Randomise our placement in the table
		int r = random->RandomInt(-1, 2);

		if(r == 0)
			r--;

		return r;
		//return (lhs->m_flChance - rhs->m_flChance);
	}
}

CBasePlayer * COverlordData::GetNextOverlord() const
{	
#ifdef OVER_PLAYTEST
	if(eo_playtest_nextover.GetInt() >= 0)
	{
		const int userid = eo_playtest_nextover.GetInt();

		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);
			if(!pPlayer)
				continue;

			if(userid == pPlayer->GetUserID())
				return pPlayer;
		}

	}
#endif
	// Overriden Ov here
	if(m_NextOverlord)
		return m_NextOverlord;

	CBasePlayer * pNewOverlord = NULL;

	if(eo_selection_type.GetInt() != 0)
	{
		int highest = 0;
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);
			if(!pPlayer)
				continue;

			if(!pPlayer->IsRebel() && (pPlayer->GetTeamNumber() != TEAM_SPECTATOR))
				continue;


			char clue[256];
			Q_snprintf(clue, ARRAYSIZE(clue), "%s has gathered %i points\n", pPlayer->GetPlayerName(), pPlayer->DamageToOverlord());
			UTIL_ClientPrintAll(HUD_PRINTCONSOLE, clue);
		

			if(highest < pPlayer->DamageToOverlord())
			{
				highest = pPlayer->DamageToOverlord();
				pNewOverlord = pPlayer;
			}
		}
	}
	else
	{
		CUtlVector<SelectionRecord> records;

		float sum = 0;
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

			if(!pPlayer || pPlayer->IsOverlord() || pPlayer->DamageToOverlord() <= 0 || !pPlayer->WantsToBeOverlord())
				continue;

			sum += pPlayer->DamageToOverlord();
		}

		
		if(sum != 0.0f)
		{
			for(int i = 1; i <= gpGlobals->maxClients; i++)
			{
				CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

				if(!pPlayer || pPlayer->IsOverlord() || pPlayer->DamageToOverlord() <= 0 || !pPlayer->WantsToBeOverlord())
					continue;

				SelectionRecord rec;
				rec.m_Player = pPlayer;
				rec.m_flChance = (float(pPlayer->DamageToOverlord()) / sum) * 100.0f;

				char print[128];
				Q_snprintf(print, ARRAYSIZE(print), "%s has acquired %f percent.", pPlayer->GetPlayerName(), rec.m_flChance);
				UTIL_ClientPrintAll(HUD_PRINTCONSOLE, print);

				records.AddToTail(rec);
			}


			char print[128];
			Q_snprintf(print, ARRAYSIZE(print), "The throw is %f", m_OverlordThrow);
			UTIL_ClientPrintAll(HUD_PRINTCONSOLE, print);

			records.Sort(SelectionCompare);

			float chance = 0.0f;
			for(int i = 0; i < records.Count(); i++)
			{
				char print[128];
				Q_snprintf(print, ARRAYSIZE(print), "%s is between %f - %f", records[i].m_Player->GetPlayerName(), 
					chance, chance + records[i].m_flChance);
				UTIL_ClientPrintAll(HUD_PRINTCONSOLE, print);

				chance += records[i].m_flChance;

				if(m_OverlordThrow <= chance)
				{
					pNewOverlord = records[i].m_Player;
					break;
				}
			}
		}
	}

	// No player selected
	if(!pNewOverlord)
	{
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		// Overlord lives, check state
		if(GetOverlord() && GetOverlord()->IsAlive())
		{
			// Overlord's afk
			if(!IsOverlordPlaying())
				return NULL;

			// Ov has resigned here;
			if(IsOverlordPlaying() && (!pRebels || pRebels->GetNumPlayers() > 0))
				return NULL;
		}

		// Overlord has won
		//if(highest <= 0 || (!pRebels || pRebels->GetNumPlayers() <= 0))
			return GetOverlord();
	}

	return pNewOverlord;
}

void COverlordData::DisableWallSight(float flLength)
{
	m_bWallSightDisabled = true;
	m_flWallSightDisabledEnd = gpGlobals->curtime + flLength;
}

void COverlordData::MarkOverlord(float flLength)
{
	m_bOverlordMarked = true;
	m_flOverlordMarkedEnd = gpGlobals->curtime + flLength;
}

void COverlordData::ChangeFerocity(int ferocity)
{
	m_Ferocity += ferocity;

	if(m_Ferocity > eo_ferocity_max.GetInt())
		m_Ferocity = eo_ferocity_max.GetInt();
}

void COverlordData::UpdateFerocity(CBasePlayer * pVictim)
{
	if(m_Ferocity >= eo_ferocity_max.GetInt())
		return;

	if(!pVictim || pVictim->IsOverlord())
		return;

	float lastDeath = 0.0f;
	for(int i = 0; i < gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || pPlayer->IsOverlord() || (pVictim == pPlayer))
			continue;

		if(lastDeath < pPlayer->GetDeathTime())
			lastDeath = pPlayer->GetDeathTime();
	}

	int ferocityGain = CalculateFerocity((gpGlobals->curtime - lastDeath)/60.0f);
	m_Ferocity = GetFerocity() + ferocityGain;

	if(m_Ferocity > eo_ferocity_max.GetInt())
		m_Ferocity = eo_ferocity_max.GetInt();
}

void COverlordData::InputStartCount( inputdata_t &inputdata )
{
	SetCounterEnabled(true);
}

void COverlordData::InputStopCount( inputdata_t &inputdata )
{
	SetCounterEnabled(false);
}

void COverlordData::InputAddPower(inputdata_t &inputdata)
{
	HandlePowerEvent(EVENT_MAP, inputdata.value.Int());
}

void COverlordData::InputRemovePower(inputdata_t &inputdata)
{
	HandlePowerEvent(EVENT_MAP, -inputdata.value.Int());
}

void COverlordData::InputStartInstaSpawn(inputdata_t &inputdata)
{
	m_bInstaSpawn = true;
}

void COverlordData::InputStopInstaSpawn(inputdata_t &inputdata)
{
	m_bInstaSpawn = false;
}

void COverlordData::InputRespawnAllPlayers(inputdata_t &inputdata)
{

}

void COverlordData::InputEnableWarMode(inputdata_t &inputdata)
{
	EnableWarMode();
	SetCounterEnabled(false);
}

#ifdef OVER_PLAYTEST
CON_COMMAND(eo_playtest_restartround, "Playtest command, restarts current round. Works like eo_giveup" )
{
	if(!UTIL_IsCommandIssuedByServerAdmin())
		return;

	mp_restartgame.SetValue(5);
}


#endif // OVER_PLAYTEST

CON_COMMAND_F(eo_give_resources, "Give specific number of resources", FCVAR_CHEAT)
{
	COverlordData & data = *GET_OVERLORD_DATA;

	data.HandlePowerEvent(EVENT_UNREGISTERED, atoi(args.Arg(1)));
}

CON_COMMAND_F(eo_give_ferocity, "Gives ferocity", FCVAR_CHEAT)
{
	COverlordData & data = *GET_OVERLORD_DATA;

	data.ChangeFerocity(atoi(args.Arg(1)));
}
CON_COMMAND_F(eo_give_spawns, "Gives x spawns", FCVAR_CHEAT)
{
	COverlordData & data = *GET_OVERLORD_DATA;

	data.SetSpawnsAmount(data.GetSpawnsAmount() + atoi(args.Arg(1)));
}

CON_COMMAND_F(eo_give_points, "Gives x points", FCVAR_CHEAT)
{
	int userid = atoi(args.Arg(1));

	CBasePlayer * pPlayer = UTIL_PlayerByUserId(userid);

	if(pPlayer)
	{
		int points = atoi(args.Arg(2));

		pPlayer->IncreaseOverlordDamage(points);
	}
}

CON_COMMAND_F(eo_give_random_points, "Gives random points to every player", FCVAR_CHEAT)
{
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(pPlayer)
		{
			int points = random->RandomInt(0, 200);

			pPlayer->IncreaseOverlordDamage(points);
		}
	}
}

#endif // CLIENT_DLL


COverlordData * GetOverlordData()
{
	static CHandle<COverlordData> sData = NULL;
#ifndef CLIENT_DLL
	if(!sData)
	{
		COverlordData * pData = static_cast<COverlordData*>(gEntList.FindEntityByClassname(NULL, "overlord_data_proxy"));

		if(!pData)
		{
			pData = static_cast<COverlordData*>(CreateEntityByName("overlord_data_proxy"));

			if(pData)
			{
				pData->Spawn();
				Warning("No proxy on the map, creating...\n");
			}
			else
			{
				Warning("Cannot create proxy...\n");
				return NULL;
			}
		}

		sData = pData;
	}
#else
	if(!sData)
	{
		for(int i = 0; i <= ClientEntityList().GetHighestEntityIndex(); i++)
		{
			C_BaseEntity * pEnt = ClientEntityList().GetBaseEntity(i);

			if(!pEnt)
				continue;

			C_OverlordData * pData = dynamic_cast<C_OverlordData*>(pEnt);

			if(pData)
			{
				sData = pData;
				break;
			}
		}
	}
#endif
	return sData;
}