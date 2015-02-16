//==============Overlord mod=====================
//	Sentrygun controlled by the overlord.
//	Client-side
//===============================================

#include "cbase.h"
#include "c_overlord_remotesentry.h"
#include "over_popupsupport.h"
#include "hl2mp_gamerules.h"

IMPLEMENT_CLIENTCLASS_DT( C_OverlordSentry, DT_OverlordSentry, COverlordSentry)
	
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_OverlordSentry)

END_PREDICTION_DATA()

C_OverlordSentry::C_OverlordSentry()
{
}

C_OverlordSentry::~C_OverlordSentry()
{
}
