//==============Overlord mod=====================
//	Freeroam dynamic camera
//===============================================

#include "cbase.h"
#include "overlord_freeroam.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"
#include "overlord_data.h"

#ifdef CLIENT_DLL
extern ConVar eo_predict_cameras;
#endif

BEGIN_DATADESC(COverlordFreeRoam)

END_DATADESC()

LINK_ENTITY_TO_CLASS(overlord_freeroam, COverlordFreeRoam);

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordFreeRoam, DT_OverlordFreeRoam)

BEGIN_NETWORK_TABLE(COverlordFreeRoam, DT_OverlordFreeRoam)
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flMoveStartTime)),
#else
	RecvPropFloat(RECVINFO(m_flMoveStartTime)),
#endif

END_NETWORK_TABLE()
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(C_OverlordFreeRoam)
	DEFINE_PRED_FIELD(m_flMoveStartTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif
COverlordFreeRoam::COverlordFreeRoam()
{
	m_LastSpeedX = 0.0f;
	m_LastSpeedY = 0.0f;
	m_flSwitchDelay = 0.0f;
	m_bWasInMouseRotation = false;
	m_flMoveStartTime = 0.0f;
	m_flLastMove = 0.0f;
#ifdef CLIENT_DLL
	m_OldParentCamera = NULL;
#endif
}

COverlordFreeRoam::~COverlordFreeRoam()
{
}

#ifndef CLIENT_DLL
void COverlordFreeRoam::FireOnExit()
{
	// Remove us
	//UTIL_Remove(this);
}

#endif

float COverlordFreeRoam::GetFreeroamSpeed() const
{
	const float MAX_TIME = 1.75f;
	const float MAX_SPEED = 20.0f;
	const float MIN_SPEED = 11.0f;

	float fraction = clamp(gpGlobals->curtime - m_flMoveStartTime, 0, MAX_TIME)/MAX_TIME;
	
	return clamp(fraction * MAX_SPEED, MIN_SPEED, MAX_SPEED);
}


#ifndef CLIENT_DLL
COverlordCamera * COverlordFreeRoam::GetNearestCamera(bool bLineOfSight)
{
	CBaseEntity * pEntity = NULL;
	COverlordCamera * pNewCamera = NULL;
	float flDist = FLT_MAX;

	while((pEntity = gEntList.FindEntityByClassname(pEntity, "point_overcamera")) != NULL)
	{
		COverlordCamera * pCam = static_cast<COverlordCamera*>(pEntity);

		float dist = (GetCameraVector() - pCam->GetCameraVector()).Length();
		if(dist < flDist)
		{
			if(bLineOfSight)
			{
				trace_t tr;
				UTIL_TraceLine(GetCameraVector(), pCam->GetCameraVector(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

				if(tr.fraction >= 0.985f)
				{
					pNewCamera = pCam;
					flDist = dist;
					continue;
				}
			}
			else
			{
				pNewCamera = pCam;
				flDist = dist;
				continue;
			}
		}
	}

	return pNewCamera;
}
#endif

void COverlordFreeRoam::MoveCamera(const Vector &dir, float flSpeed)
{
	Vector newOrigin = GetAbsOrigin() + (dir * flSpeed);

	SetAbsOrigin(newOrigin);
}

void COverlordFreeRoam::ProcessMovement(const CUserCmd * cmd)
{
	int nButtons = cmd->buttons;

#ifdef CLIENT_DLL
	HandleKeyboardInput();
#endif

	// Check whether we are in freeroam mode
	if(IsInMouseRotation())
	{
		if(!m_bWasInMouseRotation)
		{
			m_bWasInMouseRotation = true;
			m_flSwitchDelay = 0.0f;
		}

		QAngle angles = EyeAngles();

		Vector forw, right;
		GetVectors(&forw, &right, NULL);

		// Any of the move buttons pressed, sort out our move times
		if(!(nButtons & (IN_FORWARD|IN_MOVELEFT|IN_MOVERIGHT|IN_BACK)))
			m_flMoveStartTime = 0.0f;
		else if(m_flMoveStartTime == 0.0f)
			m_flMoveStartTime = gpGlobals->curtime;

		if(nButtons & IN_FORWARD)
			MoveCamera(forw, GetFreeroamSpeed());
		if(nButtons & IN_MOVELEFT)
			MoveCamera(-right, GetFreeroamSpeed());
		if(nButtons & IN_MOVERIGHT)
			MoveCamera(right, GetFreeroamSpeed());
		if(nButtons & IN_BACK)
			MoveCamera(-forw, GetFreeroamSpeed());

		m_LastSpeedX = MOUSE_SPEED * (float)cmd->mousedx;
		m_LastSpeedY = MOUSE_SPEED * (float)cmd->mousedy;

		if(m_LastSpeedX < 0)
			m_LastSpeedX = -m_LastSpeedX;

		if(m_LastSpeedY < 0)
			m_LastSpeedY = -m_LastSpeedY;

		if(cmd->mousedx < 0)
			RotateLeft();
		else if(cmd->mousedx > 0)
			RotateRight();

		if(cmd->mousedy < 0)
			MoveUp();
		else if(cmd->mousedy > 0)
			MoveDown();
	}
	else
	{
		if(m_bWasInMouseRotation)
		{
			m_bWasInMouseRotation = false;
			m_flSwitchDelay = gpGlobals->curtime + 0.25f;
		}

		if(m_flSwitchDelay > gpGlobals->curtime)
			return;

#ifndef CLIENT_DLL
		 if(nButtons & IN_FORWARD)
			FireOnForward();
		 if(nButtons & IN_BACK)
			FireOnBack();
		 if(nButtons & IN_MOVELEFT)
			FireOnLeft();
		 if(nButtons & IN_MOVERIGHT)
			FireOnRight();
		 if(nButtons & IN_JUMP)
			FireOnJump();
		 if(nButtons & IN_DUCK)
			FireOnDuck();
		 if(nButtons & IN_RELOAD)
			FireOnReload();
#endif
		if(nButtons & IN_FORWARD)
			 MoveUp();
		if(nButtons & IN_BACK)
			 MoveDown();
		if(nButtons & IN_MOVELEFT)
			RotateLeft();
		if(nButtons & IN_MOVERIGHT)
			 RotateRight();
	}
}
