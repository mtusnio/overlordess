//==============Overlord mod=====================
//	Server-side implementation of the hint entity
//  Proper methods are in client file
//===============================================

#include "cbase.h"


class COverlordHintEntity : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordHintEntity, CBaseEntity);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	COverlordHintEntity();
	virtual ~COverlordHintEntity();

	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	virtual bool IsHintEnabled() const { return m_bEnabled.Get(); };

	void InputDisableHint( inputdata_t &inputdata );
	void InputEnableHint( inputdata_t &inputdata );
	void InputToggleHint( inputdata_t &inputdata ) ;
private:
	CNetworkVar(string_t, m_Hint);
	CNetworkVar(int, m_Team);
	CNetworkVar(bool, m_bEnabled);
};

BEGIN_DATADESC(COverlordHintEntity)

	DEFINE_KEYFIELD(m_Hint, FIELD_STRING, "HintDisplayed"),
	DEFINE_KEYFIELD(m_Team, FIELD_INTEGER, "HintTeam"),

	DEFINE_INPUTFUNC( FIELD_VOID, "EnableHint", InputEnableHint ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableHint", InputDisableHint ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ToggleHint", InputToggleHint ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(over_hintentity, COverlordHintEntity);

IMPLEMENT_SERVERCLASS_ST(COverlordHintEntity, DT_OverlordHintEntity)

	SendPropStringT(SENDINFO(m_Hint)), 
	SendPropInt(SENDINFO(m_Team)),
	SendPropBool(SENDINFO(m_bEnabled)),

END_SEND_TABLE()

COverlordHintEntity::COverlordHintEntity()
{
	m_bEnabled = true;
}

COverlordHintEntity::~COverlordHintEntity()
{
}

void COverlordHintEntity::InputDisableHint(inputdata_t &inputdata)
{
	m_bEnabled = false;
}

void COverlordHintEntity::InputEnableHint(inputdata_t &inputdata)
{
	m_bEnabled = true;
}

void COverlordHintEntity::InputToggleHint(inputdata_t &inputdata)
{
	m_bEnabled = !m_bEnabled;
}