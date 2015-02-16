//==============Overlord mod=====================
//	Sentry gun's barrel
//	
//===============================================

#include "cbase.h"
#include "overlord_barrel.h"
#include "ammodef.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "hl2mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(COverlordBarrel)

	DEFINE_KEYFIELD(m_flIntermission, FIELD_FLOAT, "AttackIntermission"),
	DEFINE_KEYFIELD(m_flDistance, FIELD_FLOAT, "SentryDistance"),
	DEFINE_KEYFIELD(m_iDamage, FIELD_INTEGER, "SentryDamage"),
	DEFINE_KEYFIELD(m_iGenericDamage, FIELD_INTEGER, "PropDamage"),
	DEFINE_KEYFIELD(m_iszSoundName, FIELD_SOUNDNAME, "FireSound"),

	DEFINE_INPUTFUNC(FIELD_VOID, "Fire", InputFire),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_overbarrel, COverlordBarrel);

COverlordBarrel::COverlordBarrel()
{
}

COverlordBarrel::~COverlordBarrel()
{

}

void COverlordBarrel::Spawn()
{
	Precache();

	SetSolid(SOLID_VPHYSICS);

	SetMoveType(MOVETYPE_NONE);

	AddSolidFlags( FSOLID_NOT_SOLID );

	SetModel(STRING(GetModelName()));

	BaseClass::Spawn();

}

void COverlordBarrel::Precache()
{
	BaseClass::Precache();

	if(m_iszSoundName)
		PrecacheScriptSound(m_iszSoundName);
}

void COverlordBarrel::Fire(const Vector &forward)
{
	if(!CanFire())
		return;

	CAmmoDef *pAmmoDef = GetAmmoDef();
	int ammoType = pAmmoDef->Index("SMG1");

	FireBulletsInfo_t info;
	info.m_vecSrc = GetAbsOrigin();
	info.m_vecDirShooting = forward;
	info.m_flDistance = m_flDistance;
	info.m_iAmmoType = ammoType;
	info.m_iDamage = m_iGenericDamage;
	info.m_iPlayerDamage = m_iDamage;
	info.m_pAttacker = this;

	DoEffects(forward);
	FireBullets(info);

	UpdateNextFire();
}

void COverlordBarrel::FireToLocation(const Vector &location)
{
	Vector forw = location - GetAbsOrigin();
	
	VectorNormalize(forw);
	Fire(forw);
}

bool COverlordBarrel::CanFire() const
{
	return (m_flNextAttack <= gpGlobals->curtime);
}

CBasePlayer * COverlordBarrel::GetScorer()
{
	return (GET_OVERLORD_DATA)->GetOverlord();
}

void COverlordBarrel::DoEffects(const Vector &forward)
{
	CEffectData data;
    data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = GetParentAttachment();
	data.m_flScale = 1.0f;
	data.m_fFlags = MUZZLEFLASH_COMBINE_TURRET;

	DispatchEffect("MuzzleFlash", data);

	if(m_iszSoundName)
		EmitSound(m_iszSoundName);

	Vector vecend;
	trace_t tr;

	VectorMA(GetAbsOrigin(), m_flDistance, forward, vecend);
	UTIL_TraceLine(GetAbsOrigin(), vecend, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	UTIL_Tracer( GetAbsOrigin(), tr.endpos );

	UTIL_ImpactTrace(&tr, DMG_BULLET);
}

void COverlordBarrel::InputFire(inputdata_t &inputData)
{
	Vector forward;
	AngleVectors(GetAbsAngles(), &forward);

	Fire(forward);
}