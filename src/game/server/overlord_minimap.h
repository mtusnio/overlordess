//==============Overlord mod=====================
//	Overlord's interactive minimap
//	
//===============================================

#ifndef H_OV_MINIMAP
#define H_OV_MINIMAP

#ifdef _WIN32
#pragma once
#endif

class COverlordMinimap : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordMinimap, CBaseEntity);
	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();

	COverlordMinimap() { };
	~COverlordMinimap() { };

	virtual void Spawn();

	virtual void Think();

	bool IsMinimapActivated() { return GetNextThink() != TICK_NEVER_THINK; };

	void InputEnable(inputdata_t &inputData) { 	EnableMinimap(); };

	void InputDisable(inputdata_t &inputData) { DisableMinimap(); }

private:
	void EnableMinimap() { SetNextThink(gpGlobals->curtime + m_flUpdate); };
	void DisableMinimap() { SetNextThink(TICK_NEVER_THINK); };

	float m_flUpdate;

	int m_iScale;
	int m_iSize;
};





#endif