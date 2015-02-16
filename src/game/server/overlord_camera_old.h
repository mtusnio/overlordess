//==============Overlord mod=====================
//	Overlord's camera
//	
//===============================================

#ifndef H_OV_CAMERA
#define H_OV_CAMERA

#ifdef _WIN32
#pragma once
#endif

#ifdef OVER_DEBUG
#include "baseanimating.h"
#else
#include "baseentity.h"
#endif

#include "overlord_modulelist.h"
#include "overlord_baseturret.h"
#include "overlord_area.h"


#define MAX_STRING 256

#define SF_START_DISABLED 1
#define SF_DONT_REVERT_ANGLES 128
#define SF_NOSPEEDCHANGES 256

#define NUM_OF_LISTS 8

#define LAST_LIST (NUM_OF_LISTS - 1)
#define FIRST_LIST 0

class COverlordBaseModule;

// Inheriting from CBaseAnimating lets us use the camera model

// We inherit from base turret for rotation purposes
class COverlordCamera : public CBaseAnimating, public COverlordBaseTurret 
{
public:
	DECLARE_CLASS(COverlordCamera, CBaseAnimating);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	COverlordCamera();
	virtual ~COverlordCamera();


	virtual void Spawn();
	virtual void Activate();
	virtual void Precache();

	virtual void Think();

	virtual void InputExit(inputdata_t &inputData);
	virtual void InputSwitch(inputdata_t &inputData);

	virtual void ResetTurret();

	virtual Vector GetCameraVector() const 
	{
		if(m_flDist == 0.0f)
		{
			Vector forw;
			AngleVectors(GetCameraAngles(), &forw);

			return GetLocalOrigin() + forw * 4; 
		}

		return GetShootVector();
	};
	virtual QAngle GetCameraAngles() const { return EyeAngles(); };
	virtual COverlordBaseModule * GetModule(int i) const 
	{ 
		if(!GetCurrentList())
			return NULL;

		return GetCurrentList()->GetModule(i); 
	};

	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	// Button and menu stuff
	virtual const char * GetDescription() const { return STRING(m_iszDesc.Get()); };
	virtual COverlordArea * GetNearestArea(void) const;

	virtual bool ShouldShowCrosshair(void) const { return false; };


	virtual void FireOnModule(int module) 
	{ 
		if(!GetCurrentList())
			return;

		GetCurrentList()->FireOnModule(module);
	};

	virtual void FireOnExit() { m_OnExit.FireOutput(NULL, this); };
	virtual void FireOnEnter() { m_OnEnter.FireOutput(NULL, this); };
	virtual void FireOnUse() { m_OnUsePressed.FireOutput(NULL, this); };
	virtual void FireOnForward() { m_OnForwardPressed.FireOutput(NULL, this); };
	virtual void FireOnBack() { m_OnBackPressed.FireOutput(NULL, this); };
	virtual void FireOnRight() { m_OnRightPressed.FireOutput(NULL, this); };
	virtual void FireOnLeft() { m_OnLeftPressed.FireOutput(NULL, this); };
	virtual void FireOnJump() { m_OnJumpPressed.FireOutput(NULL, this); };
	virtual void FireOnReload() { m_OnReloadPressed.FireOutput(NULL, this); };
	virtual void FireOnDuck() {  m_OnDuckPressed.FireOutput(NULL, this); };

	virtual void InputEnable(inputdata_t &inputData) { Enable(); };
	virtual void InputDisable(inputdata_t &inputData) { Disable(); };
	virtual void SetPreviousCamera(inputdata_t &inputData);

	virtual void OverrideNextCamera(inputdata_t &inputData);
	virtual void OverridePreviousCamera(inputdata_t &inputData);

	virtual void ListForward();
	virtual void ListBackward();

	bool IsEnabled() const { return m_bEnabled; };

	//void SetButtonsCaption(const char * caption, int button) { m_iszButtons[button] = AllocPooledString(caption); };
	//void SetButtonsCaption(string_t caption, int button) { m_iszButtons[button] = caption; };

	virtual int GetCurrentListNumber() const { return m_CurrentList.Get(); };
	virtual COverlordModuleList * GetCurrentList() const { return m_lLists[GetCurrentListNumber()]; };
	virtual int GetLastListNumber() const;
	
	virtual void SetPreviousCamera(COverlordCamera * camera) 
	{
		m_pPrevious = camera; 
	};
	virtual void SetUpperCamera(COverlordCamera * pCam)
	{
		m_pUpper = pCam;
	};
	virtual void SetLowerCamera(COverlordCamera * pCam)
	{
		m_pLower = pCam;
	};

	virtual COverlordCamera * GetNextCamera() const { return m_pNext.Get(); };
	virtual COverlordCamera * GetPreviousCamera() const { return m_pPrevious.Get(); };
	virtual COverlordCamera * GetUpperCamera() const { return m_pUpper; };
	virtual COverlordCamera * GetLowerCamera() const { return m_pLower; };
	
	COverlordArea * GetArea() const { return m_pArea.Get(); };

	virtual float GetYawSpeed() const { return COverlordCamera::GetMoveSpeed(); };
	virtual float GetPitchSpeed() const { return COverlordCamera::GetMoveSpeed(); };
	virtual float GetMoveSpeed() const;

protected:
	virtual void RotateRight()
	{
		if(GetMaxYaw() <= 0)
			return;

		if(m_flLastMove + 0.135f <= gpGlobals->curtime)
		{
			m_flFirstMove = gpGlobals->curtime;
		}
		
		m_flLastMove = gpGlobals->curtime;

		COverlordBaseTurret::RotateRight();
	}

	virtual void RotateLeft()
	{
		if(GetMaxYaw() <= 0)
			return;

		if(m_flLastMove + 0.135f <= gpGlobals->curtime)
		{
			m_flFirstMove = gpGlobals->curtime;
		}
			
		m_flLastMove = gpGlobals->curtime;

		COverlordBaseTurret::RotateLeft();
	}

	virtual void MoveUp()
	{
		if(GetMaxPitch() <= 0)
			return;

		if(m_flLastMove + 0.135f <= gpGlobals->curtime)
		{
			m_flFirstMove = gpGlobals->curtime;
		}
	
		m_flLastMove = gpGlobals->curtime;

		COverlordBaseTurret::MoveUp();
	}

	virtual void MoveDown()
	{
		if(GetMaxPitch() <= 0)
			return;

		if(m_flLastMove + 0.135f <= gpGlobals->curtime)
		{
			m_flFirstMove = gpGlobals->curtime;
		}
	
		m_flLastMove = gpGlobals->curtime;

		COverlordBaseTurret::MoveDown();
	}

private:
	virtual void Disable();
	virtual void Enable();

	virtual void HandleLists();
	virtual void HandleCameras();

	float m_flFirstMove;
	float m_flLastMove;

	bool m_bEnabled;
	string_t m_iszAreaName;
	CNetworkVar(string_t, m_iszDesc);

	// A little hack, we need this offset for client-side turret implementation
	CNetworkVector(m_vOffset);
	//string_t m_iszButtons[MAX_BUTTONS];

	//COutputEvent m_OnButtonPressed[MAX_BUTTONS];
	CNetworkVar(int, m_CurrentList);
	CNetworkArray(CHandle<COverlordModuleList>, m_lLists, NUM_OF_LISTS);

	// Only used to fill the handles
	const char * m_iszList[NUM_OF_LISTS];
	const char * m_iszPrevCamera;
	const char * m_iszNextCam;
	const char * m_iszUpperCam;
	const char * m_iszLowerCam;
	const char * m_iszOriginBrush;

	// Next camera is filled by the mapmaker
	// Previous is set while switching cameras
	CNetworkHandle(COverlordCamera, m_pNext);
	CNetworkHandle(COverlordCamera, m_pPrevious);
	CNetworkHandle(COverlordCamera, m_pUpper);
	CNetworkHandle(COverlordCamera, m_pLower);

	CNetworkHandle(COverlordArea, m_pArea);

	COutputEvent m_OnUsePressed;
	COutputEvent m_OnForwardPressed;
	COutputEvent m_OnBackPressed;
	COutputEvent m_OnRightPressed;
	COutputEvent m_OnLeftPressed;
	COutputEvent m_OnJumpPressed;
	COutputEvent m_OnReloadPressed;
	COutputEvent m_OnDuckPressed;
	COutputEvent m_OnEnter;
	COutputEvent m_OnExit;

};

#endif