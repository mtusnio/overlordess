//==============Overlord mod=====================
//	Base variables for the turrets, has basic 
//	rotation functions
//===============================================

#include "cbase.h"
#include "overlord_baseturret.h"
#include "overlord_barrel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void COverlordBaseTurret::ResetTurret()
{
	m_pRotate->SetLocalAngles(m_aInit);
}

void COverlordBaseTurret::ResolveNames()
{
	m_pRotate = NULL;

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

	if(m_iszAngleParent)
	{
		pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszAngleParent);

		if(!pEnt)
		{
			m_iszAngleParent = NULL;
			pEnt = NULL;
		}
		else
		{
			m_pRotate = pEnt;
		}
	}



}

void COverlordBaseTurret::CreateBoundries()
{
	// Maximum boundries first
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, m_iPitch/2, PITCH_AXIS);
		RotateAngle(yaw, m_iYaw/2, YAW_AXIS);

		m_aMaximum.x = (pitch.x > yaw.x) ? pitch.x : yaw.x;
		m_aMaximum.y = (pitch.y > yaw.y) ? pitch.y : yaw.y;
		m_aMaximum.z = (pitch.z > yaw.z) ? pitch.z : yaw.z;
	}
	// Minimum boundries
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, m_iPitch/2, -PITCH_AXIS);
		RotateAngle(yaw, m_iYaw/2, -YAW_AXIS);

		m_aMinimum.x = (pitch.x < yaw.x) ? pitch.x : yaw.x;
		m_aMinimum.y = (pitch.y < yaw.y) ? pitch.y : yaw.y;
		m_aMinimum.z = (pitch.z < yaw.z) ? pitch.z : yaw.z;
	}

	/*const QAngle tempMax = m_aMaximum;
	const QAngle tempMin = m_aMinimum;

	m_aMaximum.x = (tempMax.x > tempMin.x) ? tempMax.x : tempMin.x;
	m_aMaximum.y = (tempMax.y > tempMin.y) ? tempMax.y : tempMin.y;
	m_aMaximum.z = (tempMax.z > tempMin.z) ? tempMax.z : tempMin.z;

	m_aMinimum.x = (tempMax.x < tempMin.x) ? tempMax.x : tempMin.x;
	m_aMinimum.y = (tempMax.y < tempMin.y) ? tempMax.y : tempMin.y;
	m_aMinimum.z = (tempMax.z < tempMin.z) ? tempMax.z : tempMin.z;*/
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
	ConcatTransforms(m_pRotate->EntityToWorldTransform(), matrix, localToWorld);

	// Local angles
	MatrixAngles(localToWorld, angle);
}

bool COverlordBaseTurret::IsWithinBounds(const QAngle & angle) const
{
	float yawdist = UTIL_AngleDistance(angle.y, m_aInit.y);
	float pitchdist = UTIL_AngleDistance(angle.x, m_aInit.x);

	
	if(yawdist < 0)
		yawdist = -yawdist;
	if(pitchdist < 0)
		pitchdist = -pitchdist;

	//DevMsg("yaw: %f, pitch: %f\n", yawdist, pitchdist);
	if(yawdist > (float)(GetMaxYaw()/2))
		return false;
	
	if(pitchdist > (float)(GetMaxPitch()/2))
		return false;

	return true;

	
}
