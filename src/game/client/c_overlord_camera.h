//==============Overlord mod=====================
//	Finally a client-side overlord camera 
//	(hooray!)
//===============================================

#ifndef C_H_OV_CAMERA
#define C_H_OV_CAMERA


#include "c_overlord_modulelist.h"
#include "c_overlord_basemodule.h"
#include "overlord_baseturret.h"

struct sTrapDescription;

#define NUM_OF_LISTS 8

#define BACKGROUND_ALPHA 120

#define SF_NOSPEEDCHANGES 256

#define CAMERA_THINK 0.02f

class C_OverlordArea;

class C_OverlordCamera : public C_OverlordBaseTurret
{
public:
	DECLARE_CLASS(C_OverlordCamera, C_OverlordBaseTurret);

	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();
	DECLARE_CLIENTCLASS();

	C_OverlordCamera();
	virtual ~C_OverlordCamera();

	virtual void Spawn();

	//virtual bool  ShouldPredict() { return true; };
	virtual int   DrawModel(int flags);

	virtual void  ClientThink();

	//virtual bool IsPredicted() { return true; };

	virtual Vector GetCameraVector() const 
	{
		Vector forw;
		AngleVectors(GetCameraAngles(), &forw);

		if(m_vOffset.Length() == 0.0f)
		{
			return GetLocalOrigin() + forw * 4; 
		}

		float dist = m_vOffset.Length();

		return (GetLocalOrigin() + forw * dist);
	};
	virtual QAngle GetCameraAngles() const { return EyeAngles(); };

	virtual C_OverlordBaseModule * GetModule(int i) const
	{
		if(!GetCurrentList())
			return NULL;

		return GetCurrentList()->GetModule(i);
	}

	virtual sTrapDescription GetButtonRecord(int i) const;
	virtual const char *     GetButtonCaption(int i) const;
	virtual bool			 IsButtonVisible(int i) const
	{
		if(GetModule(i))
			return true;
		else
			return false;
	}

	virtual const char *     GetDescription() const { return m_iszDesc; };

	virtual C_OverlordModuleList * GetCurrentList() const { return m_lLists[GetCurrentListNumber()]; };
	virtual int GetCurrentListNumber() const { return m_CurrentList; };
	virtual C_OverlordModuleList * GetList(int num) const { return m_lLists[num]; };

	virtual C_OverlordCamera * GetNextCamera() const { return m_pNext; };
	virtual C_OverlordCamera * GetPreviousCamera() const { return m_pPrevious; };
	virtual C_OverlordCamera * GetUpperCamera() const { return m_pUpper; };
	virtual C_OverlordCamera * GetLowerCamera() const { return m_pLower; };
	
	virtual Color GetButtonColor(int i) const;
	virtual C_OverlordArea * GetArea() const { return m_pArea; };

	virtual bool ShouldShowListButtons() const;

	virtual float GetYawSpeed() const { return C_OverlordCamera::GetMoveSpeed(); };
	virtual float GetPitchSpeed() const { return C_OverlordCamera::GetMoveSpeed(); };
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

		C_OverlordBaseTurret::RotateRight();
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

		C_OverlordBaseTurret::RotateLeft();
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

		C_OverlordBaseTurret::MoveUp();
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

		C_OverlordBaseTurret::MoveDown();
	}
private:
	CHandle<C_OverlordModuleList> m_lLists[NUM_OF_LISTS];
	CHandle<C_OverlordCamera> m_pNext;
	CHandle<C_OverlordCamera> m_pPrevious;
	CHandle<C_OverlordCamera> m_pUpper;
	CHandle<C_OverlordCamera> m_pLower;
	CHandle<C_OverlordArea> m_pArea;

	int m_spawnflags;
	Vector m_vOffset;

	char m_iszDesc[DESC_LENGTH];
	int m_CurrentList;

	float m_flFirstMove;
	float m_flLastMove;
};





#endif