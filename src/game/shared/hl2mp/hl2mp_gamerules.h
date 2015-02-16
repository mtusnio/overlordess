//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MP_GAMERULES_H
#define HL2MP_GAMERULES_H
#pragma once

#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "gamevars_shared.h"
#include "overlord_data.h"

#ifndef CLIENT_DLL
#include "hl2mp_player.h"
//#include "overlord_camera.h"
#endif


#define ROUND_RESTART_TIME 10

#define VEC_CROUCH_TRACE_MIN	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMin
#define VEC_CROUCH_TRACE_MAX	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMax

enum
{
	TEAM_OVERLORD = 2,
	TEAM_REBELS,
	TEAM_CONVERTED,
};


// Use this one if you want to use your own assertion or
// cast it etc.
#define GET_OVERLORD_DATA GetOverlordData()

// This macro RETURNS after checking whether the data exists, although
// the data should always exist, use this one mostly if you want to
// stop the function when something goes wrong with the data
// (in order to let the player see the console etc.)

// Specify your own return type
// Client uses the HL2MPRules() assert for HUD, server does not need it as the HL2MPRules
// are always present 
#ifdef CLIENT_DLL
#define GET_OVERLORD_DATA_RETURN(pdata, _return) C_OverlordData * (pdata) = GET_OVERLORD_DATA; \
																			if(!(pdata)) \
																			{ \
																				return _return; \
																			}
#else
#define GET_OVERLORD_DATA_RETURN(pdata, _return) COverlordData * (pdata) = GET_OVERLORD_DATA; \
																			if(!(pdata)) \
																			{ \
																				return _return; \
																			}
#endif

// Client uses the HL2MPRules() assert for HUD, server does not need it as the HL2MPRules
// are always present 
#ifdef CLIENT_DLL
#define GET_OVERLORD_DATA_ASSERT(pdata) COverlordData * (pdata) = GET_OVERLORD_DATA; \
										if(!(pdata)) \
										{ \
											return; \
										}
#else
#define GET_OVERLORD_DATA_ASSERT(pdata) COverlordData * (pdata) = GET_OVERLORD_DATA
#endif

// Return false
#define GET_OVERLORD_DATA_ASSERT_FALSE(pdata) GET_OVERLORD_DATA_RETURN(pdata, false)

#define GET_OVERLORD_DATA_ASSERT_TRUE(pdata) GET_OVERLORD_DATA_RETURN(pdata, true)

#ifdef CLIENT_DLL
	#define CHL2MPRules C_HL2MPRules
	#define CHL2MPGameRulesProxy C_HL2MPGameRulesProxy
#endif

class CHL2MPGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CHL2MPGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class HL2MPViewVectors : public CViewVectors
{
public:
	HL2MPViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vCrouchTraceMin,
		Vector vCrouchTraceMax ) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
		m_vCrouchTraceMin = vCrouchTraceMin;
		m_vCrouchTraceMax = vCrouchTraceMax;
	}

	Vector m_vCrouchTraceMin;
	Vector m_vCrouchTraceMax;	
};



class CHL2MPRules : public CTeamplayRules
{
	// In fact, Overlord data is partly rules' interface
	// therefore they should be able to access
	// each other's variables
	friend class COverlordData;

public:
	DECLARE_CLASS( CHL2MPRules, CTeamplayRules );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif
	
	CHL2MPRules();
	virtual ~CHL2MPRules();

	virtual void Precache( void );
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );

	#ifndef CLIENT_DLL 
	virtual void InitDefaultAIRelationships(void);
	#endif

	virtual float FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon );
	virtual float FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon );
	virtual int WeaponShouldRespawn( CBaseCombatWeapon *pWeapon );
	virtual void Think( void );
	virtual void CreateStandardEntities( void );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual const char *GetGameDescription( void );
	// derive this function if you mod uses encrypted weapon info files
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"x9Ke0BY7"; }
	virtual const CViewVectors* GetViewVectors() const;
	const HL2MPViewVectors* GetHL2MPViewVectors() const;

	float GetMapRemainingTime();
	void CleanUpMap();
	void CheckRestartGame();
	void RestartGame();
	
#ifndef CLIENT_DLL
	virtual Vector VecItemRespawnSpot( CItem *pItem );
	virtual QAngle VecItemRespawnAngles( CItem *pItem );
	virtual float	FlItemRespawnTime( CItem *pItem );
	virtual bool	CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem );
	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );

	void	AddLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	ManageObjectRelocation( void );
	void    CheckChatForReadySignal( CHL2MP_Player *pPlayer, const char *chatmsg );
	const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	void SetInitialPlayers(int players) { m_InitialPlayers = players; };
	void IncrementDeathsAmount() { m_Deaths++; };
#endif
	virtual void ClientDisconnected( edict_t *pClient );

	bool CheckGameOver( void );
	bool IsIntermission( void );

	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );

	
	bool	IsTeamplay( void ) { return m_bTeamPlayEnabled;	}
	void	CheckAllPlayersReady( void );

	virtual bool FPlayerCanRespawn( CBasePlayer *pPlayer );

	//CPointEntity *  GetJail() { return m_pJail; };
	bool		IsRestarting() const { return (m_flRestartGameTime != 0); };
	float		GetLastRestart() const { return m_flLastRestart; };
	float		GetRoundTime() const;
	//COverlordData * GetOverlordData(void) const { return &m_OverlordData; };

	int GetInitialPlayers() const { return m_InitialPlayers; };
	int GetDeathsAmount() const { return m_Deaths; };
private:

	void OverlordWins();
	void OverlordLoses();
	
	//CPointEntity *	PickJail();

	CNetworkVar( bool, m_bTeamPlayEnabled );
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar( int, m_InitialPlayers );
	CNetworkVar( int, m_Deaths );

	CUtlVector<EHANDLE> m_hRespawnableItemsAndWeapons;

	float m_tmNextPeriodicThink;
	float m_flRestartGameTime;
	float m_flLastRestart;

	bool m_bCompleteReset;
	bool m_bAwaitingReadyRestart;
	bool m_bHeardAllPlayersReady;

	//CPointEntity * m_pJail;
	// Vars for the Overlord player
	/*CNetworkVar(bool, m_bInVehicle);
	CNetworkVar(int, m_iPower);
	CNetworkVector(m_vVehicleVector);
	CNetworkQAngle(m_VehicleAngle);*/ // All are now included in COverlordData class
	



};

inline CHL2MPRules* HL2MPRules()
{
	return static_cast<CHL2MPRules*>(g_pGameRules);
}

#endif //HL2MP_GAMERULES_H
