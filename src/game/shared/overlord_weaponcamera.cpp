//==============Overlord mod=====================
//	Hacker's camera weapon
//===============================================

#include "cbase.h"
#include "overlord_weaponcamera.h"




IMPLEMENT_NETWORKCLASS_ALIASED( OverlordCamWeap, DT_OverlordCamWeap )

BEGIN_NETWORK_TABLE( COverlordCamWeap, DT_OverlordCamWeap )
#ifdef CLIENT_DLL
	RecvPropVector(RECVINFO(m_CameraOrigin), 0),
#else
	SendPropVector(SENDINFO(m_CameraOrigin), -1, SPROP_COORD),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COverlordCamWeap )

	//DEFINE_PRED_FIELD(m_CameraOrigin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_camera, COverlordCamWeap );

PRECACHE_WEAPON_REGISTER( weapon_camera );

acttable_t	COverlordCamWeap::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_CROSSBOW,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_CROSSBOW,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_CROSSBOW,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_CROSSBOW,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_CROSSBOW,					false },
};

IMPLEMENT_ACTTABLE(COverlordCamWeap);

COverlordCamWeap::COverlordCamWeap()
{
	m_CameraOrigin = vec3_origin;
}

COverlordCamWeap::~COverlordCamWeap()
{
	RevertCamera();
}

void COverlordCamWeap::PrimaryAttack()
{
	if(!GetOwner() || !GetOwner()->IsPlayer())
		return;
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(GetCameraOrigin() == vec3_origin)
	{

		WeaponSound(SINGLE, m_flNextPrimaryAttack);

		Vector forw, end;
		AngleVectors(pPlayer->GetAbsAngles(), &forw);
		VectorMA(pPlayer->EyePosition(), MAX_TRACE_LENGTH, forw, end);

		trace_t tr;
		UTIL_TraceLine(pPlayer->EyePosition(), end, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

		if(!(tr.surface.flags & SURF_SKY))
		{
			m_CameraOrigin = tr.endpos + tr.plane.normal * 16;

			// Disable viewmodel etc.
			pPlayer->ShowViewModel(false);
			pPlayer->SetDrawLocalModel(true);
		}
	}
	else
	{
		RevertCamera();
	}
#endif
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
}

QAngle COverlordCamWeap::GetCameraAngles() const
{
	if(m_CameraOrigin.Get() == vec3_origin)
		return vec3_angle;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return vec3_angle;

	Vector dir = pPlayer->RealEyePosition() - m_CameraOrigin.Get();
	VectorNormalize(dir);

	QAngle ang;
	VectorAngles(dir, ang);

	return ang;

}
void COverlordCamWeap::RevertCamera()
{
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;
	m_CameraOrigin = vec3_origin;

	pPlayer->ShowViewModel(true);
	pPlayer->SetDrawLocalModel(false);
#endif
}


