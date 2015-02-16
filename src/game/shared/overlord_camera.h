//==============Overlord mod=====================
//	Overlord's camera
//	
//===============================================

#ifndef H_OV_CAMERA
#define H_OV_CAMERA


#ifndef CLIENT_DLL
#include "overlord_modulelist.h"
#include "overlord_area.h"
#else
#include "c_overlord_modulelist.h"
struct sTrapDescription;
#endif

#include "overlord_baseturret.h"

#define BACKGROUND_ALPHA 120

#define MAX_STRING 256

#define MOUSE_SPEED 0.1f

#define SF_START_DISABLED 1
#define SF_DONT_REVERT_ANGLES 128
#define SF_NOSPEEDCHANGES 256

#define CAMERA_THINK 0.02f

#define NUM_OF_LISTS 8

#define LAST_LIST (NUM_OF_LISTS - 1)
#define FIRST_LIST 0

#ifndef CLIENT_DLL
class COverlordBaseModule;
#endif

#ifdef CLIENT_DLL
#define COverlordCamera C_OverlordCamera
#define COverlordBaseModule C_OverlordBaseModule
#define COverlordModuleList C_OverlordModuleList
#define COverlordArea C_OverlordArea
#endif

class COverlordArea;

// Inheriting from CBaseAnimating lets us use the camera model

// We inherit from base turret for rotation purposes
class COverlordCamera : public COverlordBaseTurret 
{
	/* Static stuff */
public:
	static void SetMouseRotationEnabled(bool bEnabled);
private:
	static bool m_MouseRotation;
public:
	DECLARE_CLASS(COverlordCamera, COverlordBaseTurret);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();
#endif

	COverlordCamera();
	virtual ~COverlordCamera();

	// Predictions
	virtual bool IsPredicted() const { return true; };
#ifdef CLIENT_DLL
	virtual bool ShouldPredict();
#endif

	virtual void Spawn();
	virtual void Precache();

#ifdef CLIENT_DLL
	virtual int   DrawModel(int flags);
#endif

	//virtual bool IsPredicted() { return true; };
#ifndef CLIENT_DLL
	virtual void InputExit(inputdata_t &inputData);
	virtual void InputSwitch(inputdata_t &inputData);
#endif
	virtual void ResetTurret();

	virtual Vector GetCameraVector() const 
	{
		return GetShootVector();
	};

	virtual QAngle GetCameraAngles() const { return EyeAngles(); };

#ifndef CLIENT_DLL
	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}
#endif

	// Button and menu stuff
#ifndef CLIENT_DLL
	virtual const char * GetDescription() const { return STRING(m_iszDesc.Get()); };
#else
	virtual const char * GetDescription() const { return m_iszDesc; };
#endif
	virtual bool ShouldShowCrosshair(void) const { return true; };

	virtual bool HasSpawnFlags(int flags) const { return (m_spawnflags & flags) != 0; };

	virtual bool ShouldBlockControls() const { return false; };
#ifndef CLIENT_DLL
	virtual void FireOnModule(int module); 
	/*{ 

		if(!GetCurrentList())
			return;

		GetCurrentList()->FireOnModule(module);
	};*/

	virtual void FireOnExit() { OnCameraExit(); m_OnExit.FireOutput(NULL, this); };
	virtual void FireOnEnter() { OnCameraEnter(); m_OnEnter.FireOutput(NULL, this); };
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

	virtual bool IsEnabled() const { return m_bEnabled; };
#endif
	//void SetButtonsCaption(const char * caption, int button) { m_iszButtons[button] = AllocPooledString(caption); };
	//void SetButtonsCaption(string_t caption, int button) { m_iszButtons[button] = caption; };

	// Do the same, kept for backward compatibility

	virtual float GetYawSpeed() const { return m_flSpeedOverrideX > 0 ? m_flSpeedOverrideX : COverlordCamera::GetMoveSpeed(); };
	virtual float GetPitchSpeed() const { return m_flSpeedOverrideY > 0 ? m_flSpeedOverrideY : COverlordCamera::GetMoveSpeed(); };
	virtual float GetMoveSpeed() const;
	virtual float GetSpeedPerSecond() const { return COverlordBaseTurret::GetMoveSpeed() * 10; };

	virtual bool UsesSpeedChanges() const { return !HasSpawnFlags(SF_NOSPEEDCHANGES); };

	virtual void ProcessMovement(const CUserCmd * cmd);

	void OnCameraEnter();
	void OnCameraExit();

	virtual bool	  IsInMouseRotation() const;
protected:
#ifdef CLIENT_DLL
	virtual void     HandleKeyboardInput();
#endif


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

	float m_flSpeedOverrideX;
	float m_flSpeedOverrideY;

#ifndef CLIENT_DLL
	virtual void Disable();
	virtual void Enable();

	CNetworkVar(float, m_flFirstMove);
	CNetworkVar(float, m_flLastMove);

	CNetworkVar(string_t, m_iszDesc);

	//string_t m_iszButtons[MAX_BUTTONS];

	//COutputEvent m_OnButtonPressed[MAX_BUTTONS];
	CNetworkVar(int, m_CurrentList);


	// Only used to fill the handles
	const char * m_iszList;
	const char * m_iszPrevCamera;
	const char * m_iszNextCam;
	const char * m_iszUpperCam;
	const char * m_iszLowerCam;

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

	bool m_bEnabled;
	string_t m_iszAreaName;
#else
	CHandle<C_OverlordCamera> m_pNext;
	CHandle<C_OverlordCamera> m_pPrevious;
	CHandle<C_OverlordCamera> m_pUpper;
	CHandle<C_OverlordCamera> m_pLower;

	int m_spawnflags;
	Vector m_vOffset;

	char m_iszDesc[DESC_LENGTH];
	int m_CurrentList;

	float m_flFirstMove;
	float m_flLastMove;

	bool m_BuildMode;

	// Trap panels
	vgui::VPANEL m_Panels[MAX_BUTTONS];
#endif

};

#endif