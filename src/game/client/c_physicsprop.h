//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_PHYSICSPROP_H
#define C_PHYSICSPROP_H
#ifdef _WIN32
#pragma once
#endif

#include "c_breakableprop.h"
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_PhysicsProp : public C_BreakableProp
{
	typedef C_BreakableProp BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_PhysicsProp();
	~C_PhysicsProp();

	virtual bool OnInternalDrawModel( ClientModelRenderInfo_t *pInfo );

	// Some helper methods for the traps etc.
	virtual bool CanHaveTrapsBuilt() const { return m_bCanBuildOn; };
	virtual bool IsConstrained() const { return m_bConstrained; };
protected:
	// Networked vars.
	bool m_bCanBuildOn;
	bool m_bAwake;
	bool m_bAwakeLastTime;
	bool m_bConstrained;
};

#endif // C_PHYSICSPROP_H 
