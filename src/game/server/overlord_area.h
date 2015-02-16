//==============Overlord mod=====================
//	Class of the area brush	
//
//===============================================

#ifndef H_OV_AREA
#define H_OV_AREA

#ifdef _WIN32
#pragma once
#endif

class COverlordArea : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordArea, CBaseEntity);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	COverlordArea(void);
	virtual ~COverlordArea(void);

	void Spawn();

	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}


	const char * GetAreaName() const { return STRING(m_iszAreaName.Get()); };

private:
	CNetworkVar(string_t, m_iszAreaName);

};

#endif