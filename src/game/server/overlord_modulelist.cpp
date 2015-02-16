//==============Overlord mod=====================
//	Contains list of all modules, allows switching
//	between different lists
//===============================================

#include "cbase.h"
#include "overlord_modulelist.h"
#include "overlord_basemodule.h"

BEGIN_DATADESC(COverlordModuleList)

	DEFINE_KEYFIELD(m_iszButtons[0], FIELD_STRING, "Module01"),     
	DEFINE_KEYFIELD(m_iszButtons[1], FIELD_STRING, "Module02"),     
	DEFINE_KEYFIELD(m_iszButtons[2], FIELD_STRING, "Module03"),     
	DEFINE_KEYFIELD(m_iszButtons[3], FIELD_STRING, "Module04"),     
	DEFINE_KEYFIELD(m_iszButtons[4], FIELD_STRING, "Module05"),     
	DEFINE_KEYFIELD(m_iszButtons[5], FIELD_STRING, "Module06"),     
	DEFINE_KEYFIELD(m_iszButtons[6], FIELD_STRING, "Module07"),
	DEFINE_KEYFIELD(m_iszButtons[7], FIELD_STRING, "Module08"),
	DEFINE_KEYFIELD(m_iszButtons[8], FIELD_STRING, "Module09"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(COverlordModuleList, DT_OverlordModuleList)

	SendPropArray3( SENDINFO_ARRAY3(m_Module), SendPropEHandle( SENDINFO_ARRAY(m_Module) ) ),

END_SEND_TABLE()

//LINK_ENTITY_TO_CLASS(overlord_list, COverlordModuleList);

COverlordModuleList::COverlordModuleList()
{
	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		m_Module.Set(i, NULL);
	}
}

COverlordModuleList::~COverlordModuleList()
{
}

void COverlordModuleList::Activate()
{
	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		CBaseEntity * pModule = gEntList.FindEntityByName(NULL, m_iszButtons[i]);

		if(!pModule || !dynamic_cast<COverlordBaseModule*>(pModule))
			continue;

		m_Module.Set(i, static_cast<COverlordBaseModule*>(pModule));
		

	}
}

void COverlordModuleList::FireOnModule(int module)
{
	if(m_Module[module]) 
		m_Module[module]->InputActivate(inputdata_t()); 
}