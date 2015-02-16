//==============Overlord mod=====================
//	Module entity
//===============================================

#include "cbase.h"
#include "overlord_logmodule.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MODULE_THINK 0.2f
BEGIN_DATADESC(COverlordLogicModule)

//	DECLARE_MODULE(),

END_DATADESC()

LINK_ENTITY_TO_CLASS(logic_overmodule, COverlordLogicModule);


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordLogicModule, DT_OverlordLogicModule )

BEGIN_NETWORK_TABLE(COverlordLogicModule, DT_OverlordLogicModule)

END_NETWORK_TABLE()

COverlordLogicModule::COverlordLogicModule()
{

}

COverlordLogicModule::~COverlordLogicModule()
{

}

/*void COverlordLogicModule::InputActivate(inputdata_t &inputData)
{
	BaseClass::InputActivate(inputData);
}

void COverlordLogicModule::Deactivate()
{
	BaseClass::Deactivate();
}*/

