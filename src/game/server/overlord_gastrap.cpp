//==============Overlord mod=====================
//	Gas trap
//===============================================

#include "cbase.h"
#include "overlord_gastrap.h"
#include "soundenvelope.h"

#define GAS_THINK 0.5f
#define GAS_SOUND "coast.leech_bites_loop"

BEGIN_DATADESC(COverlordGasTrap)

//	DECLARE_MODULE(),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_overgas, COverlordGasTrap);

COverlordGasTrap::COverlordGasTrap()
{

}


COverlordGasTrap::~COverlordGasTrap()
{
}

void COverlordGasTrap::Spawn()
{
	BaseClass::Spawn();

	Precache();

	SetSolid(SOLID_BBOX);

	AddSolidFlags( FSOLID_NOT_SOLID );

	AddSolidFlags( FSOLID_TRIGGER );

#ifndef OVER_DEBUG
	AddEffects( EF_NODRAW );
#endif
	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetModel(STRING(GetModelName()));
}


void COverlordGasTrap::Activate()
{
	BaseClass::Activate();

}

void COverlordGasTrap::RunModule()
{

}

void COverlordGasTrap::Think()
{
	BaseClass::Think();

	SetNextThink(gpGlobals->curtime + GAS_THINK);
}

void COverlordGasTrap::StartTouch(CBaseEntity * pOther)
{
	if(!pOther->IsPlayer())
		return;

	CBasePlayer * pPlayer = static_cast<CBasePlayer*>(pOther);

	int effect = 43;
	CSingleUserRecipientFilter user(pPlayer);
	enginesound->SetPlayerDSP(user, effect, false );
}

void COverlordGasTrap::EndTouch(CBaseEntity * pOther)
{
}