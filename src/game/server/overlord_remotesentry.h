//==============Overlord mod=====================
//	Sentrygun controlled by the overlord
//	
//===============================================



#ifndef H_OV_SENTRY
#define H_OV_SENTRY

#ifdef _WIN32
#pragma once
#endif

#include "overlord_camera.h"

#define AMMO_BUTTON 0

#define SF_CONCENTRATE_BARRELS 2

class COverlordData;

class COverlordSentry : public COverlordCamera
{
public:
	DECLARE_CLASS(COverlordSentry, COverlordCamera);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	COverlordSentry();
	virtual ~COverlordSentry();

	virtual void Think();
	virtual void Spawn();
	virtual void Activate();
	
	virtual bool ShouldShowCrosshair(void) const { return true; };

	//virtual void FireOnUse() { m_OnUsePressed.FireOutput(NULL, this); };

	// These binds are used for sentry's movement, therefore
	// it is neccessary to disable them
	virtual void FireOnForward() { return; };
	virtual void FireOnBack() { return; };
	virtual void FireOnRight() { return; };
	virtual void FireOnLeft() { return; };
	virtual void FireOnDuck() { return; };

	//virtual void FireOnJump() { m_OnJumpPressed.FireOutput(NULL, this); };
	//virtual void FireOnReload() { m_OnReloadPressed.FireOutput(NULL, this); };
protected:
	virtual void Fire();
	
	//char *	m_iszModuleName;

};

#endif