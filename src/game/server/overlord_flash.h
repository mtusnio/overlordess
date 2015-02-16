//==============Overlord mod=====================
//	Flash that blinds players
//	
//===============================================

#ifndef H_OV_FLASH
#define H_OV_FLASH

#include "overlord_basemodule.h"

#ifdef _WIN32
#pragma once
#endif

class CSprite;

class COverlordFlash : public COverlordBaseModule
{
public:
	DECLARE_CLASS(COverlordFlash, COverlordBaseModule);
	DECLARE_DATADESC();

	COverlordFlash();
	virtual ~COverlordFlash();
	void Spawn();
	void Activate();

	void Precache();

	void Think();

	void InputActivate(inputdata_t &inputData);

	virtual void Deactivate();
protected:
	virtual void RunModule();

private:
	void Flash(CBasePlayer * pPlayer);

	CSprite * m_pFlash;

	char * m_szSoundOn;
	char * m_szSoundOff;
	float m_flRadius;
	byte m_Alpha;
};


#endif