//==============Overlord mod=====================
//	Sentry gun's rocket barrel
//	
//===============================================

#include "cbase.h"
#include "overlord_rocketbarrel.h"
#include "weapon_rpg.h"

BEGIN_DATADESC(COverlordRocketBarrel)

	DEFINE_KEYFIELD(m_flSpeed, FIELD_FLOAT, "RocketSpeed"),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_overrocketbarrel, COverlordRocketBarrel);

COverlordRocketBarrel::COverlordRocketBarrel()
{
}

COverlordRocketBarrel::~COverlordRocketBarrel()
{
}

void COverlordRocketBarrel::Fire(const Vector &forward)
{
	if(!CanFire())
		return;

	QAngle fireAngle;

	VectorAngles(forward, fireAngle);
	CMissile *pRocket = static_cast<CMissile*>(CBaseEntity::Create("rpg_missile", EyePosition() + forward * 4, fireAngle, this));

	if(!pRocket)
	{
		Warning("No rocket created!\n");
		return;
	}

	pRocket->DumbFire();
	pRocket->SetAbsVelocity(forward * GetSpeed());
	pRocket->SetDamage(GetDamage());
	pRocket->SetNextThink(gpGlobals->curtime + 0.1f);

	UpdateNextFire();
}

/*
void COverlordRocketBarrel::DoEffects(const Vector &forward)
{
}
*/