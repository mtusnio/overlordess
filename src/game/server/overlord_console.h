//==============Overlord mod=====================
//	Overlord's console, should be also declared
//	client-side.
//===============================================

#ifndef OV_CONSOLE_H
#define OV_CONSOLE_H

#ifdef _WIN32
#pragma once
#endif

#include "overlord_area.h"

class COverlordCamera;

class COverlordConsole : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordConsole, CBaseEntity);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	
	COverlordConsole();
	virtual ~COverlordConsole();

	void		Spawn();
	bool		CreateVPhysics();
	void		Activate();

	void		ResolveNames();
	const char* GetAreaName() { return m_iszArea; };

	bool		IsEnabled() { return m_bEnabled; };

	void		UseConsole( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	    SetEnabled(bool enabled) { m_bEnabled = enabled; };

	void		OverwriteCamera(COverlordCamera * pCam);	

	// This one takes spawn flags into account
	void		UpdateCamera(COverlordCamera * pCam);

	void		InputLock(inputdata_t & data);
	void		InputUnlock(inputdata_t & data);
protected:
	virtual int	ObjectCaps(void);

private:
	CNetworkVar(bool, m_bEnabled);
	CNetworkString(m_iszArea, MAX_AREA_NAME_SIZE);

	// Again, someone may want to destroy the camera during the game
	CHandle<COverlordCamera> m_pCamera;
	string_t m_iszCamera;

};



#endif