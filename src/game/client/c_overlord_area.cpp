//==============Overlord mod=====================
//	Client-side class of the area brush	
//
//===============================================

#include "cbase.h"
#include "c_overlord_area.h"

IMPLEMENT_CLIENTCLASS_DT( C_OverlordArea, DT_OverlordArea, COverlordArea)

	RecvPropString(RECVINFO(m_iszAreaName) ),

END_RECV_TABLE()


C_OverlordArea::C_OverlordArea()
{
}

C_OverlordArea::~C_OverlordArea()
{
}
