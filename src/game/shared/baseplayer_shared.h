//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEPLAYER_SHARED_H
#define BASEPLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// PlayerUse defines
#define	PLAYER_USE_RADIUS	80.f
#define CONE_45_DEGREES		0.707f
#define CONE_15_DEGREES		0.9659258f
#define CONE_90_DEGREES		0

#define TRAIN_ACTIVE	0x80 
#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

// entity messages
#define PLAY_PLAYER_JINGLE	1
#define UPDATE_PLAYER_RADAR	2

#define DEATH_ANIMATION_TIME	3.0f

// multiplayer only
#define NOINTERP_PARITY_MAX			4
#define NOINTERP_PARITY_MAX_BITS	2

// Class consts

/*
#ifndef OVERLORD_PLAYTEST
#define OVERLORD_MAX_HEALTH 450
#define ASSAULT_MAX_HEALTH 120
#define PSION_MAX_HEALTH 100
#define HACKER_MAX_HEALTH 100
#define STEALTH_MAX_HEALTH 65
#define CONVERTED_MAX_HEALTH 110

#define OVERLORD_MAX_ARMOR 100
#define ASSAULT_MAX_ARMOR 20
#define PSION_MAX_ARMOR 80
#define HACKER_MAX_ARMOR 30
#define STEALTH_MAX_ARMOR 45
#define CONVERTED_MAX_ARMOR 85
#else*/
// Playtest commands let us change the maximum health ingame
extern ConVar eo_playtest_overlordhealth;
extern ConVar eo_playtest_assaulthealth;
extern ConVar eo_playtest_psionhealth;
extern ConVar eo_playtest_hackerhealth;
extern ConVar eo_playtest_stealthhealth;
extern ConVar eo_playtest_convertedhealth;

extern ConVar eo_playtest_tankarmour;
extern ConVar eo_playtest_phasingarmour;
extern ConVar eo_playtest_techarmour;

extern ConVar eo_playtest_assaultarmour;
extern ConVar eo_playtest_hackerarmour;
extern ConVar eo_playtest_stealtharmour;
extern ConVar eo_playtest_psionarmour;

extern ConVar hl2_walkspeed;
extern ConVar hl2_normspeed;
extern ConVar hl2_sprintspeed;

#define OVERLORD_MAX_HEALTH GET_OVERLORD_DATA->GetMinimumHealth()
#define ASSAULT_MAX_HEALTH eo_playtest_assaulthealth.GetInt()
#define PSION_MAX_HEALTH eo_playtest_psionhealth.GetInt()
#define HACKER_MAX_HEALTH eo_playtest_hackerhealth.GetInt()
#define STEALTH_MAX_HEALTH eo_playtest_stealthhealth.GetInt()
#define CONVERTED_MAX_HEALTH eo_playtest_convertedhealth.GetInt()

#define TANK_MAX_ARMOR eo_playtest_tankarmour.GetInt()
#define PHASING_MAX_ARMOR eo_playtest_phasingarmour.GetInt()
#define TECH_MAX_ARMOR eo_playtest_techarmour.GetInt()

#define ASSAULT_MAX_ARMOR eo_playtest_assaultarmour.GetInt()
#define PSION_MAX_ARMOR eo_playtest_psionarmour.GetInt()
#define HACKER_MAX_ARMOR eo_playtest_hackerarmour.GetInt()
#define STEALTH_MAX_ARMOR eo_playtest_stealtharmour.GetInt()
#define CONVERTED_MAX_ARMOR 85

//#endif
// Psi protection bits
#define NO_PROTECTION ( 1 << 0 )
#define PSI_SHIELD ( 1 << 1 )
#define PHYS_SHIELD ( 1 << 2 )
#define BULLET_SHIELD ( 1 << 3 )

enum
{
	INVIS_OFF = 0,
	INVIS_ON  =  ( 1<<1 ),
	INVIS_EXTERNAL = ( 1<<2 ),
};

typedef struct 
{
	Vector		m_vecAutoAimDir;		// The direction autoaim wishes to point.
	Vector		m_vecAutoAimPoint;		// The point (world space) that autoaim is aiming at.
	EHANDLE		m_hAutoAimEntity;		// The entity that autoaim is aiming at.
	bool		m_bAutoAimAssisting;	// If this is true, autoaim is aiming at the target. If false, the player is naturally aiming.
	bool		m_bOnTargetNatural;		
	float		m_fScale;
	float		m_fMaxDist;
} autoaim_params_t;

enum stepsoundtimes_t
{
	STEPSOUNDTIME_NORMAL = 0,
	STEPSOUNDTIME_ON_LADDER,
	STEPSOUNDTIME_WATER_KNEE,
	STEPSOUNDTIME_WATER_FOOT,
};

/*
enum PlayerClass_t
{
	CLASS_DEFAULT = 0, // Mainly for spectators, overlord uses his last selected class now
	CLASS_ASSAULT = (1 >> 1),
	CLASS_PSION = (1 >> 2),
	CLASS_HACKER = (1 >> 3),
	CLASS_STEALTH = (1 >> 4), 
	// Some more?
	CLASS_NUMBER = 5, // last class, gives us the number of classes
	
};*/

// Definitions needed for returning combinations
#define CLASS_DEFAULT 0 // Mainly for spectators, overlord uses his last selected class now
#define	CLASS_ASSAULT  (1 << 1)
#define	CLASS_PSION  (1 << 2)
#define	CLASS_HACKER  (1 << 3)
#define	CLASS_STEALTH  (1 << 4)

#define CLASS_REBEL_COUNT 4

#define CLASS_TANK (1 << 5)
#define CLASS_PHASING (1 << 6)
#define CLASS_TECH (1 << 7)

#define CLASS_OVERLORD_COUNT 3

#define CLASS_NUMBER 8
#define CLASS_ALL (CLASS_ASSAULT | CLASS_PSION | CLASS_HACKER | CLASS_STEALTH)

// Ov classes
typedef int PlayerClass_t;

const char g_PlayerClassnames[CLASS_NUMBER][20] = 
{ 
  "Default",
  "Assault", 
  "Psion",
  "Hacker",
  "Operative",
  "Offensive",
  "Phasing",
  "TechOv",
};


// Shared header file for players
#if defined( CLIENT_DLL )
#define CBasePlayer C_BasePlayer
#include "c_baseplayer.h"
#else
#include "player.h"
#endif

#endif // BASEPLAYER_SHARED_H
