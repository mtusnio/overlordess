//==============Overlord mod=====================
//	Client data proxy
//===============================================

#include "cbase.h"
#include "c_overlord_data_proxy.h"

IMPLEMENT_CLIENTCLASS_DT(C_OverlordDataProxy, DT_OverlordDataProxy, COverlordDataProxy)

	RecvPropFloat(RECVINFO(m_flSecondsPerHealth)),
	RecvPropFloat(RECVINFO(m_flSpawnMultiplier)),
	RecvPropInt(RECVINFO(m_iStartHealth)),
	RecvPropInt(RECVINFO(m_iHealthCap)),
	RecvPropInt(RECVINFO(m_iMaxTraps)),

END_RECV_TABLE()

C_OverlordDataProxy::C_OverlordDataProxy()
{
}

C_OverlordDataProxy::~C_OverlordDataProxy()
{
}