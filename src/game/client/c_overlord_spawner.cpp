//==============Overlord mod=====================
//	Player spawner
//	
//===============================================

#include "cbase.h"
#include "c_overlord_spawner.h"
#include "hl2mp_gamerules.h"
#include "c_team.h"

BEGIN_DATADESC(C_OverlordSpawner)

END_DATADESC()

IMPLEMENT_CLIENTCLASS_DT( C_OverlordSpawner, DT_OverlordSpawner, COverlordSpawner)
	//RecvPropInt(RECVINFO(m_iUses)),
	RecvPropBool(RECVINFO(m_bAlreadyMarked)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_OverlordSpawner)

END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(over_spawner, C_OverlordSpawner);

C_OverlordSpawner::C_OverlordSpawner()
{
}

C_OverlordSpawner::~C_OverlordSpawner()
{
}


int C_OverlordSpawner::GetUses() const
{
	return  ((GET_OVERLORD_DATA)->GetSpawnsAmount() >= 1) ? (GET_OVERLORD_DATA)->GetSpawnsAmount() : 0;
}
