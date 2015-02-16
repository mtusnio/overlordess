//==============Overlord mod=====================
//	Overlord's teleport weapon
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "overlord_teleport.h"
#include "hl2mp_gamerules.h"
#include "particle_parse.h"

#ifndef CLIENT_DLL
#include "ilagcompensationmanager.h"
#include "client.h"
#endif

ConVar eo_teleport_cost("eo_teleport_cost", "110", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_teleport_jump_distance("eo_teleport_jump_distance", "472.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_teleport_jump_time("eo_teleport_jump_time", "5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_teleport_cost_primary("eo_teleport_cost_primary", "100", FCVAR_REPLICATED | FCVAR_CHEAT);

#define POWER_DRAIN eo_teleport_cost.GetInt()
#define JUMP_DISTANCE eo_teleport_jump_distance.GetFloat()
#define JUMP_TIME eo_teleport_jump_time.GetFloat()
#define TELEPORT_PARTICLE "TeleportEffect"
#define TELEPORT_SOUND "Weapon_Teleport.Teleport"
//#define WARNING_SOUND ""

#define FAIL_SOUND "WallHealth.Deny"
#define DENY_SOUND "SuitRecharge.Deny"

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordTeleport, DT_OverlordTeleport)

BEGIN_NETWORK_TABLE(COverlordTeleport, DT_OverlordTeleport)
#ifndef CLIENT_DLL
	SendPropEHandle(SENDINFO(m_Target)),
	SendPropBool(SENDINFO(m_bState)),
	SendPropFloat(SENDINFO(m_flJumpTime)),
#else
	RecvPropEHandle(RECVINFO(m_Target)),
	RecvPropBool(RECVINFO(m_bState)),
	RecvPropFloat(RECVINFO(m_flJumpTime)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordTeleport)


END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_teleport, COverlordTeleport);
PRECACHE_WEAPON_REGISTER(weapon_teleport);

acttable_t COverlordTeleport::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PISTOL,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PISTOL,					false },
};

IMPLEMENT_ACTTABLE(COverlordTeleport);

COverlordTeleport::COverlordTeleport()
{
	m_Target = NULL;
	m_bState = false;
	m_flJumpTime = 0.0f;
}

COverlordTeleport::~COverlordTeleport()
{
	if(m_Target.Get() || m_bState.Get())
		Revert();
}

void COverlordTeleport::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(TELEPORT_PARTICLE);
	PrecacheScriptSound(TELEPORT_SOUND);
	PrecacheScriptSound(FAIL_SOUND);
	PrecacheScriptSound(DENY_SOUND);
}

void COverlordTeleport::PrimaryAttack()
{
#ifndef CLIENT_DLL
	if(GetState())
		return;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	
	if(!pPlayer)
		return;

	COverlordData * pData = GET_OVERLORD_DATA;

	if(pData->GetPower() < eo_teleport_cost_primary.GetInt())
	{
		CSingleUserRecipientFilter filter(pPlayer);
		pPlayer->EmitSound(filter, pPlayer->entindex(), DENY_SOUND);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
		return;
	}

	bool bTeleported = Teleport();

	if(bTeleported)
	{
		pPlayer->EmitSound(TELEPORT_SOUND);
		DispatchParticleEffect(TELEPORT_PARTICLE, pPlayer->WorldSpaceCenter(), pPlayer->GetAbsAngles(), pPlayer);
		pData->HandlePowerEvent(EVENT_WEAPONUSED, eo_teleport_cost_primary.GetInt());
	}
	else
	{
		CSingleUserRecipientFilter filter(pPlayer);
		pPlayer->EmitSound(filter, pPlayer->entindex(), DENY_SOUND);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.3f;
		return;
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
#endif
}

void COverlordTeleport::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	GET_OVERLORD_DATA_ASSERT(pData);

	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

	if(!m_Target.Get())
	{
		if(pData->GetPower() < POWER_DRAIN)
		{
			CSingleUserRecipientFilter filter(pOwner);
			pOwner->EmitSound(filter, pOwner->entindex(), DENY_SOUND);
			return;
		}

#ifndef CLIENT_DLL
		CHL2MP_Player * pCompensated = ToHL2MPPlayer( GetPlayerOwner() );
		// Move other players back to history positions based on local player's lag
		lagcompensation->StartLagCompensation( pCompensated, pCompensated->GetCurrentCommand() );
#endif

		Vector end, forw;
		AngleVectors(pOwner->GetAbsAngles(), &forw);

		VectorMA(pOwner->RealEyePosition(), MAX_TRACE_LENGTH, forw, end);

		trace_t tr;
		UTIL_TraceLine(pOwner->RealEyePosition(), end, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tr);

		// Player hit!
		CBasePlayer * pPlayer = static_cast<CBasePlayer*>(tr.m_pEnt);
		if(pPlayer && pPlayer->IsRebel())
		{
#ifndef CLIENT_DLL
			m_Target = pPlayer;
			m_bState = true;
			pOwner->AddFlag(FL_ATCONTROLS);
			pOwner->AddEffects(EF_NODRAW);
			pOwner->MakeInvisible();
			pData->HandlePowerEvent(EVENT_WEAPONUSED, POWER_DRAIN);		
#endif
			pOwner->m_takedamage = DAMAGE_NO;
			m_flJumpTime = gpGlobals->curtime;
		}
		else
		{
			CSingleUserRecipientFilter filter(pOwner);
			EmitSound(filter, pOwner->entindex(), FAIL_SOUND);
		}

#ifndef CLIENT_DLL
		// Move other players back to history positions based on local player's lag
		lagcompensation->FinishLagCompensation( pCompensated );
#endif
	}
	else
	{
		Jump();	
		m_flNextSecondaryAttack = gpGlobals->curtime + 5.0f;
	}
}

void COverlordTeleport::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

#ifdef CLIENT_DLL
	if(m_bState.Get() && m_Target.Get())
	{
		if(!(m_Target.Get()->GetEffects() & EF_NODRAW))
			m_Target.Get()->AddEffects(EF_NODRAW);
	}
#endif
	if(m_bState.Get() && (!m_Target.Get() || !m_Target.Get()->IsAlive() || !m_Target.Get()->IsRebel()))
	{
		Revert();
	}

	if((m_flJumpTime + JUMP_TIME) <= gpGlobals->curtime && m_bState.Get())
		Jump();
}

bool COverlordTeleport::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if(GetState())
	{
		return false;
	}

	return BaseClass::Holster(pSwitchingTo);
}

void COverlordTeleport::Jump()
{
#ifndef CLIENT_DLL
	CBasePlayer * pOwner = ToBasePlayer(GetOwner());
	CBasePlayer * pPlayer = m_Target.Get();

	if(!pOwner || !pPlayer)
	{
		Revert();
		return;
	}

	// Jump out

	// Set up multiple dirs to calculate our jump
	Vector dirs[10], vNull;
	AngleVectors(pPlayer->GetAbsAngles(), &dirs[2], &dirs[3], &vNull);

	dirs[0] = -dirs[2];
	dirs[1] = -dirs[3];

	for(int i = 4; i < ARRAYSIZE(dirs); i++)
	{
		dirs[i].Random(0.0f, 1.0f);
	}

	// Normalize just in case
	for(int i = 0; i < ARRAYSIZE(dirs); i++)
		VectorNormalize(dirs[i]);

	for(int i = 0; i < ARRAYSIZE(dirs); i++)
	{
		Vector end;
		Vector & dir = dirs[i];

		VectorMA(pPlayer->RealEyePosition(), JUMP_DISTANCE, dir, end);

		trace_t tr;
		UTIL_TraceEntity(pPlayer, pPlayer->RealEyePosition(), end, MASK_SHOT_HULL, &tr);

		if(tr.startsolid)
			continue;

		// Check whether it's a valid location
		if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
			continue;

		if((tr.endpos - pPlayer->WorldSpaceCenter()).Length() < 96)
			continue;

		// Check for fall damage
		Vector endDown;
		VectorMA(tr.endpos, MAX_TRACE_LENGTH, Vector(0, 0, -1), endDown);

		trace_t trDown;
		UTIL_TraceLine(tr.endpos, endDown, MASK_SHOT_HULL, NULL, COLLISION_GROUP_NONE, &trDown);

		if((trDown.endpos - tr.endpos).Length() > 282)
			continue;

		Vector endPos = ((trDown.endpos - tr.endpos).Length() < 142) ? (trDown.endpos + (trDown.plane.normal * 34))
			: (tr.endpos + (tr.plane.normal * 34));


		// Make sure we aim at the person
		QAngle teleportAngle;
		Vector teleportVector = pPlayer->GetAbsOrigin() - endPos; 
		VectorNormalize(teleportVector);

		VectorAngles(teleportVector, teleportAngle);

		// Teleport (literally)

		pOwner->Teleport(&endPos, &teleportAngle, &vec3_origin);
		DispatchParticleEffect(TELEPORT_PARTICLE, endPos, teleportAngle, pPlayer);
		pOwner->EmitSound(TELEPORT_SOUND);
		break;
	}

	
#endif

	Revert();

#ifndef CLIENT_DLL
	// Switch to rifle
	pOwner->Weapon_Switch( pOwner->Weapon_OwnsThisType( "weapon_rifle" ) );
#endif
}
void COverlordTeleport::Revert()
{
	CBasePlayer * pOwner = ToBasePlayer(GetOwner());

	if(!pOwner)
		return;

#ifndef CLIENT_DLL
	pOwner->UnmakeInvisible();
	pOwner->RemoveEffects(EF_NODRAW);
#endif
	pOwner->RemoveFlag(FL_ATCONTROLS);

	pOwner->m_takedamage = DAMAGE_YES;
	if(m_Target.Get())
	{
#ifdef CLIENT_DLL
		m_Target.Get()->RemoveEffects(EF_NODRAW);
#endif
	}

#ifndef CLIENT_DLL
	m_Target = NULL;
	m_bState = false;
#endif

	m_flJumpTime = 0.0f;
}

bool COverlordTeleport::Teleport(Vector vTeleport /* = vec3_origin*/)
{
#ifndef CLIENT_DLL
	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return false;

	// If no vector specified we need to find it ourselves
	if(vTeleport == vec3_origin)
	{
		Vector forw, end;
		AngleVectors(EyeAngles(), &forw);
		VectorMA(pPlayer->Weapon_ShootPosition(), MAX_TRACE_LENGTH, forw, end);

		trace_t tr;
		UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), end, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr);

		if(tr.surface.flags & SURF_SKY)
			return false;

		vTeleport = tr.endpos;
	}

	Vector oldOrigin = pPlayer->GetAbsOrigin();
	Vector newOrigin;

	Vector dir = vTeleport - pPlayer->Weapon_ShootPosition();
	VectorNormalize(dir);

	// Now, move a player there and find a spot
	pPlayer->SetAbsOrigin(vTeleport);

	if(!FindPassableSpace(pPlayer, -dir, 1, newOrigin, 164))
	{
		// Check whether there's something below we can teleport to
		Vector vDown = vTeleport + Vector(0, 0, -1) * MAX_TRACE_LENGTH;
	
		trace_t tr;
		UTIL_TraceLine(vTeleport, vDown, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr);

		pPlayer->SetAbsOrigin(tr.endpos);
		if(!FindPassableSpace(pPlayer, -dir, 1, newOrigin, 164))
		{
			pPlayer->SetAbsOrigin(tr.endpos);
			if(!FindPassableSpace(pPlayer, Vector(0, 0, 1), 1, newOrigin, 32))
			{
				pPlayer->SetAbsOrigin(oldOrigin);
				return false;
			}
		}
	}

	// Move now completely
	QAngle angles;
	dir.z = 0.0f;
	VectorNormalize(dir);
	VectorAngles(-dir, angles);
	pPlayer->SetAbsOrigin(newOrigin);
	pPlayer->SetAbsAngles(angles);
#endif
	return true;
}



const char * COverlordTeleport::GetCostLabel() const
{
	static char label[16];
	Q_snprintf(label, ARRAYSIZE(label), "%i/%i", eo_teleport_cost_primary.GetInt(), eo_teleport_cost.GetInt());

	return label;
}