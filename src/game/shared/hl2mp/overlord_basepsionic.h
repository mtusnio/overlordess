//######Overlord#######
//	Psychokinesis 
//	weapon
//#####################

#ifndef H_OV_BASEPSIONIC
#define H_OV_BASEPSIONIC

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CBasePsionic C_BasePsionic
#endif

class CBasePsionic : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(CBasePsionic, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual ~CBasePsionic() { };

	virtual void Drop(const Vector &vecVelocity);
	virtual void HandleFireOnEmpty(void) { PrimaryAttack(); };

	virtual void ItemPostFrame();

	virtual bool IsOverlordWeapon() { return true; };

protected:

#ifndef CLIENT_DLL
	void	DrainPower(const int power) const;
#endif
};

#endif