//==============Overlord mod=====================
//	Overlord's MOVING camera
//	
//===============================================

#ifndef H_MOVINGCAMERA
#define H_MOVINGCAMERA

#ifdef _WIN32
#pragma once
#endif

#include "overlord_camera.h"

class COverlordData;

enum movetype_t
{
	FORWARD = 0,
	BACK = 1,
	LEFT = 2,
	RIGHT = 3,
};

class COverlordMovCam : public COverlordCamera
{
public:
	DECLARE_CLASS(COverlordMovCam, COverlordCamera);
	DECLARE_DATADESC();

	COverlordMovCam();
	virtual ~COverlordMovCam() { };


	virtual void Spawn();
	virtual void Think();

	virtual void FireOnForward() { };
	virtual void FireOnBack() { };
	virtual void FireOnRight() { };
	virtual void FireOnLeft() { };
private:
	COverlordData * m_pData;

	virtual void MoveCamera(const movetype_t type);

	QAngle m_aForward;
	QAngle m_aSide;
};

#endif