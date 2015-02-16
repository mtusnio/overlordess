//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADEAR2_H
#define	GRENADEAR2_H

#include "basegrenade_shared.h"

#define	MAX_AR2_NO_COLLIDE_TIME 0.2

class SmokeTrail;


extern ConVar eo_emp_disable_time;
extern ConVar eo_emp_radius;

class COverlordEMP : public CBaseGrenade
{
public:
	DECLARE_CLASS( COverlordEMP, CBaseGrenade );

	CHandle< SmokeTrail > m_hSmokeTrail;
	float				 m_fSpawnTime;
	float				m_fDangerRadius;


	void		Spawn( void );
	void		Precache( void );
	void 		GrenadeAR2Touch( CBaseEntity *pOther );
	void		GrenadeAR2Think( void );
	void		Event_Killed( const CTakeDamageInfo &info );

public:
	void EXPORT				Detonate(void);
	COverlordEMP(void);

	DECLARE_DATADESC();
};

#endif	//GRENADEAR2_H
