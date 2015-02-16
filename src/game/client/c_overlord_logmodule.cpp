//==============Overlord mod=====================
//	Module entity
//===============================================

#include "cbase.h"
#include "c_overlord_logmodule.h"
#include "c_overlord_modulelist.h"
#include "hl2mp_gamerules.h"

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordLogicModule, DT_OverlordLogicModule )

BEGIN_NETWORK_TABLE( C_OverlordLogicModule, DT_OverlordLogicModule )

END_NETWORK_TABLE()

C_OverlordLogicModule::C_OverlordLogicModule()
{
}

C_OverlordLogicModule::~C_OverlordLogicModule()
{
}


