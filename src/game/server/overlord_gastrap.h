//==============Overlord mod=====================
//	Gas trap
//	
//===============================================


#ifndef H_OV_GASTRAP
#define H_OV_GASTRAP

#ifdef _WIN32
#pragma once
#endif

#include "overlord_basemodule.h"

class COverlordGasTrap : public COverlordBaseModule
{
public:
	DECLARE_CLASS(COverlordGasTrap, COverlordBaseModule);
	DECLARE_DATADESC();

	COverlordGasTrap();
	~COverlordGasTrap();

	virtual void Spawn();
	virtual void Activate();

	virtual void Think();

	virtual void StartTouch(CBaseEntity * pOther);
	virtual void EndTouch(CBaseEntity * pOther);
protected:
	virtual void RunModule();

private:

};





#endif