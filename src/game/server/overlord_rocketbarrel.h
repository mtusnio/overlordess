//==============Overlord mod=====================
//	Sentry gun's rocket barrel
//	
//===============================================

#ifndef H_OV_ROCKET_BARREL
#define H_OV_ROCKET_BARREL


#include "overlord_barrel.h"

// Main class
class COverlordRocketBarrel : public COverlordBarrel
{
public:
	DECLARE_CLASS(COverlordRocketBarrel, COverlordBarrel);
	DECLARE_DATADESC();

	COverlordRocketBarrel();
	virtual ~COverlordRocketBarrel();

	virtual void Fire(const Vector &forward);

	float GetSpeed() const { return m_flSpeed; };
private:
	//virtual void DoEffects(const Vector &forward);

	float m_flSpeed;
};

#endif