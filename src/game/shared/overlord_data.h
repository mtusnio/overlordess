//==============Overlord mod=====================
//	Overlord data, previously present in
//	hl2mp_gamerules.h
//===============================================

#ifndef H_OVERLORDDATA
#define H_OVERLORDDATA

#ifdef CLIENT_DLL
#define COverlordData C_OverlordData
#endif

#include "overlord_camera.h"


#include "gameeventlistener.h"

class COverlordArea;

// Spawn flags
#define SF_COUNT_ON_START 1

#ifndef CLIENT_DLL
class COverlordTrap;
class COverlordConsole;
class COverlordDataProxy;
#else
class C_OverlordTrap;
class C_OverlordDataProxy;
#endif

#ifndef CLIENT_DLL
enum iPowerEvent
{
	// Event_None is just ignored by handler
	EVENT_UPDATE = 0,
	EVENT_PLAYERSPAWNED,
	EVENT_PLAYERKILLED,
	EVENT_MODULE,
	EVENT_DYNAMICTRAP,
	// Custom event, power should be negative/positive then
	EVENT_WEAPONUSED,
	// Map adds some
	EVENT_MAP,
	EVENT_CUSTOM,

	// Don't register this one, just pass the power and be done with it
	EVENT_UNREGISTERED,

	EVENT_MAXEVENTS,
};

struct powerevent_t
{
	iPowerEvent iEvent;
	int power;

	// Player killed
	CBasePlayer * m_pKilled;
};
#else

#endif
/* Originally it was supposed to contain only few variables,
   now it contains more than few. However, remember that
   it should only have data required for the Overlord team,
   round-specific functions should be in HL2MPRules. Bear in
   mind that rules often pass some functions to the data*/
class COverlordData : public CBaseEntity, public CGameEventListener
{
	// In fact, Overlord data is partly rules' interface
	// therefore they should be able to access
	// each other's variables
	friend class CHL2MPRules;

	// Access proxy too
#ifndef CLIENT_DLL
	friend class COverlordDataProxy;
#else
	friend class C_OverlordDataProxy;
#endif
public:

	DECLARE_CLASS( COverlordData, CBaseEntity );
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
	DECLARE_PREDICTABLE();


	COverlordData ();
	virtual ~COverlordData ();

	virtual void Spawn();
	virtual void Activate();

	void RestartData();
	virtual void Precache();

	virtual void Think();

#ifndef CLIENT_DLL
	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}
#endif

	bool IsInVehicle(void) const { return m_bInVehicle.Get(); };
	int  GetPower(void) const { return m_iPower; };
	int  GetMaxPower(void) const { return m_iMaxPower; };

	Vector GetVehicleVector(void) const 
	{
		return (GetCamera() ? GetCamera()->GetCameraVector() : Vector(0, 0, 0));
	};

	QAngle GetVehicleAngle(void) const 
	{ 
		return (GetCamera() ? GetCamera()->GetCameraAngles() : QAngle(0, 0, 0));
	};


	virtual void FireGameEvent( IGameEvent *event );

	// Clients can't change anything, only get
	inline CBasePlayer * GetOverlord(void) const;

	int GetHealthCap() const { return m_iHealthCap.Get(); };
	bool IsCountingHealth() const { return m_bCounter.Get(); };
	bool ShouldInstantlySpawn() const { return m_bInstaSpawn.Get(); };

	int	GetMaxTraps() { return m_iMaxTraps.Get(); };
	int GetTrapsAmount();

	bool			  IsInTutorial() const { return m_bTutorial.Get(); };
	int				  GetFerocity() const { return m_Ferocity.Get(); };
	float			  GetFerocityPercantage() const;
	int				  CalculateFerocity(float minutesSinceDeath);

#ifndef CLIENT_DLL
	COverlordConsole * GetConsole() const { return m_Console; };
	void SetConsole(COverlordConsole * console) { m_Console = console; };

	COverlordCamera * GetCamera(void) const { return m_pCamera.Get(); };
	void			  SetCamera(COverlordCamera * camera) { m_pCamera.Set(camera); };

	void  SwitchToCamera(COverlordCamera * cam);
	void  SwitchFromCamera();

	void   SetPosition(COverlordCamera * pCam) { SetCamera(pCam); }; // Old method kept for compatibility

	// Currently we have two functions - UpdateOverlord and DispatchNewOverlod
	// The first one is used for cheat commands etc., while the latter
	// is called after the round restarted
	void   UpdateOverlord(CBasePlayer * overlord);	
			
	// Creates event for us
	void   HandlePowerEvent(iPowerEvent powerevent, int power)
	{
		powerevent_t event;
		event.iEvent = powerevent;
		event.power = power;
		HandlePowerEvent(event);
	}
	// This overload can be used for events that do not require power update
	void   HandlePowerEvent(iPowerEvent powerevent) 
	{
		powerevent_t event;
		event.iEvent = powerevent;
		event.power = 0;
		HandlePowerEvent(event);
	}
	void   HandlePowerEvent(powerevent_t event);

	void   SetInVehicle(bool invehicle) { m_bInVehicle = invehicle; };

	bool			  AddTrap(COverlordTrap * pTrap);

	void			  UpdateLabels();
	void			  DispatchNewOverlord(CBasePlayer * pOverlord);
	
	void			  SetConsolesEnabled(bool bEnabled) const;

	bool			  IsOverlordPlaying() const;

	CBasePlayer *	  GetNextOverlord() const; // Returns the would-be Overlord,
											   // null if Overlord still hasn't
											   // been hurt

	void			  SetNextOverlord(CBasePlayer * pPlayer) { m_NextOverlord = pPlayer; };

	void			  DisableWallSight(float flLength);
	void			  MarkOverlord(float flLength);

	void			  SetTutorial(bool bSet) { m_bTutorial = bSet; };
	void			  SetBuildMode(Build_t bSet) { m_BuildMode = bSet; };

	void			  ChangeFerocity(int ferocity);
	void			  UpdateFerocity(CBasePlayer * pVictim);
	//==============================
	//				Inputs
	//==============================
	void InputStartCount( inputdata_t &inputdata );
	void InputStopCount( inputdata_t &inputdata );
	void InputAddPower(inputdata_t &inputdata);
	void InputRemovePower(inputdata_t &inputdata);
	void InputStartInstaSpawn(inputdata_t &inputdata);
	void InputStopInstaSpawn(inputdata_t &inputdata);
	void InputRespawnAllPlayers(inputdata_t &inputdata);
	void InputEnableWarMode(inputdata_t &inputdata);
#else
	void			   PrintRoundStatistics();
	C_OverlordCamera * GetCamera(void) const { return m_pCamera; };
	void			   SetCamera(C_OverlordCamera * pCamera) { m_pCamera = pCamera; };
#endif

#ifndef CLIENT_DLL
	COverlordTrap *   GetTrap(int num);
#else
	C_OverlordTrap *  GetTrap(int num);
#endif

	void			  OverlordLoses();
	void			  OverlordWins();

	// War mode, activated when the Overlord is attacked
	void			  EnableWarMode();
	void			  DisableWarMode();

	bool			  IsInWarMode() const { return m_bInWarMode; };

	float			  GetNextPowerUpdate() const { return m_flLastResourceUpdate; };
	float			  GetPowerUpdateTime() const;
	float			  GetPowerUpdateTime(int currentPower) const;
	int				  GetCurrentPowerGain() const;

	bool			  IsInBuildMode() const { return m_BuildMode != BL_NONE; };
	Build_t			  GetBuildMode() const { return m_BuildMode; };
#ifndef CLIENT_DLL
	void			  SetSpawnsAmount(float spawns) { m_Spawns = spawns; };
	void			  SetInitialSpawns(float spawns) { m_InitialSpawns = spawns; };
	void			  DecrementSpawns() { m_Spawns--; };

	void			  SetCounterEnabled(bool bEnabled) { m_bCounter = bEnabled; m_flLastCount = gpGlobals->curtime; };

	void		      Pause();

	void			  ResetPowerups();

	void   AddPower(const int power) 
	{ 
		SetPower(GetPower() + power); 
	};
	void   SubtractPower(const int power) 
	{ 
		SetPower(GetPower() - power); 
	};

	void   SetPower(const int power);


#endif

	float			  GetSpawnsPerPlayer() const { return m_flSpawnMultiplier; };
	float			  GetSpawnsAmount() const { return m_Spawns.Get(); };
	float			  GetInitialSpawns() const { return m_InitialSpawns.Get(); };
	int				  GetMinimumHealth() const { return m_iMinimumHealth.Get(); };

	bool			  IsWallSightDisabled() const { return m_bWallSightDisabled.Get(); };
	bool			  IsOverlordMarked() const { return m_bOverlordMarked.Get(); };
	int				  GetPauseCount() const { return m_PauseCount.Get(); };
	bool			  IsPaused() const;
private:
	void			  EraseData();

	// Clients cannot refer directly to the Overlord
#ifndef CLIENT_DLL

	// This arrays need to stay here due to the fact that
	// different types are used to receive them (char for string_t etc.)
	//CNetworkArray(string_t, m_iszButtons, MAX_BUTTONS);

	CNetworkHandle(COverlordCamera, m_pCamera);
	CNetworkHandle(CBasePlayer, m_pOverlord);

	//float m_flWarModeRestart; // Last time warmode was enabled, 0 if it is disabled	

	float m_flLastCount;

	float m_OverlordThrow;

	CHandle<COverlordConsole> m_Console;
	CHandle<CBasePlayer> m_NextOverlord;

	float m_Pause;
#else
	CHandle<C_OverlordCamera> m_pCamera;
	CHandle<C_BasePlayer> m_pOverlord;
	CHandle<C_OverlordDataProxy> m_pProxy;
#endif

	// Basic network vars
	CNetworkVar(bool, m_bInVehicle);
	CNetworkVar(bool, m_bInWarMode);
	CNetworkVar(int, m_iPower);
	CNetworkVar(int, m_iMaxPower);
	CNetworkVar(bool, m_bCounter);
	CNetworkVar(bool, m_bInstaSpawn);

	CNetworkVar(float, m_flSecondsPerHealth);
	CNetworkVar(float, m_flSpawnMultiplier);
	CNetworkVar(int, m_iMinimumHealth);
	CNetworkVar(int, m_iHealthCap);
	CNetworkVar(int, m_iMaxTraps);

	CNetworkVar(bool, m_bWallSightDisabled);
	CNetworkVar(float, m_flWallSightDisabledEnd);

	CNetworkVar(bool, m_bOverlordMarked);
	CNetworkVar(float, m_flOverlordMarkedEnd);

	// We're gonna send this handles in order to let us control the traps client-side,
	// we need EHANDLE type though for networking reasons.
	// Store up to 64 traps
	CNetworkArray(EHANDLE, m_Traps, 64);

	CNetworkVar(float, m_Spawns);
	CNetworkVar(float, m_InitialSpawns);
	CNetworkVar(float, m_flLastResourceUpdate);
	CNetworkVar(int, m_PauseCount);

	CNetworkVar(bool, m_bTutorial);

	CNetworkVar(Build_t, m_BuildMode);
	CNetworkVar(int, m_Ferocity);

	

};


inline CBasePlayer * COverlordData::GetOverlord(void) const
{
	// That works for things like eo_makeoverlord, so when there is no overlord
	// the last one won't get spammed with useless messages
	if(!m_pOverlord || !m_pOverlord->IsOverlord())
		return NULL;

	return m_pOverlord.Get();
}


// This is needed for gamerules
#ifdef CLIENT_DLL
EXTERN_RECV_TABLE(DT_OverlordData);
#else
EXTERN_SEND_TABLE(DT_OverlordData);
#endif

COverlordData * GetOverlordData();

#endif