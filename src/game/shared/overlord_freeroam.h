//==============Overlord mod=====================
//	Freeroam dynamic camera
//===============================================

#ifndef H_OV_FREEROAM
#define H_OV_FREEROAM

#include "overlord_camera.h"

#ifdef CLIENT_DLL
#define COverlordFreeRoam C_OverlordFreeRoam
#endif

class COverlordFreeRoam : public COverlordCamera
{
public:
	DECLARE_CLASS(COverlordFreeRoam, COverlordCamera);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();
#endif

	COverlordFreeRoam();
	virtual ~COverlordFreeRoam();

	// Override speed etc.
	virtual float GetYawSpeed() const { return (IsInMouseRotation()) ? m_LastSpeedX : BaseClass::GetYawSpeed(); };
	virtual float GetPitchSpeed() const { return (IsInMouseRotation()) ? m_LastSpeedY : BaseClass::GetPitchSpeed(); };
	virtual float GetSpeedPerSecond() const { return 155.0f; };

	virtual int		GetMaxYaw() const { return 360; };
	virtual int		GetMaxPitch() const { return 360; };
	
	virtual bool    MovesOnRelativePitch() const { return true; }
	virtual bool	MovesOnRelativeYaw() const   { return false; };

	virtual Vector GetCameraVector() const 
	{
		return GetAbsOrigin();
	};
	
	// Now relay all calls to our parent camera

	virtual float GetFreeroamSpeed() const;

	virtual void ProcessMovement(const CUserCmd * cmd);;

	virtual bool UsesSpeedChanges() const { return false; };
#ifndef CLIENT_DLL
	virtual void FireOnExit();

#endif

	virtual void MoveUp()
	{
		QAngle before = GetAbsAngles();

		BaseClass::MoveUp();

		QAngle after = GetAbsAngles();
		if((before[PITCH] - after[PITCH] ) < 0.0f)
		{
			after[PITCH] = -90.0f;
			SetAbsAngles(after);
		}
	}

	virtual void MoveDown()
	{
		QAngle before = GetAbsAngles();

		BaseClass::MoveDown();

		QAngle after = GetAbsAngles();
		if((before[PITCH] - after[PITCH] ) > 0.0f)
		{
			after[PITCH] = 90.0f;
			SetAbsAngles(after);
		}
	}
private:
	virtual void MoveCamera(const Vector & dir, float flSpeed);

	float m_LastSpeedX;
	float m_LastSpeedY;

	float m_flSwitchDelay;

	bool m_bWasInMouseRotation;

	CNetworkVar(float, m_flMoveStartTime);

#ifndef CLIENT_DLL
	COverlordCamera * GetNearestCamera(bool bLineOfSight);

	
#else
	CHandle<C_OverlordCamera> m_OldParentCamera;
#endif
};






#endif