//==============Overlord mod=====================
//	Door console
//	
//===============================================

#ifndef H_OV_DOOR_CONSOLE
#define H_OV_DOOR_CONSOLE

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

#define AREAS 2
#define NEXT_HACK 0.15f // was 0.5f

class COverlordArea;

class COverlordDoorLock : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordDoorLock, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	COverlordDoorLock();
	virtual ~COverlordDoorLock();

	virtual void Precache();

	virtual void Spawn();
	virtual void Activate();

	virtual void Think();

	virtual void ConsoleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	virtual void ResolveAreas();

	virtual void SetLocked(bool bLocked) { m_bLocked = bLocked; };
	virtual bool IsLocked() { return m_bLocked; };

	virtual void RunHack(CBasePlayer * pPlayer);

	virtual void InputLock(inputdata_t & inputData);
	virtual void InputUnlock(inputdata_t & inputData);

	const char * GetAreaName(int i) 
	{ 
		if(i > (AREAS - 1))
			return NULL;

		return m_iszAreaName[i];
	}

	const char * GetAreaEntityName(int i) 
	{ 
		if(i > (AREAS - 1))
			return NULL;

		return STRING(m_iszAreaEntName[i]);
	}

	static COverlordDoorLock * GetLastHacked() { return m_LastHacked; };

	bool CreateVPhysics()
	{
		VPhysicsInitNormal( SOLID_BBOX, 0, false );
		return true;
	}

protected:
	// Gets the amount of players around the console
	int GetNumPlayersAround();

	int  ObjectCaps();

	virtual void Lock(bool bOutput = true);
	virtual void Unlock(bool bOutput = true);

	CNetworkVar(float, m_flHackTime);
	CNetworkVar(float, m_flHacked);
	CNetworkVar(bool, m_bLocked);
	CNetworkString(m_HackingString, 64);
	CNetworkString(m_HackedString, 64);
	int   m_PercentageAlive;
	int   m_PlayerDistance;


	float m_flMaximumHackTime;
	float m_flMinimumHackTime;

	CHandle<CBasePlayer> m_hHacker;

	
	float m_flNextHack;
	float m_flSoundTime;

	

	char m_iszAreaName[AREAS][MAX_AREA_NAME_SIZE];
	string_t m_iszAreaEntName[AREAS];

	COutputEvent m_OnUnlocked;
	COutputEvent m_OnLocked;

	static CHandle<COverlordDoorLock> m_LastHacked;

};


#endif