//==============Overlord mod=====================
//	Base class for class weapons
//	
//===============================================
#if 0
#ifndef H_OV_BASECLASS
#define H_OV_BASECLASS

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define COverlordClassWeap C_OverlordClassWeap
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#define LABEL_SIZE 128
// This class is purely abstract, although it must have empty
// functions in order to implement inherited network table,
// always override those empty functions
class COverlordClassWeap : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordClassWeap, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordClassWeap();
	virtual ~COverlordClassWeap();

	virtual void		  SetPickupTouch() { BaseClass::SetPickupTouch(); SetTouch(&COverlordClassWeap::DefaultTouch); };
	virtual void		  DefaultTouch(CBaseEntity * pOther);

	// If we have more than one class weapon, use it in order
	// to mark which weapon should be used for the display
	// purposes. It can be easily manipulated to handle
	// multiple weapons
	virtual PlayerClass_t GetClassWeapon() const { return CLASS_DEFAULT; }; // Override!
	virtual bool		  IsMainClassWeapon() const { return true; };
	virtual bool		  IsClassWeapon() const { return true; };

	virtual const char * GetPrimaryMode() const { return "None"; }; // Override
	virtual const char * GetSecondaryMode() const { return "None"; }; // Override
};

#endif
#endif