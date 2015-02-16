//==============Overlord mod=====================
//	Hacker's camera weapon
//===============================================

#ifndef H_C_OV_CAMERAWEAPON
#define H_C_OV_CAMERAWEAPON

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define COverlordCamWeap C_OverlordCamWeap
#endif

class COverlordCamWeap : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordCamWeap, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();


	DECLARE_ACTTABLE();


	COverlordCamWeap();
	virtual ~COverlordCamWeap();

	virtual void PrimaryAttack();
	virtual	void		  HandleFireOnEmpty() { PrimaryAttack(); };

	virtual void Drop(const Vector &vecVelocity)
	{
		RevertCamera();

		BaseClass::Drop(vecVelocity);
	}

	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo)
	{
		RevertCamera();

		return BaseClass::Holster(pSwitchingTo);
	}
	
	Vector GetCameraOrigin() const { return m_CameraOrigin.Get(); };
	QAngle GetCameraAngles() const;
private:
	void RevertCamera();
	CNetworkVector(m_CameraOrigin);
};

#endif