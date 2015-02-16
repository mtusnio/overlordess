//==============Overlord mod=====================
//	Overlord's camera
//	
//===============================================

#include "cbase.h"
#include "overlord_movingcamera.h"
#include "overlord_camera.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"


#define MOVCAM_THINK 0.1f

BEGIN_DATADESC(COverlordMovCam)

	DEFINE_THINKFUNC(Think),

//	DECLARE_CAMERA(),

	DEFINE_KEYFIELD(m_aForward, FIELD_VECTOR, "ForwardAxis"),
	DEFINE_KEYFIELD(m_aSide, FIELD_VECTOR, "SideAxis"),

	DEFINE_OUTPUT(m_OnUsePressed, "OnUsePressed"),
	DEFINE_OUTPUT(m_OnJumpPressed, "OnJumpPressed"),
	DEFINE_OUTPUT(m_OnReloadPressed, "OnReloadPressed"),
	DEFINE_OUTPUT(m_OnDuckPressed, "OnDuckPressed"),

END_DATADESC()

LINK_ENTITY_TO_CLASS(point_overmovcam, COverlordMovCam);


COverlordMovCam::COverlordMovCam()
{
	
}
void COverlordMovCam::Spawn()
{
	m_pData = GET_OVERLORD_DATA;

	if(!m_pData)
	{
		SetThink(NULL);
		SetNextThink(0.0f);
		m_pData = NULL;
		Warning("COverlordData does not exist!\n");
		return;
	}

	BaseClass::Spawn();

	SetThink(&COverlordMovCam::Think);

	SetNextThink(gpGlobals->curtime + MOVCAM_THINK);

}

void COverlordMovCam::Think()
{
	SetNextThink(gpGlobals->curtime + MOVCAM_THINK);

	// Moves the camera
	if(m_pData->IsInVehicle())
	{

		if( m_pData->GetCamera() == this )
		{
			CBasePlayer * pPlayer = m_pData->GetOverlord();

			if(!pPlayer)
				return;

			int buttons = pPlayer->m_nButtons;

			if(buttons & IN_FORWARD)
				MoveCamera(FORWARD);
			else if(buttons & IN_BACK)
				MoveCamera(BACK);
			else if(buttons & IN_LEFT)
				MoveCamera(LEFT);
			else if(buttons & IN_RIGHT)
				MoveCamera(RIGHT);
		}
	}
}


void COverlordMovCam::MoveCamera(const movetype_t type)
{
	QAngle angle;
	const QAngle * pAngle = NULL;

	// Assign pointer, this way we can manipulate
	// the angle more easily
	if(type == FORWARD || type == BACK)
		pAngle = &m_aForward;
	else if(type == LEFT || type == RIGHT)
		pAngle = &m_aSide;
	

	if(!pAngle)
		return;

	angle = *pAngle;

	// Assign correct angle
	if(type == BACK || type == RIGHT)
	{
		angle.x = pAngle->x + 180.0f;
		angle.y = pAngle->y + 180.0f;
		angle.z = pAngle->z + 180.0f;
	}

	Vector origin = GetAbsOrigin();
	Vector vDir;
	Vector endpos;
	AngleVectors(angle, &vDir);
	
	Vector move = origin + vDir * 3;

	trace_t tr;

	// Do not move if something is blocking
	UTIL_TraceLine(origin, move, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);
	/*if(tr.fraction != 1.0f)
	{
		move = origin + vDir;

		UTIL_TraceLine(origin, move, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if(tr.fraction != 1.0f)
		{
			return;
		}
	}*/
	
	endpos = tr.endpos;

	SetAbsOrigin(endpos);
}