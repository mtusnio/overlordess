//==============Overlord mod=====================
//	Client-side declaration of the overlord's
//	console
//===============================================

#include "cbase.h"
#include "c_overlord_console.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



IMPLEMENT_CLIENTCLASS_DT( C_OverlordConsole, DT_OverlordConsole, COverlordConsole)
	RecvPropBool(RECVINFO(m_bEnabled)),
	RecvPropString(RECVINFO(m_iszArea)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_OverlordConsole)

END_PREDICTION_DATA()

C_OverlordConsole::C_OverlordConsole()
{

}

C_OverlordConsole::~C_OverlordConsole()
{

}

