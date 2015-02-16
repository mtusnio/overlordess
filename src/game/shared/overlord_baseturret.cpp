//==============Overlord mod=====================
//	Base variables for the turrets, has basic 
//	rotation functions
//===============================================

#include "cbase.h"
#include "overlord_baseturret.h"

#ifndef CLIENT_DLL
#include "overlord_barrel.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


BEGIN_DATADESC(COverlordBaseTurret)

END_DATADESC()

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordBaseTurret, DT_OverlordBaseTurret)

BEGIN_NETWORK_TABLE( COverlordBaseTurret, DT_OverlordBaseTurret)

#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bRelPitch)),
	SendPropBool(SENDINFO(m_bRelYaw)),

	SendPropFloat(SENDINFO(m_flMoveSpeed)),
	SendPropFloat(SENDINFO(m_iPitch)),
	SendPropFloat(SENDINFO(m_iYaw)),
	SendPropFloat(SENDINFO(m_flDist)),

	SendPropVector(SENDINFO(m_aMaximum)),
	SendPropVector(SENDINFO(m_aMinimum)),
	SendPropVector(SENDINFO(m_aInit)),
#else
	RecvPropBool(RECVINFO(m_bRelPitch)),
	RecvPropBool(RECVINFO(m_bRelYaw)),

	RecvPropFloat(RECVINFO(m_flMoveSpeed)),
	RecvPropFloat(RECVINFO(m_iPitch)),
	RecvPropFloat(RECVINFO(m_iYaw)),
	RecvPropFloat(RECVINFO(m_flDist)),

	RecvPropVector(RECVINFO(m_aMaximum)),
	RecvPropVector(RECVINFO(m_aMinimum)),
	RecvPropVector(RECVINFO(m_aInit)),
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordBaseTurret)

END_PREDICTION_DATA()
#endif

COverlordBaseTurret::COverlordBaseTurret()
{
	m_bRelYaw = false;
	m_bRelPitch = false;
}

void COverlordBaseTurret::ResetTurret()
{
	SetLocalAngles(m_aInit);
}

void COverlordBaseTurret::ResolveNames()
{
#ifndef CLIENT_DLL
	CBaseEntity * pEnt = NULL;
	int i;
	for(i=0; i < MAX_BARRELS; i++)
	{
		if(m_iszBarrelName[i])
		{
			pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszBarrelName[i]);

			if(!pEnt)
			{
				m_pBarrel[i] = NULL;
				m_iszBarrelName[i] = NULL;
				continue;
			}

			m_pBarrel[i] = dynamic_cast<COverlordBarrel*>(pEnt);

			if(!m_pBarrel[i])
			{
				m_pBarrel[i] = NULL;
				m_iszBarrelName[i] = NULL;
				continue;
			}
		}
		else
		{
			m_pBarrel[i] = NULL;
			continue;
		}
	}
#endif
}

void COverlordBaseTurret::CreateBoundries()
{
#ifndef CLIENT_DLL
	// Maximum boundries first
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, GetMaxPitch()/2, PITCH_AXIS);
		RotateAngle(yaw, GetMaxYaw()/2, YAW_AXIS);

		m_aMaximum.GetForModify().x = (pitch.x > yaw.x) ? pitch.x : yaw.x;
		m_aMaximum.GetForModify().y = (pitch.y > yaw.y) ? pitch.y : yaw.y;
		m_aMaximum.GetForModify().z = (pitch.z > yaw.z) ? pitch.z : yaw.z;
	}
	// Minimum boundries
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, GetMaxPitch()/2, -PITCH_AXIS);
		RotateAngle(yaw, GetMaxYaw()/2, -YAW_AXIS);

		m_aMinimum.GetForModify().x = (pitch.x < yaw.x) ? pitch.x : yaw.x;
		m_aMinimum.GetForModify().y = (pitch.y < yaw.y) ? pitch.y : yaw.y;
		m_aMinimum.GetForModify().z = (pitch.z < yaw.z) ? pitch.z : yaw.z;
	}

	/*const QAngle tempMax = m_aMaximum;
	const QAngle tempMin = m_aMinimum;

	m_aMaximum.x = (tempMax.x > tempMin.x) ? tempMax.x : tempMin.x;
	m_aMaximum.y = (tempMax.y > tempMin.y) ? tempMax.y : tempMin.y;
	m_aMaximum.z = (tempMax.z > tempMin.z) ? tempMax.z : tempMin.z;

	m_aMinimum.x = (tempMax.x < tempMin.x) ? tempMax.x : tempMin.x;
	m_aMinimum.y = (tempMax.y < tempMin.y) ? tempMax.y : tempMin.y;
	m_aMinimum.z = (tempMax.z < tempMin.z) ? tempMax.z : tempMin.z;*/
#endif
}

void COverlordBaseTurret::RotateLeft()
{
	RotateTurret(BASETURRET_LEFT, GetYawSpeed(), YAW_AXIS);
}

void COverlordBaseTurret::RotateRight()
{
	RotateTurret(BASETURRET_RIGHT, -GetYawSpeed(), YAW_AXIS);
}

void COverlordBaseTurret::MoveUp()
{
	RotateTurret(BASETURRET_UP, -GetPitchSpeed(), PITCH_AXIS);
}

void COverlordBaseTurret::MoveDown()
{
	RotateTurret(BASETURRET_DOWN, GetPitchSpeed(), PITCH_AXIS);
}

// Rotates around selected axis and returns the angle
void COverlordBaseTurret::RotateAngle(QAngle & angle, float flAngle, const Vector & axis) const
{
	// Rotation axis
	Vector rotaxis = axis;

	///////////////////
	// Now the rotation
	///////////////////
	Quaternion q;
	
	// Rotate the quaternion and create a matrix from it
	AxisAngleQuaternion(rotaxis, flAngle, q);
	matrix3x4_t matrix;
	QuaternionMatrix(q, vec3_origin, matrix);

	// Local matrix
	matrix3x4_t localToWorld;

	// Make local or world transformation
	ConcatTransforms(EntityToWorldTransform(), matrix, localToWorld);

	// Local angles
	MatrixAngles(localToWorld, angle);
}

bool COverlordBaseTurret::IsWithinBounds(const QAngle & angle) const
{
	float yawdist = AngleDistance(angle.y, m_aInit.Get().y);
	float pitchdist = AngleDistance(angle.x, m_aInit.Get().x);

	
	if(yawdist < 0)
		yawdist = -yawdist;
	if(pitchdist < 0)
		pitchdist = -pitchdist;

	//DevMsg("yaw: %f, pitch: %f\n", yawdist, pitchdist);
	if(yawdist >= (float)(GetMaxYaw()/2))
		return false;
	
	if(pitchdist >= (float)(GetMaxPitch()/2))
		return false;

	return true;

	
}
