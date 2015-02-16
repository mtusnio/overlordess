//==============Overlord mod=====================
//	Class of the area brush	
//
//===============================================

#include "cbase.h"
#include "overlord_area.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(COverlordArea)

	DEFINE_KEYFIELD(m_iszAreaName, FIELD_STRING, "Areaname"),
	//DEFINE_ENTITYFUNC(AreaTouch),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(COverlordArea, DT_OverlordArea)

	SendPropStringT(SENDINFO(m_iszAreaName) ),

END_SEND_TABLE()


//LINK_ENTITY_TO_CLASS(func_overarea, COverlordArea);

COverlordArea::COverlordArea()
{
	
}

COverlordArea::~COverlordArea()
{

}

void COverlordArea::Spawn()
{

	BaseClass::Spawn();

	Precache();

	SetSolid(SOLID_BBOX);

	AddEffects(EF_NODRAW);

	AddSolidFlags( FSOLID_NOT_SOLID );

	AddSolidFlags( FSOLID_TRIGGER );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetModel(STRING(GetModelName()));

}

