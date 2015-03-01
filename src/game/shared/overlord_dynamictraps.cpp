//==============Overlord mod=====================
//	Dynamic traps baseclass
//===============================================

#include "cbase.h"
#include "overlord_dynamictraps.h"
#include "overlord_dynamictraps_defenses.h"
#include "beam_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_team.h"
#include "fx_line.h"
#include "ClientEffectPrecacheSystem.h"
#include <vgui/IInput.h>
#include "input.h"
#include "fx.h"
#include "overlord_consolemanager.h"
#include "c_physicsprop.h"
#else
#include "props.h"
#include "team.h"
//#include "Fire.h"
#include "explode.h"
#include "particle_parse.h"
#include "player_pickup.h"
#include "sprite.h"
#include "tier0/vprof.h"
#include "movevars_shared.h"
#include "soundent.h"
#include "gib.h"
#include <tier1/KeyValues.h>
#endif

#include "hl2mp_gamerules.h"

// Initialize the static record table

#ifdef CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN( PrecacheTrapsFX )
CLIENTEFFECT_MATERIAL( TARGET_BEAM )
CLIENTEFFECT_REGISTER_END()


#else
#define GENERAL_THINK 0.03f

// Alpha lost per second
#define DECAY_COLOR_ALPHA 320

#define NEVER_RUN -1
#endif

// We need those to easily set model names without all this hassle
#define SET_TRAP_MODEL_FROM_VAR(modelName) Q_strncpy(m_szTrapModel, modelName, ARRAYSIZE(m_szTrapModel))
#define SET_TRAP_MODEL(modelName) Q_strncpy(m_szTrapModel, #modelName, ARRAYSIZE(m_szTrapModel))	

#define EXPLOSION_VOLUME 1024

#define ZAP_SPRITE "sprites/laserbeam.vmt"

#define MARK_SOUND "NPC_CombineBall.Impact"
#define MARK_BEAM "sprites/strider_bluebeam.vmt"
#define MARK_PARTICLE "MarkEffect"
#define DISABLER_PARTICLE "DisablerEffect"


// Entity message
#define MESSAGE_ZAP 1
#define MESSAGE_DISABLE 2
#define MESSAGE_ENABLE 3


ConVar eo_trapsneverdecay("eo_trapsneverdecay", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_trapsdistance("eo_trapsdistance", "225", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum distance between a trap and a player when building");
ConVar eo_traps_initial_dormant_range("eo_traps_initial_dormant_range", "1024.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_defaulthealth("eo_traps_defaulthealth", "140", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_repair_delay("eo_traps_repair_delay", "3.0", FCVAR_REPLICATED | FCVAR_CHEAT);

#ifndef CLIENT_DLL
ConVar eo_traps_explosion_magnitude("eo_traps_explosion_magnitude", "20", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_explosion_radius("eo_traps_explosion_radius", "128", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_explosion_force("eo_traps_explosion_force", "90", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_remark_time("eo_traps_remark_time", "21", FCVAR_REPLICATED | FCVAR_CHEAT);


ConVar eo_traps_discharge_delay_min("eo_traps_discharge_delay_min", "0.6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_discharge_delay_max("eo_traps_discharge_delay_max", "1.35", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_discharge_min("eo_traps_discharge_min", "10", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_discharge_max("eo_traps_discharge_max", "28", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_zap_damage_multiplier("eo_traps_zap_damage_multiplier", "30.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_zap_slowdown("eo_traps_zap_slowdown", "45", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_zap_slowdown_length("eo_traps_zap_slowdown_length", "1.10", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_defense_chance("eo_traps_defense_chance", "80", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_explode("eo_traps_explode", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_restore_duration_multiplier("eo_traps_restore_duration_multiplier", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_restore_health_multiplier("eo_traps_restore_health_multiplier", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_points("eo_traps_points", "4", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_points_per_kill("eo_traps_points_per_kill", "5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_points_invisible("eo_traps_points_invisible", "15", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_hacker_points("eo_hacker_points", "20", FCVAR_REPLICATED | FCVAR_CHEAT);

#endif

COverlordTrap::STrapRecord COverlordTrap::sTrapRecord[NUM_OF_TRAPS] = { };
int COverlordTrap::STrapRecord::usedIndex = 0;

#define PLAYER_RADIUS eo_trapsdistance.GetFloat()

//************************************************
//				Base class
//************************************************

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordTrap , DT_OverlordTrap )

BEGIN_NETWORK_TABLE( COverlordTrap , DT_OverlordTrap )

#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO( m_flSpawnTime ),0,SPROP_NOSCALE),
	SendPropFloat(SENDINFO( m_flSetDormant ),0,SPROP_NOSCALE),
	SendPropFloat(SENDINFO( m_flLifeAdd ),0,SPROP_NOSCALE),
	SendPropBool(SENDINFO(m_bIsDormant)),
	SendPropBool(SENDINFO(m_bInitialDormant)),
	SendPropInt(SENDINFO(m_iMaxHealth), 12 ),
	SendPropInt(SENDINFO(m_iHealth), 12 ),
	SendPropBool(SENDINFO(m_bEmitParticles)),
	SendPropBool(SENDINFO(m_bDecaying)),
	SendPropBool(SENDINFO(m_bDisabled)),
#else
	RecvPropFloat(RECVINFO( m_flSpawnTime)),
	RecvPropFloat(RECVINFO( m_flSetDormant)),
	RecvPropFloat(RECVINFO( m_flLifeAdd)),
	RecvPropBool(RECVINFO(m_bIsDormant)),
	RecvPropBool(RECVINFO(m_bInitialDormant)),
	RecvPropInt(RECVINFO(m_iMaxHealth)),
	RecvPropInt(RECVINFO(m_iHealth)),
	RecvPropBool(RECVINFO(m_bEmitParticles)),
	RecvPropBool(RECVINFO(m_bDecaying)),
	RecvPropBool(RECVINFO(m_bDisabled)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(COverlordTrap)
END_PREDICTION_DATA()


BEGIN_DATADESC(COverlordTrap)
#ifndef CLIENT_DLL
	//DEFINE_USEFUNC(TrapUse),
	//DEFINE_FUNCTION(DecayThink),
#endif
END_DATADESC()

COverlordTrap * COverlordTrap::EntityToTrap(CBaseEntity * pEnt)
{
	if(!pEnt)
		return NULL;

#ifndef CLIENT_DLL
	if ( strncmp( pEnt->GetClassname(), "trap_", 5 ) == 0 )
		return static_cast<COverlordTrap*>(pEnt);
#else
	/*for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, pEnt->GetClientClass()->GetName()))
			continue;

		return static_cast<COverlordTrap*>(pEnt);
	}*/

	// Just dynamic cast it... might be buggy otherwise
	return dynamic_cast<C_OverlordTrap*>(pEnt);
#endif
	return NULL;
}

COverlordTrap::COverlordTrap()
{
#ifndef CLIENT_DLL
	m_flNextRun = 0.0f;
	m_bDecaying = false;
	m_bDisabled = false;
	m_flTimeEnable = 0.0f;
	m_bExploding = false;
	m_bHasParent = false;
	m_flExplosionMagnitude = 0.0f;
	m_flExplosionRadius = 0.0f;
	m_flExplosionForce = 0.0f;
	m_bIsDormant = false;
	m_flLifeAdd = 0.0f;
	m_flSetDormant = 0.0f;
	m_bInitialDormant = false;
	m_EventSent = false;
	m_HandledInitialDormant = false;
	m_NextZap = 0;
	m_flZapDelay = 0.0f;
	m_flZapStart = 0.0f;
	m_vZapPoint = vec3_origin;
	m_bUseDefenses = true;
	m_LagTrack = new CUtlFixedLinkedList< LagRecordTrap >();
	m_TrapDefenseRef = 0;
	m_iKills = 0;
	m_flLastTimeMarked = 0.0f;
#else
	m_flBlinkDisable = 0.0f;
	m_BeamTexture = -1;
	m_DisableEffect = NULL;
	m_bOverrideGhost = false;
#endif
}

COverlordTrap::~COverlordTrap()
{
#ifdef CLIENT_DLL

#else
	if(m_LagTrack)
	{
		m_LagTrack->Purge();
		delete m_LagTrack;
	}

	if(m_RestoreData)
	{
		delete m_RestoreData;
	}

	if(m_ChangeData)
		delete m_ChangeData;
#endif
}

void COverlordTrap::Precache()
{ 
		BaseClass::Precache(); 
		PrecacheModel(m_szTrapModel); 
		PrecacheModel(MARK_BEAM);
#ifdef CLIENT_DLL
		//m_BeamTexture = PrecacheModel("sprites/lgtning.vmt");
		m_BeamTexture = PrecacheModel("sprites/physbeam.vmt");
		PrecacheModel(TARGET_BEAM);
		//PrecacheModel("sprites/physbeam.vmt");
		PrecacheParticleSystem(DISABLER_PARTICLE);
		PrecacheParticleSystem("ZapParticle");
#else
		PrecacheParticleSystem(EXPLOSION_PARTICLE);
		PrecacheParticleSystem("TrapImpact");
		PrecacheScriptSound("ambient.electrical_zap_1");
		PrecacheScriptSound(MARK_SOUND);
		PrecacheModel("models/gibs/metal_gib1.mdl");
		PrecacheModel("models/gibs/metal_gib2.mdl");
		PrecacheModel("models/gibs/metal_gib3.mdl");
		PrecacheModel("models/gibs/metal_gib4.mdl");
		PrecacheModel("models/gibs/metal_gib5.mdl");
#endif
};

void COverlordTrap::Spawn() 
{
		//BaseClass::Spawn();

		Precache();

		SetModel(m_szTrapModel);

		SetSolid(SOLID_VPHYSICS);

		SetMoveType(MOVETYPE_NONE);

		//SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);

		CreateVPhysics();

		m_flSpawnTime = gpGlobals->curtime;

#ifdef CLIENT_DLL
		m_BeamMat =  materials->FindMaterial(TARGET_BEAM, TEXTURE_GROUP_CLIENT_EFFECTS);

		
		if(!IsGhost())
		{
			InitializeTrap();
			ListenForGameEvent("trap_mark");
			ListenForGameEvent("trap_unmark");
		}
		SetNextClientThink(CLIENT_THINK_ALWAYS);
#else
		if(GetMoveParent())
			m_bHasParent = true;

		SetThink(&COverlordTrap::Think); 
		
		SetNextThink(gpGlobals->curtime + GENERAL_THINK);

		// Resolve initial dormant state
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		bool bSetDormant = true;
		if(CanBeDormant() && pRebels)
		{
			for(int i = 0; i < pRebels->GetNumPlayers(); i++)
			{
				CBasePlayer * pRebel = pRebels->GetPlayer(i);

				if(!pRebel)
					continue;

				float length = FLT_MAX;
				if(Use2DForInitialDormant())
					length = (pRebel->GetAbsOrigin() - GetAbsOrigin()).Length2D();
				else
					length = (pRebel->GetAbsOrigin() - GetAbsOrigin()).Length();

				if(length <= GetInitialDormantRange())
				{
					if(!UseLOSForInitialDormant())
					{
						bSetDormant = false;
						break;
					}
					else
					{
						trace_t tr;
						UTIL_TraceLine(GetEmitterPosition(), pRebel->EyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

						if(tr.m_pEnt == pRebel)
						{
							bSetDormant = false;
							break;
						}

						UTIL_TraceLine(GetEmitterPosition(), pRebel->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

						if(tr.m_pEnt == pRebel)
						{
							bSetDormant = false;
							break;
						}
					}
				}
			}

			if(pRebels->GetNumPlayers() <= 0)
				bSetDormant = false;
		}
		else
			bSetDormant = false;

		if(bSetDormant)
		{
			SetTrapDormant(true);
			m_bInitialDormant = true;
		}

		InitializeTrap();

		SelectNextZap();
		//SetOwnerEntity(GetOverlordData()->GetOverlord());
#endif


}

error_t COverlordTrap::CanBuild()
{
	COverlordData * pData = GET_OVERLORD_DATA;

	error_t error;
	error.canBuild = true;

	int cost = GetTrapCost();
	int power = pData->GetPower();
	if(cost > power)
	{
		error.canBuild = false;
		Q_snprintf(error.desc, ARRAYSIZE(error.desc), "To build this trap you need %i more energy.", cost - power);
		return error;
	}

	// Make sure we got free spots
	const int MAX_TRAPS = (GET_OVERLORD_DATA)->GetMaxTraps();
	for(int i = 0; i < MAX_TRAPS; i++)
	{
		// If no place for the trap is present, return error
		if(GET_OVERLORD_DATA->GetTrap(i) && (i < (MAX_TRAPS - 1)))
			continue;
		else if(GET_OVERLORD_DATA->GetTrap(i) && (i == (MAX_TRAPS - 1)))
		{
			Q_strncpy(error.desc, "Maximum number of traps has been reached.", ARRAYSIZE(error.desc));
			error.canBuild = false;
			return error;
		}

		break;
	}
	 
	
#ifndef CLIENT_DLL
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
#else
	C_Team * pRebels = GetGlobalTeam(TEAM_REBELS);
#endif
	if(!pRebels || pRebels->GetNumPlayers() <= 0)
		return error;

	const Vector trapPos = GetAbsOrigin();
	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);
		if(!pPlayer)
			continue;

		bool bError = false;
		float dist = (trapPos - pPlayer->GetAbsOrigin()).Length();
		if(dist <= eo_trapsdistance.GetFloat())
		{
			trace_t tr;
			UTIL_TraceLine(GetEmitterPosition(), pPlayer->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt == pPlayer)
				bError = true;
			else
			{
				UTIL_TraceLine(GetEmitterPosition(), pPlayer->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
				if(tr.m_pEnt == pPlayer)
					bError = true;
			}
		}

		if(bError)
		{
			error.canBuild = false;
			Q_strncpy(error.desc, "The trap is too close to rebels.", ARRAYSIZE(error.desc));
			return error;
		}
		//else if((trapPos - pPlayer->GetAbsOrigin()).Length() <= eo_trapsdistance.GetFloat())
		//	return false;
	}

	return error;
}

void COverlordTrap::SetTrapDormant(bool bDormant)
{
	m_bIsDormant = bDormant;
	
#ifndef CLIENT_DLL
	if(bDormant)
	{
		m_flSetDormant = gpGlobals->curtime;
	}
	else
	{
		m_bInitialDormant = false;
		// Figure out how long it's been dormant and add that time to the lifetime
		float add = gpGlobals->curtime - m_flSetDormant;
		m_flLifeAdd += add;
		m_flSetDormant = 0.0f;
	}

	IGameEvent * event = gameeventmanager->CreateEvent("trap_dormant");
	if(event)
	{
		event->SetInt("entindex", entindex());
		event->SetBool("dormant", bDormant);

		gameeventmanager->FireEvent(event);
	}
#endif
}

int COverlordTrap::GetTrapIndex() const
{
	COverlordData * pData = GetOverlordData();

	for(int i = 0; i < pData->GetMaxTraps(); i++)
	{
		COverlordTrap * pTrap = pData->GetTrap(i);

		if(pTrap != this)
			continue;

		return i;
	}

	return -1;
}

void COverlordTrap::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	BaseClass::ImpactTrace(pTrace, iDamageType, pCustomImpactName);

	if(m_iHealth > 0)
	{
		Vector reflect = pTrace->endpos - pTrace->startpos;
		reflect.x = random->RandomFloat(-0.45f, 0.45f);
		reflect.y = random->RandomFloat(-0.45f, 0.45f);
		reflect.z = random->RandomFloat(-0.45f, 0.45f);
		VectorNormalize(reflect);

		QAngle angle;
		VectorAngles(reflect, angle);

		DispatchParticleEffect("TrapImpact", pTrace->endpos, angle, this);
	}
}

#ifdef CLIENT_DLL
void COverlordTrap::DrawBeam()
{
	if(!ShouldUseBeam() || !m_BeamMat)
		return;
	
	const float DIE_TIME = 0.000001f;
	const Vector center = GetBeamOrigin();

	Vector end = vec3_origin;
	// Draw the red beam now
	if(BeamRange() > 0.0f)
	{
		VectorMA(center, BeamRange(), GetBeamVector(), end);
		trace_t tr;
		UTIL_TraceLine(center, end, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		end = tr.endpos;

		color32 color;
		color.r = 255; color.b = 0; color.g = 0; color.a = 255;

		FXLineData_t data;
		data.m_flDieTime = DIE_TIME;
		data.m_vecStart = center;
		data.m_vecEnd = end;
		data.m_pMaterial = m_BeamMat;
		data.m_Color = color;
		data.m_flStartScale = 6;
		data.m_flEndScale = 6;
		data.m_flStartAlpha = 255;
		data.m_flEndAlpha = 255;

		data.m_vecStartVelocity = vec3_origin;
		data.m_vecEndVelocity = vec3_origin;

		FX_AddLine(data);


		if(GetArcDegrees() > 0.0f)
		{
			// Create vectors
			QAngle aForw;
			VectorAngles(GetBeamVector(), aForw);

			Vector forward, right, up;
			AngleVectors(aForw, &forward, &right, &up);

			Quaternion q;

			AxisAngleQuaternion(up, GetArcDegrees(), q);
			Vector vLeft;
			VectorRotate(forward, q, vLeft);

			AxisAngleQuaternion(up, -GetArcDegrees(), q);
			Vector vRight;
			VectorRotate(forward, q, vRight);

			// Prepare data
			data.m_flStartScale = 2;
			data.m_flEndScale = 2;
			data.m_Color.r = 0;
			data.m_Color.b = 0;
			data.m_Color.g = 255;

			Vector arcend;

			// First left
			VectorMA(center, BeamRange(), vLeft, arcend);
			UTIL_TraceLine(center, arcend, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			data.m_vecEnd = tr.endpos;

			FX_AddLine(data);

			// Now right
			VectorMA(center, BeamRange(), vRight, arcend);
			UTIL_TraceLine(center, arcend, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			data.m_vecEnd = tr.endpos;

			FX_AddLine(data);
		}
		if(GetPitchDeegres() > 0.0f)
		{
			// Create vectors
			QAngle aForw;
			VectorAngles(GetBeamVector(), aForw);

			Vector forward, right, up;
			AngleVectors(aForw, &forward, &right, &up);

			Quaternion q;

			AxisAngleQuaternion(right, GetPitchDeegres(), q);
			Vector vUp;
			VectorRotate(forward, q, vUp);

			AxisAngleQuaternion(right, -GetPitchDeegres(), q);
			Vector vDown;
			VectorRotate(forward, q, vDown);

			// Prepare data
			data.m_flStartScale = 2;
			data.m_flEndScale = 2;
			data.m_Color.r = 0;
			data.m_Color.b = 255;
			data.m_Color.g = 0;

			Vector arcend;

			// First left
			VectorMA(center, BeamRange(), vUp, arcend);
			UTIL_TraceLine(center, arcend, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			data.m_vecEnd = tr.endpos;

			FX_AddLine(data);

			// Now right
			VectorMA(center, BeamRange(), vDown, arcend);
			UTIL_TraceLine(center, arcend, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			data.m_vecEnd = tr.endpos;

			FX_AddLine(data);
		}

		//FX_DrawLine(center, end, 15, m_BeamMat, color);
	}

	Vector start = (end != vec3_origin) ? end : center;
	VectorMA(start, MAX_TRACE_LENGTH, GetBeamVector(), end);

	trace_t tr;
	UTIL_TraceLine(center, end, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	end = tr.endpos;

	color32 color;
	color.r = 255; color.b = 255; color.g = 255; color.a = 255;

	FXLineData_t data;
	data.m_flDieTime = DIE_TIME;
	data.m_vecStart = start;
	data.m_vecEnd = end;
	data.m_pMaterial = m_BeamMat;
	data.m_Color = color;
	data.m_flStartScale = 8;
	data.m_flEndScale = 8;
	data.m_flStartAlpha = 255;
	data.m_flEndAlpha = 255;
	data.m_vecStartVelocity = vec3_origin;
	data.m_vecEndVelocity = vec3_origin;

	FX_AddLine(data);
}

void COverlordTrap::DrawBeam(Vector start, Vector end, color32 color, float flDieTime /*= 0.035f*/)
{
	FXLineData_t data;
	data.m_flDieTime = flDieTime;
	data.m_vecStart = start;
	data.m_vecEnd = end;
	data.m_pMaterial = m_BeamMat;
	data.m_Color = color;
	data.m_flStartScale = 8;
	data.m_flEndScale = 8;
	data.m_flStartAlpha = 255;
	data.m_flEndAlpha = 255;

	data.m_vecStartVelocity = vec3_origin;
	data.m_vecEndVelocity = vec3_origin;

	FX_AddLine(data);
}

void COverlordTrap::ClientThink()
{
	// Blink stuff
	if(( m_flBlinkDisable != 0.0f ) &&
		m_flBlinkDisable <= gpGlobals->curtime)
	{
		DeregisterGlow();
	}

	if(IsGhost())
	{
		if(GetRenderColor().a > 0)
		{
			DrawBeam();
				//FX_DrawLine(start, end, 9, m_BeamMat, color);

			if(GetRadiusRange() > 0.0f)
				DrawRadius();
		

			GhostThink();

		}
	}
	else
	{
		if(C_BasePlayer::GetLocalPlayer()->IsOverlord())
		{
			if((g_ConsoleManager.GetHoverTrap() == this) || 
				(g_ConsoleManager.IsInBuildMode() && !IsDecaying()))
			{
				DrawBeam();

				if(GetRadiusRange() > 0.0f)
					DrawRadius();
			}
		}

		TrapThink();
	}

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

Vector COverlordTrap::GetBeamVector() const
{
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	return forw;
}

#endif

#ifndef CLIENT_DLL
void COverlordTrap::Think()
{
	VPROF_BUDGET( "COverlordTrap::Think", VPROF_BUDGETGROUP_GAME );

	// Delayed hint for the client for initial dormant
	if(!m_HandledInitialDormant && IsInInitialDormant() && ((gpGlobals->curtime - GetTrapSpawnTime()) >= 0.225f))
	{
		//DevMsg("Sending a hint for delayed initial\n");
		m_HandledInitialDormant = true;
	
		IGameEvent * event = gameeventmanager->CreateEvent("trap_dormant");
		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetBool("dormant", true);

			gameeventmanager->FireEvent(event);
		}
	}
	else if(!IsInInitialDormant() && ((gpGlobals->curtime - GetTrapSpawnTime()) >= 0.30f))
	{
		m_HandledInitialDormant = true;
	}
	if(IsDecaying())
	{
		DecayThink();
	}
	else if(ShouldDecay())
	{
		DecayTrap();
	}
	else if((GetNextRun() <= gpGlobals->curtime) && GetNextRun() != NEVER_RUN)
	{
		if(!IsTrapDormant() && IsTrapEnabled())
			RunTrap();
		else if(IsTrapDormant())
		{
			// Activate after a set period of time,
			// just to make sure it isn't dormant for too long
			//if(GetDormantDuration() >= GetTrapLifetime())
			//	SetTrapDormant(false);

			if(IsInInitialDormant())
			{
				bool bSetDormant = true;
				CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
				if(pRebels)
				{
					for(int i = 0; i < pRebels->GetNumPlayers(); i++)
					{
						CBasePlayer * pRebel = pRebels->GetPlayer(i);

						if(!pRebel)
							continue;

						float length = FLT_MAX;
						if(Use2DForInitialDormant())
							length = (pRebel->GetAbsOrigin() - GetAbsOrigin()).Length2D();
						else
							length = (pRebel->GetAbsOrigin() - GetAbsOrigin()).Length();

						if(length <= GetInitialDormantRange())
						{
							if(!UseLOSForInitialDormant())
							{
								bSetDormant = false;
								break;
							}
							else
							{
								trace_t tr;
								UTIL_TraceLine(GetEmitterPosition(), pRebel->EyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

								if(tr.m_pEnt == pRebel)
								{
									bSetDormant = false;
									break;
								}

								UTIL_TraceLine(GetEmitterPosition(), pRebel->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

								if(tr.m_pEnt == pRebel)
								{
									bSetDormant = false;
									break;
								}
							}
						}
					}

					if(!bSetDormant)
						SetTrapDormant(false);

				}
			}
		}
		else if(!IsTrapEnabled())
		{
			if(m_flTimeEnable <= gpGlobals->curtime)
				EnableTrap();
		}
	}

	// Checking for dead parent
	if(m_bHasParent && !IsDecaying())
	{
		if(!GetMoveParent() || (GetMoveParent() && GetMoveParent()->GetEffects() & EF_NODRAW))
		{
			if(m_bExploding)
			{
				ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), this, m_flExplosionMagnitude, m_flExplosionRadius, true, m_flExplosionForce);
				UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 3.5f, m_flExplosionRadius, SHAKE_START);
				CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	
				CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	

				UTIL_Remove(this);
			}
			else
			{
				DecayTrap();
			}
		}
	
	}

	// Delayed event now for the client to pickup
	if((GetTrapSpawnTime() + 0.3f <= gpGlobals->curtime) && !m_EventSent)
	{
		// Fire an event
		IGameEvent * event = gameeventmanager->CreateEvent("trap_built");

		if(event)
		{
			event->SetInt("entindex", entindex());

			gameeventmanager->FireEvent(event);
		}

		m_EventSent = true;
	}

	// Zap damage now
	if(m_flZapDelay != 0.0f && m_flZapStart != 0.0f && m_flZapDelay <= gpGlobals->curtime)
	{
		Vector dir = m_vZapPoint - WorldSpaceCenter();
		VectorNormalize(dir);
		Vector end;
		VectorMA(WorldSpaceCenter(), MAX_TRACE_LENGTH, dir, end);

		trace_t tr;
		UTIL_TraceHull(WorldSpaceCenter(), end, Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		
		if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
		{
			CBasePlayer * pZapped = static_cast<CBasePlayer*>(tr.m_pEnt);
			float damage = (m_flZapDelay - m_flZapStart) * eo_traps_zap_damage_multiplier.GetFloat();
			pZapped->OnTakeDamage(CTakeDamageInfo(this, this, damage, DMG_SHOCK));
			pZapped->ViewPunch( QAngle( -8, damage, 4 ) );

			// Minor slowdown
			pZapped->AddSlowdown(eo_traps_zap_slowdown.GetFloat(), eo_traps_zap_slowdown_length.GetFloat(), NULL);
		}

		EmitSound("ambient.electrical_zap_1");
		// Zero them now
		m_flZapStart = 0.0f;
		m_flZapDelay = 0.0f;

	}

	if(IsTrapMarked() && (m_flMarkEnd <= gpGlobals->curtime))
	{
		m_flMarkEnd = 0.0f;

		IGameEvent * event = gameeventmanager->CreateEvent("trap_unmark");
	
		if(event)
		{
			event->SetInt("entindex", entindex());
			gameeventmanager->FireEvent(event);
		}
	}

	SetNextThink(gpGlobals->curtime + GENERAL_THINK);
}

int  COverlordTrap::ObjectCaps()
{
	return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE);
}

bool COverlordTrap::ShouldDecay() const
{ 
	return (gpGlobals->curtime >= (GetTrapSpawnTime() + GetTrapLifetime())) && 
		!eo_trapsneverdecay.GetBool() && 
		!m_bIsDormant && !GetOverlordData()->IsInTutorial(); 
}

void COverlordTrap::DecayThink()
{
	color32 c = GetRenderColor();
	int a = c.a;
	a -= DECAY_COLOR_ALPHA * GENERAL_THINK;

	// So it doesn't get reset
	if(a <= 0)
	{
		UTIL_Remove(this); 
		return;
	}

	SetRenderColor(c.r, c.g, c.b, a);
}

CBasePlayer * COverlordTrap::GetScorer()
{
	return (GET_OVERLORD_DATA)->GetOverlord();
}
void COverlordTrap::ReleaseTrap()
{
	SetParent(NULL);
	m_bHasParent = false;

	m_bUseDefenses = false;
}

void COverlordTrap::Event_Killed(const CTakeDamageInfo &info)
{
	if(!IsDecaying())
	{
		if(ShouldUseDefenses())
		{
			int rand = random->RandomInt(0, 100);

			int chance = eo_traps_defense_chance.GetInt();

			if(rand <= chance)
				CreateTrapDefense(GetEmitterPosition(), GetAbsAngles(), this);
		}
		else if(m_bUseDefenses)
		{
			CBasePlayer * pPlayer = GetLastHacker();

			if(pPlayer)
			{
				pPlayer->IncreaseOverlordDamage(GetPoints(CLASS_HACKER));
			}
		}

		int points = GetPointsOnDestroyed();

		if(points > 0)
		{
			CBasePlayer * pPlayer = ToBasePlayer(info.GetAttacker());

			if(pPlayer)
			{
				pPlayer->IncreaseOverlordDamage(points);
			}
		}


	}


	// We need to destroy the trap AFTER creating defense but BEFORE exploding
	BaseClass::Event_Killed(info);

	if(!IsDecaying())
	{
		if(m_bExploding && eo_traps_explode.GetBool())
		{
			ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), NULL, m_flExplosionMagnitude, m_flExplosionRadius, true, m_flExplosionForce);
		}
	}

	if(ShouldGib())
	{	
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib1.mdl", 10.0f );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib2.mdl", 10.0f );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib3.mdl", 10.0f );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib4.mdl", 10.0f );
		CGib::SpawnSpecificGibs( this, 1, 500, 250, "models/gibs/metal_gib5.mdl", 10.0f );
	}

}

void COverlordTrap::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	if(!IsDecaying())
	{
		IGameEvent * event = gameeventmanager->CreateEvent("trap_destroyed");

		if(event)
		{
			event->SetInt("entindex", entindex());

			gameeventmanager->FireEvent(event);
		}
	}
}

int COverlordTrap::OnTakeDamage(const CTakeDamageInfo &info)
{
	int retVal = BaseClass::OnTakeDamage(info);

	// Explosions don't zap people
	if(ShouldFireZaps() && GetHealth() > 0 && 
		info.GetDamageType() != DMG_BLAST && 
		info.GetAttacker() && 
		info.GetAttacker()->IsPlayer())
	{
		if((GetHealth() <= m_NextZap))
		{
			SelectNextZap();
			FireZap(static_cast<CBasePlayer*>(info.GetAttacker()));
		}
	}

	// Don't let Ov's grenades, defenses etc. harm this trap!
	if(info.GetAttacker() && info.GetAttacker()->GetOwnerEntity() == GetOverlordData()->GetOverlord())
		return 0;

	// Don't let other traps harm us
	if(info.GetAttacker() && !strncmp("trap_", info.GetAttacker()->GetClassname(), 5))
		return 0;

	return retVal;
}

void COverlordTrap::RestoreDuration(bool bPay)
{
	// Check whether we can pay first
	if(bPay)
	{
		float cost = eo_traps_restore_duration_multiplier.GetFloat() *(float)((GetTrapDefaultLifetime() - GetRemainingLifetime())/GetTrapDefaultLifetime()) * (float)GetTrapCost();

		if(cost <= 0)
			cost = 1;

		cost = ceil(cost);

		if(cost > GetOverlordData()->GetPower())
			return;

		GetOverlordData()->HandlePowerEvent(EVENT_DYNAMICTRAP, cost);
	}

	// Increase the lifetime addition
	m_flLifeAdd += GetTrapDefaultLifetime() - GetRemainingLifetime();
}

void COverlordTrap::RestoreHealth(bool bPay)
{
	// Check whether we can pay first
	if(bPay)
	{
		float cost = eo_traps_restore_health_multiplier.GetFloat() * (float)((float)(GetMaxHealth() - GetHealth())/(float)GetMaxHealth()) * (float)GetTrapCost();

		if(cost <= 0)
			cost = 1;

		cost = ceil(cost);

		if(cost > GetOverlordData()->GetPower())
			return;

		GetOverlordData()->HandlePowerEvent(EVENT_DYNAMICTRAP, cost);
	}

	// Repair
	m_iHealth = m_iMaxHealth;
}

void COverlordTrap::MarkTrap(float flLength)
{
	m_flMarkEnd = gpGlobals->curtime + flLength;

	IGameEvent * event = gameeventmanager->CreateEvent("trap_mark");
	
	if(event)
	{
		event->SetInt("entindex", entindex());
		gameeventmanager->FireEvent(event);
	}
}

int COverlordTrap::GetPoints(PlayerClass_t playerClass) const
{
	if(playerClass == CLASS_ASSAULT)
	{
		if((m_flLastTimeMarked == 0.0f) && (GetRenderColor().a < 255))
			return eo_traps_points_invisible.GetInt() + GetKills() * eo_traps_points_per_kill.GetInt();
		else
			return eo_traps_points.GetInt() + GetKills() * eo_traps_points_per_kill.GetInt();
	}

	if(playerClass == CLASS_HACKER)
		return eo_hacker_points.GetInt();

	return 0;
}
#endif


void COverlordTrap::EnableTrap()
{
#ifndef CLIENT_DLL
	// Inform the entity
	EntityMessageBegin( this, true );
		
		WRITE_SHORT(MESSAGE_ENABLE);

	MessageEnd();

	m_flTimeEnable = 0.0f;
	m_bDisabled = false;
#else
	if(m_DisableEffect)
	{
		ParticleProp()->StopEmission(m_DisableEffect);
		m_DisableEffect = NULL;
	}
#endif
}

void COverlordTrap::DisableTrap(float flTime)
{
#ifndef CLIENT_DLL
	// Inform the entity
	EntityMessageBegin( this, true );
		
		WRITE_SHORT(MESSAGE_DISABLE);
		WRITE_FLOAT(flTime);

	MessageEnd();

	m_flTimeEnable = flTime;
	m_bDisabled = true;
#else
	if(m_DisableEffect)
	{
		ParticleProp()->StopEmission(m_DisableEffect);
		m_DisableEffect = NULL;
	}

	m_DisableEffect = ParticleProp()->Create(DISABLER_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);

#endif
}

void COverlordTrap::DecayTrap()
{
#ifndef CLIENT_DLL
	SetSolid(SOLID_NONE);
	SetRenderMode(kRenderTransColor);

	m_bDecaying = true;

	DecayThink();

	IGameEvent * event = gameeventmanager->CreateEvent("trap_decayed");

	if(event)
	{
		event->SetInt("entindex", entindex());

		gameeventmanager->FireEvent(event);
	}
#else

#endif
}

Vector COverlordTrap::GetEmitterPosition()
{
	Vector emitter;
	if(GetAttachment( LookupAttachment("dynamic_trap"), emitter))
		return emitter;

	return WorldSpaceCenter();
}

// Looks up the trap's cost in the records
int COverlordTrap::GetTrapCost(void) const
{
	int index = -1;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
#ifdef CLIENT_DLL
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, const_cast<COverlordTrap*>(this)->GetClientClass()->GetName()))
			continue;
#else
		if(Q_strcmp(COverlordTrap::GetRecord(i).entName, const_cast<COverlordTrap*>(this)->GetClassname()))
			continue;
#endif
		index = i;
		break;
	}

	if(index == -1)
	{
		Warning("No entity found in GetTrapCost()!\n");
		return -1;
	}

	return COverlordTrap::GetRecord(index).trapCost;
}

// Looks up the trap's lifetime in the records
float COverlordTrap::GetTrapDefaultLifetime(void) const
{
	int index = -1;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
#ifdef CLIENT_DLL
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, const_cast<COverlordTrap*>(this)->GetClientClass()->GetName()))
			continue;
#else
		if(Q_strcmp(COverlordTrap::GetRecord(i).entName, const_cast<COverlordTrap*>(this)->GetClassname()))
			continue;
#endif
		index = i;
		break;
	}

	if(index == -1)
	{
		Warning("No entity found in GetTrapLifeTime()!\n");
		return -1;
	}

	// Add up our dormant time
	return COverlordTrap::GetRecord(index).trapDuration;
}

// Spews out the true lifetime for this specific trap
float COverlordTrap::GetTrapLifetime(void) const
{
	// Add up our dormant time
	float baseLife = GetTrapDefaultLifetime() + m_flLifeAdd;

	if(IsTrapDormant() && m_flSetDormant != 0.0f)
	{
		float dormant = gpGlobals->curtime - m_flSetDormant;
		baseLife += dormant;
	}

	return baseLife;
}

const char * COverlordTrap::GetTrapName() const
{
	int index = -1;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
#ifdef CLIENT_DLL
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, const_cast<COverlordTrap*>(this)->GetClientClass()->GetName()))
			continue;
#else 
		if(Q_strcmp(COverlordTrap::GetRecord(i).entName, const_cast<COverlordTrap*>(this)->GetClassname()))
			continue;
#endif
		index = i;
		break;
	}

	if(index == -1)
	{
		Warning("No entity found in GetTrapName()!\n");
		return "";
	}

	return COverlordTrap::GetRecord(index).buttonName;
}


#ifdef CLIENT_DLL
void COverlordTrap::DrawRadius(int radius, int width, color32 color, const Vector * center)
{
	const Vector & ctr = (center != NULL) ? (*center) : GetRadiusOrigin();
	
	CSingleUserRecipientFilter thisplayer(C_BasePlayer::GetLocalPlayer());
	te->BeamRingPoint(thisplayer, 0.0f, ctr,
		radius-3, radius, 
		m_BeamTexture, 0, 0, 2, 0.01f, width, 0, 0, color.r, color.g, color.b, color.a, 0, FBEAM_FADEOUT);
}
#endif

/*void COverlordTrap::SmartTraceHull(const Vector & start, const Vector & end, const Vector & hulMin, const Vector & hulMax, int mask, 
									const IHandleEntity * ignore, int collisiongroup, trace_t & tr) const
{
	trace_t trhul;
	UTIL_TraceHull(start, end, hulMin, hulMax, mask, ignore, collisiongroup, &trhul);

	if(trhul.m_pEnt)
	{
		tr = trhul;
		return;
	}

	// Tracehull didn't work, try traceline now
	trace_t trtr;
	UTIL_TraceLine(start, end, mask, ignore, collisiongroup, &trtr);

	if(trtr.m_pEnt || trhul.startsolid)
	{
		tr = trtr;
		return;
	}

	tr = trhul;	
}*/

void COverlordTrap::SetDestroyable(int health)
{
	if(health > 0)
	{
#ifndef CLIENT_DLL
		SetMaxHealth(health);
#endif
		SetHealth(health);

		m_takedamage = DAMAGE_YES;
	}
	else
	{
#ifndef CLIENT_DLL
		SetMaxHealth(1000);
#endif
		SetHealth(1000);

		m_takedamage = DAMAGE_NO;
	}
}

bool COverlordTrap::IsPlayerInRadius(float radius, bool bLineOfSight, bool bTwoDimensions)
{
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive() || pPlayer->IsInvisible())
			continue;

		float dist = 0;
		
		if(!bTwoDimensions)
			dist = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length();
		else
			dist = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length2D();

		if(dist <= radius)
		{
			if(bLineOfSight)
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->WorldSpaceCenter(), 
					MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

				if(tr.m_pEnt != pPlayer)
					continue;
			}

			return true;
		}
	}
	return false;
}

#ifndef CLIENT_DLL
void COverlordTrap::SelectNextZap()
{
	m_NextZap = GetHealth() - random->RandomInt(eo_traps_discharge_min.GetInt(), eo_traps_discharge_max.GetInt());
}

void COverlordTrap::FireZap(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return;

	float delay = random->RandomFloat(eo_traps_discharge_delay_min.GetFloat(),
		eo_traps_discharge_delay_max.GetFloat());
	m_flZapDelay = gpGlobals->curtime + delay;
	m_flZapStart = gpGlobals->curtime;
	m_vZapPoint = pPlayer->RealEyePosition();

	IGameEvent * event = gameeventmanager->CreateEvent("trap_discharge");

	if(event)
	{
		event->SetInt("entindex", entindex());
		event->SetInt("userid", pPlayer->GetUserID());
		event->SetFloat("delay", delay);
		gameeventmanager->FireEvent(event);
	}

	// Inform the entity
	EntityMessageBegin( this, true );
		
		WRITE_SHORT(MESSAGE_ZAP);
		WRITE_FLOAT(m_flZapDelay);
		WRITE_SHORT(pPlayer->GetUserID());

	MessageEnd();

}
#else

// We need this for just one beam that we want to use
extern void FX_BuildTesla( 
	C_BaseEntity *pEntity, 
	const Vector &vecOrigin, 
	const Vector &vecEnd,
	const char *pModelName,
	float flBeamWidth,
	const Vector &vColor,
	int nFlags,
	float flTimeVisible );

void COverlordTrap::ReceiveMessage(int classID, bf_read &msg)
{
	if(classID != GetClientClass()->m_ClassID)
	{
		BaseClass::ReceiveMessage(classID, msg);
		return;
	}

	int type = msg.ReadShort();

	if(type == MESSAGE_ZAP)
	{
		float life = msg.ReadFloat() - gpGlobals->curtime;
		int userid = msg.ReadShort();

		CBasePlayer * pPlayer = ToBasePlayer( ClientEntityList().GetEnt( engine->GetPlayerForUserID( userid ) ) );

		if(pPlayer)
		{
			Vector dir = pPlayer->RealEyePosition() - WorldSpaceCenter();
			VectorNormalize(dir);

			Vector pPoint;
			VectorMA(WorldSpaceCenter(), MAX_TRACE_LENGTH, dir, pPoint);

			CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
			trace_t tr;
			UTIL_TraceLine(WorldSpaceCenter(), pPoint, MASK_SHOT_HULL, &filter, &tr);
			//CollisionProp()->RandomPointInBounds(Vector(0.0f, 0.0f, 0.8f), Vector(1.0f, 1.0f, 1.0f), &pPoint);

			FX_BuildTesla(this, WorldSpaceCenter(), tr.endpos, ZAP_SPRITE, 8, Vector(255, 0, 0), 
				FBEAM_FADEOUT | FBEAM_ONLYNOISEONCE, life);

			/*CNewParticleEffect * pEffect = ParticleProp()->Create("ZapParticle", PATTACH_ABSORIGIN_FOLLOW);

			if(pEffect)
			{
				//pEffect->SetControlPoint(0, WorldSpaceCenter());
				pEffect->SetControlPoint(1, tr.endpos);
			}*/
		}
	}
	else if(type == MESSAGE_DISABLE)
	{
		DisableTrap(msg.ReadFloat());
	}
	else if(type == MESSAGE_ENABLE)
	{
		EnableTrap();
	}
}


void COverlordTrap::FireGameEvent(IGameEvent * event)
{
	if(event)
	{
		if(!Q_stricmp(event->GetName(), "trap_mark"))
		{
			int entind = event->GetInt("entindex");

			if(entind == entindex())
			{
				C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

				if(!pPlayer)
					return;

				if(!pPlayer->IsRebel())
					return;

				if(!IsGlowRegistered())
					RegisterGlow(Color(255, 255, 255));
			}
		}
		else if(!Q_stricmp(event->GetName(), "trap_unmark"))
		{
			int entind = event->GetInt("entindex");
			
			if(entind == entindex())
			{
				C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

				if(!pPlayer)
					return;

				if(pPlayer->IsOverlord())
					return;

				if(IsGlowRegistered())
					DeregisterGlow();
			}
		}
	}
}
#endif
//************************************************
//				End base class
//************************************************

#ifdef CLIENT_DLL
#define COverlordLaser C_OverlordLaser
#endif

ConVar eo_laser_activation_range("eo_laser_activation_range", "2200", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar eo_laser_damage("eo_laser_damage", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_laser_minimum_distance("eo_laser_minimum_distance", "24.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_laser_degree_distance_yaw("eo_laser_degree_distance_yaw", "6.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_laser_degree_distance_pitch("eo_laser_degree_distance_pitch", "30.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_laser_health("eo_laser_health", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_laser_range("eo_laser_range", "2160", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_laser_sound_duration("eo_laser_sound_duration", "0.15", FCVAR_REPLICATED | FCVAR_CHEAT);

#define LASER_PARTICLE_EFFECT "LaserEmitter"
#define LASER_VECTOR Vector(1, 1, 1)
#define LASER_RANGE eo_laser_range.GetFloat()
#define LASER_ZAP_SOUND "Weapon_MegaPhysCannon.ChargeZap"

class COverlordLaser : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordLaser, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	COverlordLaser();
	virtual ~COverlordLaser();

	virtual void Precache();

	virtual void DecayTrap();
	virtual error_t CanBuild();

	virtual void SetTrapDormant(bool bDormant)
	{
		BaseClass::SetTrapDormant(bDormant);
		

		if(bDormant)
		{
#ifdef CLIENT_DLL
			if(m_hEffect.IsValid())
			{
				ParticleProp()->StopEmission(m_hEffect.GetObject());
				m_hEffect = NULL;
			}
#else
			if(m_LaserBeam)
				m_LaserBeam->TurnOn();
#endif
		}
		else
		{
#ifdef CLIENT_DLL
			if(!m_hEffect)
				m_hEffect = ParticleProp()->Create(LASER_PARTICLE_EFFECT, PATTACH_ABSORIGIN_FOLLOW );
#else
			//m_LaserBeam->TurnOff();
#endif
		}

	}

	virtual float GetNormalMultiplier() const { return 3.5f; };
#ifdef CLIENT_DLL
	virtual void InitializeTrap();

	virtual float BeamRange() const { return LASER_RANGE; };

	virtual float GetArcDegrees() const { return eo_laser_degree_distance_yaw.GetFloat(); };
	virtual float GetPitchDeegres() const { return eo_laser_degree_distance_pitch.GetFloat(); };
	
	virtual float GetInitialDormantRange() const { return eo_laser_activation_range.GetFloat(); };

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.8f; };
	virtual float GetPanelZDistance() const { return 5.0f; };
#else
	virtual void RunTrap();

	virtual void InitializeTrap();

	virtual void EnableTrap();
	virtual void DisableTrap(float flTime);
	
#endif
	virtual Vector GetEmitterPosition()
	{
		Vector emitter;
		if(GetAttachment( LookupAttachment("dynamic_trap"), emitter))
			return emitter;

		Vector forw;
		AngleVectors(GetAbsAngles(), &forw);

		return GetAbsOrigin() + forw * 2;
	}
private:
#ifdef CLIENT_DLL
	CSmartPtr<CNewParticleEffect> m_hEffect;
#else
	CHandle<CBeam> m_LaserBeam;
	float m_flSoundDelay;
#endif
};



IMPLEMENT_NETWORKCLASS_ALIASED( OverlordLaser, DT_OverlordLaser )

BEGIN_NETWORK_TABLE( COverlordLaser, DT_OverlordLaser )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordLaser)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_laser, COverlordLaser, Laser, 45, 60, "A laser trap which hurts anyone who crosses its beam", 
					"Offense");

COverlordLaser::COverlordLaser()
{
#ifndef CLIENT_DLL
	m_flSoundDelay = 0.0f;
#endif
	SET_TRAP_MODEL(models/dynamic_traps/laser/dynamic_traps_laser.mdl); 
}

COverlordLaser::~COverlordLaser()
{ 
#ifndef CLIENT_DLL
	if(m_LaserBeam)
		UTIL_Remove(m_LaserBeam); 
#else
	if(m_hEffect.IsValid())
	{
		ParticleProp()->StopEmission(m_hEffect.GetObject());
		m_hEffect = NULL;
	}
#endif
};

void COverlordLaser::Precache()
{ 
	BaseClass::Precache(); 
	PrecacheModel(ZAP_SPRITE);
	PrecacheParticleSystem(LASER_PARTICLE_EFFECT);
	PrecacheScriptSound(LASER_ZAP_SOUND);
}

void COverlordLaser::DecayTrap()
{ 
	BaseClass::DecayTrap(); 
	
#ifndef CLIENT_DLL
	if(m_LaserBeam) 
		UTIL_Remove(m_LaserBeam);
#else
	if(m_hEffect.IsValid())
	{
		ParticleProp()->StopEmission(m_hEffect.GetObject());
		m_hEffect = NULL;
	}
#endif
}

error_t COverlordLaser::CanBuild()
{
	if(UTIL_PointContents(GetEmitterPosition()) & MASK_SHOT_HULL)
	{
		return CreateError("Trap would have no effect.");
	}

	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);
	VectorNormalize(forw);

#ifndef CLIENT_DLL
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
#else
	C_Team * pRebels = GetGlobalTeam(TEAM_REBELS);
#endif
	if(pRebels)
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer)
				continue;

			if(!pPlayer->IsAlive() || pPlayer->IsInvisible())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() > LASER_RANGE)
				continue;

			trace_t tr;
			UTIL_TraceHull(GetEmitterPosition() + forw * 6, pPlayer->WorldSpaceCenter(), -LASER_VECTOR, LASER_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt != pPlayer)
				continue;

			Vector dir = pPlayer->WorldSpaceCenter() - GetEmitterPosition();
			VectorNormalize(dir);

			Vector relDir, relForw;
			WorldToEntitySpace(forw, &relForw);
			WorldToEntitySpace(dir, &relDir);

			QAngle playerAngle, entityAngle;
			VectorAngles(forw, entityAngle);
			VectorAngles(dir, playerAngle);

			float pitch = fabs(AngleDistance(playerAngle[PITCH], entityAngle[PITCH]));
			float yaw = fabs(AngleDistance(playerAngle[YAW], entityAngle[YAW]));


			if((yaw <= (eo_laser_degree_distance_yaw.GetFloat())) && 
				(pitch <= (eo_laser_degree_distance_pitch.GetFloat())))
			{
				return CreateError("Player in range, cannot build.");
			}
		}

	}

	return BaseClass::CanBuild();
}

#ifdef CLIENT_DLL
void COverlordLaser::InitializeTrap()
{
	m_hEffect = ParticleProp()->Create(LASER_PARTICLE_EFFECT, PATTACH_ABSORIGIN_FOLLOW );
	if(!m_hEffect)
		Warning("Couldn't initialize particle effects for the laser!\n");
}
#else
void COverlordLaser::InitializeTrap()
{
	SetDestroyable(eo_laser_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	m_LaserBeam = CBeam::BeamCreate("sprites/laserbeam.vmt", 2);

	if(!m_LaserBeam)
	{
		Warning("No laser beam created!\n");
		return;
	}

	// Set position of the beam's start and end point
	const Vector center = GetEmitterPosition();
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	Vector end;
	VectorMA(center, MAX_TRACE_LENGTH, forw, end);

	trace_t tr;
	UTIL_TraceLine(center, end, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	end = tr.endpos;

	m_LaserBeam->PointsInit(center, end);
	m_LaserBeam->SetWidth( 3 );
	m_LaserBeam->SetEndWidth( 0.5f );
	m_LaserBeam->SetBrightness( 235 );
	m_LaserBeam->SetColor( 175, 200, 195 );
	m_LaserBeam->SetBrightness(125);
	//pBeam->LiveForTime( 0.5f );
	//m_LaserBeam->RelinkBeam();
	m_LaserBeam->SetNoise( 1.8f );
	m_LaserBeam->TurnOn();

}

void COverlordLaser::RunTrap()
{
	bool bRunning = false;
		
	Vector forw, end;
	AngleVectors(GetAbsAngles(), &forw);

	Vector finalDir = forw;

	// Get our angles based on players
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
	if(pRebels)
	{
		float maxpitch = eo_laser_degree_distance_pitch.GetFloat();
		float maxyaw = eo_laser_degree_distance_yaw.GetFloat();
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer)
				continue;

			if(pPlayer->IsDead() || pPlayer->IsInvisible())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() > LASER_RANGE)
				continue;

			trace_t tr;
			UTIL_TraceHull(GetEmitterPosition() + forw * 6, pPlayer->WorldSpaceCenter(), -LASER_VECTOR, LASER_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt != pPlayer)
				continue;

			Vector dir = pPlayer->WorldSpaceCenter() - GetEmitterPosition();
			VectorNormalize(dir);

			Vector relDir, relForw;
			WorldToEntitySpace(forw, &relForw);
			WorldToEntitySpace(dir, &relDir);

			QAngle playerAngle, entityAngle;
			VectorAngles(forw, entityAngle);
			VectorAngles(dir, playerAngle);

			float pitch = fabs(UTIL_AngleDistance(playerAngle[PITCH], entityAngle[PITCH]));
			float yaw = fabs(UTIL_AngleDistance(playerAngle[YAW], entityAngle[YAW]));
	
			if((pitch <= maxpitch) &&
				(yaw <= maxyaw))
			{
				finalDir = dir;
				maxpitch = pitch;
				maxyaw = yaw;
			}
		}
	}

	
	VectorMA(GetEmitterPosition(), LASER_RANGE, finalDir, end); 

	trace_t tr;
	UTIL_TraceHull(GetEmitterPosition() + forw * 6, end, -LASER_VECTOR, LASER_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	CBaseEntity * pHit = NULL;
	if(tr.m_pEnt && !COverlordTrap::EntityToTrap(tr.m_pEnt) && tr.m_pEnt->m_takedamage == DAMAGE_YES 
		/*Q_stricmp("worldspawn", tr.m_pEnt->GetClassname())*/)
	{
		pHit = tr.m_pEnt;
		bRunning = true;
		pHit->TakeDamage(CTakeDamageInfo(this, this, eo_laser_damage.GetFloat(), DMG_ENERGYBEAM));

		// Fire the sound
		if(m_flSoundDelay <= gpGlobals->curtime)
		{
			CPASFilter filter(tr.endpos);
			float time = eo_laser_sound_duration.GetFloat();

			m_flSoundDelay = gpGlobals->curtime + time;
			tr.m_pEnt->EmitSound(LASER_ZAP_SOUND, 0.0f, &time);
		}

		UTIL_TraceHull(pHit->WorldSpaceCenter() + Vector(0,0, 16), end, -LASER_VECTOR, LASER_VECTOR, MASK_SHOT_HULL, pHit, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt && !COverlordTrap::EntityToTrap(tr.m_pEnt) && tr.m_pEnt->m_takedamage == DAMAGE_YES)
		{
			tr.m_pEnt->TakeDamage(CTakeDamageInfo(this, this, eo_laser_damage.GetFloat(), DMG_ENERGYBEAM));
			pHit = tr.m_pEnt;
		}
	}

	if(bRunning)
	{
		m_LaserBeam->PointsInit(GetEmitterPosition(), tr.endpos);

		m_LaserBeam->RelinkBeam();
		m_LaserBeam->TurnOff();
	}
	else
	{
		m_LaserBeam->TurnOn();
	}

	
	SetNextRun(gpGlobals->curtime + 0.05f);

}


void COverlordLaser::EnableTrap()
{
	BaseClass::EnableTrap();

	if(m_LaserBeam)
		m_LaserBeam->TurnOff();
}

void COverlordLaser::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);

	if(m_LaserBeam)
		m_LaserBeam->TurnOn();
}
#endif

//====================================================================================================================================


#ifdef CLIENT_DLL
#define COverlordMine C_OverlordMine
#endif

ConVar eo_mine_distance("eo_mine_distance", "75.0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar eo_mine_proximity_use("eo_mine_proximity_use", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_proximity("eo_mine_proximity", "76.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_activation_range("eo_activation_range", "272", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_buzz("eo_mine_buzz", "0", FCVAR_REPLICATED);
ConVar eo_mine_buzz_volume("eo_mine_buzz_volume", "0.075", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_damage("eo_mine_damage", "90", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_magnitude("eo_mine_magnitude", "160", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_health("eo_mine_health", "30", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_delay("eo_mine_delay", "0.45", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mine_points("eo_mine_points", "3", FCVAR_REPLICATED | FCVAR_CHEAT);

#define MINE_HEALTH eo_mine_health.GetInt()
#define MINE_LOOP "d3_citadel.combine_ball_field_loop1"

#define MINE_PROXIMITY eo_mine_proximity.GetFloat()
#define MINE_RANGE eo_mine_distance.GetFloat()
#define MINE_TICK "NPC_RollerMine.Hurt"

class COverlordMine : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordMine, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordMine() ;
	virtual ~COverlordMine();

	virtual error_t CanBuild();
	virtual void Precache();

	virtual float GetNormalMultiplier() const { return 1; };

	virtual QAngle  NormalToAngle(const Vector & normal) const
	{
		return AlignToTop(normal);
	}
	virtual float GetInitialDormantRange() const { return eo_mine_activation_range.GetFloat(); };

#ifndef CLIENT_DLL
	virtual void SetTrapDormant(bool bDormant)
	{
		BaseClass::SetTrapDormant(bDormant);
	
		if(bDormant)
			StopSound(MINE_LOOP);
		else if(eo_mine_buzz.GetBool())
			EmitBuzzSound();
	}
	virtual void InitializeTrap();
	virtual void DecayTrap();

	virtual void RunTrap();
	float GetMineProximity();

	virtual void DisableTrap(float flTime)
	{
		BaseClass::DisableTrap(flTime);

		Explode();
	}

	virtual int GetPointsOnDestroyed() const { return eo_mine_points.GetInt(); };
#else
	virtual Vector GetBeamVector() const { return GetMineDirection(); };
	virtual float BeamRange() const { return MINE_RANGE; };

	virtual float GetRadiusRange() { return MINE_PROXIMITY; };
	virtual Vector GetRadiusOrigin();

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.8f; };
	virtual float GetPanelZDistance() const { return 4.0f; };
	virtual QAngle GetPanelAngle() const { QAngle angle = BaseClass::GetPanelAngle(); angle[PITCH] += -160; return angle; };
#endif

	Vector GetMineDirection() const;

private:
#ifndef CLIENT_DLL
	void EmitBuzzSound();
	void PrepareExplosion();
	void Explode();


	bool m_bExplode;
	float m_flDistance;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordMine, DT_OverlordMine )

BEGIN_NETWORK_TABLE( COverlordMine, DT_OverlordMine )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordMine)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_mine, COverlordMine, Landmine, 30, 90, "Explodes dealing moderate damage. Can be destroyed.",
					"Defense");

COverlordMine::COverlordMine()
{											  
	SET_TRAP_MODEL(models/dynamic_traps/mine/dynamic_traps_mine.mdl);
}

COverlordMine::~COverlordMine()
{
	StopSound(MINE_LOOP);
}

error_t COverlordMine::CanBuild()
{
	if(UTIL_PointContents(GetEmitterPosition()) & MASK_SHOT_HULL)
	{
		return CreateError("The trap would have no effect.");
	}

	if(IsPlayerInRadius(MINE_PROXIMITY + PLAYER_RADIUS))
	{
		return CreateError("Player in radius, cannot build.");
	}


	return BaseClass::CanBuild();
}

void COverlordMine::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(MINE_LOOP);
	PrecacheScriptSound(MINE_TICK);
}
#ifndef CLIENT_DLL
void COverlordMine::InitializeTrap()
{
	m_bUseDefenses = false;

	// set the distance we expect
	m_flDistance = GetMineProximity() - 2;

	// Make it destroyable
	SetDestroyable(MINE_HEALTH);
	SetExplodable(eo_mine_damage.GetFloat(), eo_mine_magnitude.GetFloat(), 45);

	if(eo_mine_buzz.GetBool())
	{
		EmitBuzzSound();
	}
}

void COverlordMine::DecayTrap()
{
	BaseClass::DecayTrap();

	StopSound(MINE_LOOP);
}

void COverlordMine::RunTrap()
{
	if(m_bExplode)
	{
		Explode();
		return;
	}

	if(int(GetMineProximity()) < int(m_flDistance))
	{
		PrepareExplosion();
		return;
	}

	if(eo_mine_proximity_use.GetBool())
	{
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

			if(!pPlayer || !pPlayer->IsAlive() || !pPlayer->IsRebel())
				continue;

			if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= MINE_PROXIMITY )
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

				if(tr.m_pEnt == pPlayer)
				{
					PrepareExplosion();
					return;
				}
			}
		}
	}

	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordMine::EmitBuzzSound()
{
	EmitSound_t params;
	params.m_flVolume = eo_mine_buzz_volume.GetFloat();
	//params.m_SoundLevel = SNDLVL_30dB;
	params.m_pSoundName = MINE_LOOP;
	params.m_nFlags = SND_CHANGE_VOL;
	CPASFilter filter(GetAbsOrigin());
	EmitSound(filter, entindex(), params);
}

void COverlordMine::PrepareExplosion()
{
	m_bExplode = true;
	EmitSound(MINE_TICK);
	SetNextRun(gpGlobals->curtime + eo_mine_delay.GetFloat());
}

void  COverlordMine::Explode()
{
	m_bUseDefenses = false;
	UTIL_Remove(this);

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), NULL, eo_mine_damage.GetInt(), eo_mine_magnitude.GetInt(), true, 45 );

	UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 3.5f, 128, SHAKE_START);
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	
}

float COverlordMine::GetMineProximity()
{
	// Now get general distance from the centre to the location
	Vector end;
	Vector forw = GetMineDirection();
	const Vector pos = GetEmitterPosition();

	VectorMA(pos, MINE_RANGE, forw, end);


	trace_t tr;
	UTIL_TraceHull(pos + forw * 4, end, -Vector(4, 4, 4), Vector(4, 4, 4), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	
	if(tr.startsolid || tr.allsolid)
		return 0.0f;

	return (tr.endpos - pos).Length();
}
#else
Vector COverlordMine::GetRadiusOrigin()
{
	Vector vUp;
	VectorRotate(Vector(0, 0, 1), EntityToWorldTransform(), vUp);
	VectorNormalize(vUp);

	return (WorldSpaceCenter() + vUp * 4);
}
#endif

Vector COverlordMine::GetMineDirection() const
{
	// Get direction vector and convert it to world
	Vector relUp(0, 0, 1);
	Vector vUp;
	EntityToWorldSpace(relUp, &vUp);
	
	return (vUp - GetAbsOrigin());
	//return vUp;
}


//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordFan C_OverlordFan
#endif


ConVar eo_fan_distance("eo_fan_distance", "1112.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_fan_activation_range("eo_fan_activation_range", "1130", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_fan_physics_factor("eo_fan_physics_factor", "50.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_fan_player_factor("eo_fan_player_factor", "0.45", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_fan_health("eo_fan_health", "70", FCVAR_REPLICATED | FCVAR_CHEAT);

#define FAN_VECTOR Vector(38, 38, 38)
#define FAN_SOUND "d3_citadel.podarm_rotate"
//#define FAN_PARTICLE "FanWave"
#define FAN_RANGE eo_fan_distance.GetFloat()

class COverlordFan : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordFan, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	COverlordFan();
	virtual ~COverlordFan();

	virtual void Precache()
	{
		BaseClass::Precache();
		PrecacheScriptSound(FAN_SOUND);
	}

	virtual void InitializeTrap();
	virtual void DecayTrap();
	virtual error_t CanBuild();

	virtual float GetNormalMultiplier() const { return 12.0f; };

	virtual void SetTrapDormant(bool bDormant)
	{
		BaseClass::SetTrapDormant(bDormant);

		if(bDormant)
			StopSound(FAN_SOUND);
#ifndef CLIENT_DLL
		else
			m_bRunning = false;
#endif
	}
#ifndef CLIENT_DLL
	virtual void RunTrap();
	virtual void DisableTrap(float flTime);
#else
	virtual void  GhostThink();
	virtual float BeamRange() const { return FAN_RANGE; };

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.65f; };
	virtual float GetPanelZDistance() const { return 8.0f; };
#endif

	virtual Vector GetEmitterPosition();

private:
#ifdef CLIENT_DLL
	//CNewParticleEffect * m_hEffect;
#else
	// Variable for the fan turnings itself on/off
	bool m_bRunning;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordFan, DT_OverlordFan )

BEGIN_NETWORK_TABLE( COverlordFan, DT_OverlordFan )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordFan)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_fan, COverlordFan, Fan, 35, 60, "Pushes back players who get into its way", "Utility");

COverlordFan::COverlordFan()
{ 
	SET_TRAP_MODEL(models/dynamic_traps/fan/dynamic_traps_fan.mdl); 
#ifndef CLIENT_DLL
	m_bRunning = false;
#endif
};

COverlordFan::~COverlordFan()
{
	StopSound(FAN_SOUND);
#ifdef CLIENT_DLL
	/*if(m_hEffect)
	{
		ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}*/
#endif
}

void COverlordFan::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_fan_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);
	//EmitSound(FAN_SOUND); 
	//DispatchParticleEffect(FAN_PARTICLE, WorldSpaceCenter(), GetAbsAngles(), this);

	ResetSequence(LookupSequence("Idle"));
//#else
//	m_hEffect = ParticleProp()->Create(FAN_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);
#endif
}

void COverlordFan::DecayTrap()
{
	BaseClass::DecayTrap();

	StopSound(FAN_SOUND);

#ifdef CLIENT_DLL
	/*if(m_hEffect)
	{
		ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}*/
#endif
}

error_t COverlordFan::CanBuild()
{
	Vector end, forw;
	AngleVectors(GetAbsAngles(), &forw);
	Vector start = GetEmitterPosition() + forw * 64;
	VectorMA(start, FAN_RANGE, forw, end);

	/*UTIL_TraceHull(start, end, -FAN_VECTOR, FAN_VECTOR, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
		return CreateError("A player is in range.");
	}*/
	Ray_t ray;
	ray.Init(start, end, -FAN_VECTOR, FAN_VECTOR);
	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

	
	trace_t tr;
	for(int i = 0; i < count; i++)
	{
		CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive())
			continue;

		UTIL_TraceLine(start, pPlayer->WorldSpaceCenter(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

		if((tr.m_pEnt == pPlayer) || tr.fraction >= 0.98f)
		{
			return CreateError("A player is in range");
		}
	}


	UTIL_TraceHull(GetEmitterPosition() + forw * 4, end, -Vector(4, 4, 4), Vector(4, 4, 4), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	if(tr.allsolid || tr.startsolid)
	{
		return CreateError("The fan will have no effect!");
	}

	return BaseClass::CanBuild();
}
#ifdef CLIENT_DLL
void COverlordFan::GhostThink()
{
	// Create the four vectors
	Vector left, right;
	left = FAN_VECTOR;
	right.x = left.x = 16;
	right.y = -left.y;
	right.z = left.z = 0;

	Vector wLeft, wRight;
	EntityToWorldSpace(left, &wLeft);
	EntityToWorldSpace(right, &wRight);

	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);
	
	// Left beam
	trace_t tr;
	UTIL_TraceLine(wLeft, wLeft + forw * FAN_RANGE, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	DrawBeam(wLeft, tr.endpos, Color(255, 0, 0, 255));

	// Right beam
	UTIL_TraceLine(wRight, wRight + forw * MAX_TRACE_LENGTH, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	DrawBeam(wRight, tr.endpos, Color(255, 0, 0, 255));

}
#else
void COverlordFan::RunTrap()
{
	// Check whether we should actually run
	bool wasRunning = m_bRunning;
	m_bRunning = false;
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsAlive() || !pPlayer->IsRebel())
			continue;

		// Also when he is close enough
		if((GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length() < eo_fan_activation_range.GetFloat())
		{
			m_bRunning = true;
			break;
		}
		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), pPlayer->RealEyePosition(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt == pPlayer)
		{
			m_bRunning = true;
			break;
		}

		UTIL_TraceLine(GetAbsOrigin(), pPlayer->WorldSpaceCenter(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt == pPlayer)
		{
			m_bRunning = true;
			break;
		}
	}

	if(wasRunning && !m_bRunning)
	{
		StopSound(FAN_SOUND);
	}
	else if(!wasRunning & m_bRunning)
	{
		EmitSound(FAN_SOUND);
	}

	if(m_bRunning)
	{
		StudioFrameAdvance();
		Vector end;
		Vector forw;

		// Create all needed vectors
		AngleVectors(GetAbsAngles(), &forw);
		VectorMA(GetEmitterPosition(), FAN_RANGE, forw, end);


		Vector emitter = GetEmitterPosition();
		CBaseEntity * pList[1024];
		Ray_t ray;
		ray.Init(emitter, emitter + forw * FAN_RANGE, -FAN_VECTOR, FAN_VECTOR);
		int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

		for(int i = 0; i < count; i++)
		{
			CBaseEntity * pEnt = pList[i];

			if(!pEnt)
				return;

			trace_t tr;
			UTIL_TraceLine(GetEmitterPosition(), pEnt->WorldSpaceCenter(), MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt != pEnt)
				continue;

			CBasePlayer * pPlayer = ToBasePlayer(pEnt);
			if(!pPlayer)
			{
				IPhysicsObject * pPhys = pEnt->VPhysicsGetObject();
				if(pPhys)
					pPhys->ApplyForceCenter((end - pEnt->WorldSpaceCenter()) * eo_fan_physics_factor.GetFloat());
			}
			else
			{
				if(pPlayer->GetMoveType() != MOVETYPE_NOCLIP)
				{
					if(pPlayer->GetMoveType() == MOVETYPE_LADDER)
					{
						pPlayer->ExitLadder();
					}

					pPlayer->ApplyAbsVelocityImpulse((end - pEnt->WorldSpaceCenter()) * eo_fan_player_factor.GetFloat());
				}
			}

		}

		// Look for players but still collide with debris
		/*trace_t tr;
		UTIL_TraceHull(GetEmitterPosition() + forw * 48, end, -FAN_VECTOR, FAN_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
		//UTIL_TraceEntity(this, GetEmitterPosition(), end, MASK_SOLID, &tr);

		// Maximum of 5 iterations
		for(int i = 0; (i < 5 && (tr.m_pEnt || tr.fraction < 1.0f)); i++)
		{
			CBasePlayer * pPlayer = ToBasePlayer(tr.m_pEnt);
			if(!pPlayer)
			{
				IPhysicsObject * pPhys = tr.m_pEnt->VPhysicsGetObject();
				if(pPhys)
					pPhys->ApplyForceCenter((end - tr.m_pEnt->WorldSpaceCenter()) * eo_fan_physics_factor.GetFloat());
			}
			else
			{
				if(pPlayer->GetMoveType() != MOVETYPE_NOCLIP)
				{
					if(pPlayer->GetMoveType() == MOVETYPE_LADDER)
					{
						pPlayer->ExitLadder();
					}

					pPlayer->ApplyAbsVelocityImpulse((end - tr.m_pEnt->WorldSpaceCenter()) * eo_fan_player_factor.GetFloat());
				}
			}

			UTIL_TraceHull(tr.endpos, end, -FAN_VECTOR, FAN_VECTOR, MASK_SHOT_HULL, tr.m_pEnt, COLLISION_GROUP_NONE, &tr);

		}*/
	}

	SetNextRun(gpGlobals->curtime + 0.065f);
}

void COverlordFan::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);

	m_bRunning = false;
	StopSound(FAN_SOUND);
}

#endif

Vector COverlordFan::GetEmitterPosition()
{
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	return BaseClass::GetEmitterPosition() + forw * 18;
}

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordFlamer C_OverlordFlamer
#endif

#define FLAMER_SOUND "d1_town.FlameTrapExtinguish"
#define FLAMER_SOUND_NEW "fire_large"
#define FLAMER_BURST_NEW "FlamerBurstNew"
#define FLAMER_BURST "FlamerBurst"
#define FLAMER_PARTICLE_UNDERWATER "FlamerUnderwater"


ConVar eo_flamer_health("eo_flamer_health", "140", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_distance("eo_flamer_distance", "520.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_counter("eo_flamer_counter", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_flamer_damage("eo_flamer_damage", "40", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_damage("eo_flamer_damage", "7", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_initial_dormant_range("eo_flamer_initial_dormant_range", "530", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_damage_interval("eo_flamer_damage_interval", "0.305", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_interval("eo_flamer_interval", "2.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_interval_min("eo_flamer_interval_min", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_interval_max("eo_flamer_interval_max", "2.25", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_flamer_afterburn_length("eo_flamer_afterburn_length", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_flamer_afterburn_damage("eo_flamer_afterburn_damage", "2", FCVAR_REPLICATED | FCVAR_CHEAT);

//ConVar eo_flamer_slowdown("eo_flamer_slowdown", "165", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_flamer_slowdown_duration("eo_flamer_slowdown_duration", "2.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_slowdown("eo_flamer_slowdown", "90", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_flamer_slowdown_duration("eo_flamer_slowdown_duration", "0.3", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_flamer_force("eo_flamer_force", "1800", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_flamer_old("eo_flamer_old", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

#define FLAMER_RANGE eo_flamer_distance.GetFloat()

#define FLAMER_DAMAGE_MAX eo_flamer_damage.GetFloat()

#define FLAMER_DAMAGE (FLAMER_DAMAGE_MAX / FLAMER_DAMAGE_COUNTER)
#define FLAMER_DAMAGE_DELAY eo_flamer_damage_interval.GetFloat()


#define FLAMER_DAMAGE_COUNTER eo_flamer_counter.GetInt()
#define FLAMER_VECTOR Vector(56, 56, 24)

class COverlordFlamer : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordFlamer, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	COverlordFlamer();
	virtual ~COverlordFlamer();

	virtual void InitializeTrap();
	virtual void DecayTrap();

	virtual error_t CanBuild();

	virtual bool CanBeDormant() const { return m_flNextDamage.Get() == 0.0f; };
	virtual float GetInitialDormantRange() const { return eo_flamer_initial_dormant_range.GetFloat(); };

	virtual float  GetNormalMultiplier() const { return 0; };

	virtual QAngle NormalToAngle(const Vector &normal) const
	{
		return AlignToTop(normal);
	}
#ifdef CLIENT_DLL
	virtual void  FireGameEvent(IGameEvent * event);
	virtual float BeamRange() const { return FLAMER_RANGE; };

	virtual void GhostThink();

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.75f; };
	virtual float GetPanelZDistance() const { return 4.0f; };
	virtual float GetPanelYDistance() const { return -12.0f; };
	
	virtual QAngle GetPanelAngle() const { QAngle angle = BaseClass::GetPanelAngle(); angle[YAW] -= 90; angle[ROLL] -= 90; return angle; };
#else
	virtual void Precache();

	virtual void RunTrap();
	
	virtual void DisableTrap(float flTime);
#endif
private:
#ifndef CLIENT_DLL
	bool CanDamage(CBaseEntity * pEnt) const;
	void StartFlame();
	void StopFlame();

	float GetFlamerDistance() const;
#endif
	CNetworkVar(float, m_flNextDamage);

#ifndef CLIENT_DLL
	int				m_Event;
	float			m_flNextFlame;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordFlamer, DT_OverlordFlamer )

BEGIN_NETWORK_TABLE( COverlordFlamer, DT_OverlordFlamer )
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flNextDamage)),
#else
	RecvPropFloat(RECVINFO(m_flNextDamage)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordFlamer)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_flamer, COverlordFlamer, Flamer, 50, 60,
					"A flamethrowing trap which burst a 10 meters long and 2.0 meter wide flame damaging everyone in its range.",
					"Offense");

COverlordFlamer::COverlordFlamer()
{ 
		//SET_TRAP_MODEL(models/props_c17/trappropeller_engine.mdl);
#ifndef CLIENT_DLL
		m_flNextFlame = 0.0f;
		m_flNextDamage = 0.0f;
#endif
		SET_TRAP_MODEL(models/dynamic_traps/flamer/dynamic_traps_flamer.mdl);
}

COverlordFlamer::~COverlordFlamer()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif

	StopSound(FLAMER_SOUND_NEW);
}

#ifndef CLIENT_DLL
void COverlordFlamer::Precache()
{ 
	BaseClass::Precache(); 
	PrecacheScriptSound(FLAMER_SOUND); 
	PrecacheScriptSound(FLAMER_SOUND_NEW);
	PrecacheParticleSystem(FLAMER_BURST_NEW); 
	PrecacheParticleSystem(FLAMER_PARTICLE_UNDERWATER); 
	PrecacheParticleSystem(FLAMER_BURST);
}

#endif

void COverlordFlamer::InitializeTrap()
{
#ifndef CLIENT_DLL
	m_flNextFlame = gpGlobals->curtime + random->RandomFloat(1.0f, 3.0f);
	SetDestroyable(eo_flamer_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	ResetSequence(LookupSequence("idle"));
#else
	ListenForGameEvent("flamer_start");
	ListenForGameEvent("flamer_stop");
#endif
}

void COverlordFlamer::DecayTrap()
{
	BaseClass::DecayTrap();

	StopSound(FLAMER_SOUND_NEW);
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

error_t COverlordFlamer::CanBuild()
{
	Vector emitter = GetEmitterPosition();
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	Ray_t ray;
	ray.Init(emitter, emitter + forw * (FLAMER_RANGE * 0.75f), -FLAMER_VECTOR, FLAMER_VECTOR);
	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

	bool bFound = false;
	for(int i = 0; i < count; i++)
	{
		CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

		if(pPlayer)
		{
			if(!pPlayer->IsAlive() || !pPlayer->IsRebel() || pPlayer->IsInvisible())
				continue;

			trace_t tr;
			UTIL_TraceLine(emitter, pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt == pPlayer)
			{
				bFound = true;
				break;
			}
		}
	}
	
	if(bFound)
	{
		return CreateError("Rebel is in 3/4 of the flamer's range, cannot build!");
	}

	Vector end;
	VectorMA(GetEmitterPosition(), MAX_TRACE_LENGTH, forw, end);

	{
		trace_t trace;
		UTIL_TraceLine(GetEmitterPosition(), end, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &trace);

		if(trace.allsolid || trace.startsolid || trace.fraction >= 1.0f)
		{
			return CreateError("The Flamer would have no effect!");
		}
	}
	/*
	{
		trace_t trace;
		UTIL_TraceLine(WorldSpaceCenter() + Vector(0, 0, 12), GetEmitterPosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &trace);

		if(trace.fraction != 1.0f)
		{
			error_t error;
			error.canBuild = false;
			Q_strncpy(error.desc, "The Flamer would have no effect!", ARRAYSIZE(error.desc));
			return error;
		}
	}*/

	return BaseClass::CanBuild();
} 
#ifndef CLIENT_DLL
void COverlordFlamer::RunTrap()
{
	StudioFrameAdvance();

	if(!eo_flamer_old.GetBool())
	{
		// Look for players
		Vector forw;
		AngleVectors(GetAbsAngles(), &forw);
		//Vector emitter = WorldSpaceCenter() + forw * 8;
		Vector emitter = GetEmitterPosition() + forw * 4;
		Vector start = emitter + forw * 38;
		Vector end = emitter + forw * GetFlamerDistance();

		CBaseEntity * pList[1024];
		Ray_t ray;
		ray.Init(start, end, -FLAMER_VECTOR, FLAMER_VECTOR);
		int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

		if(m_flNextDamage == 0.0f)
		{
			m_Event = 0;
			for(int i = 0; i < count; i++)
			{
				if(!pList[i] || !pList[i]->IsPlayer())
					continue;

				CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

				if(!pPlayer || !pPlayer->IsRebel() || pPlayer->IsDead())
					continue;

				trace_t tr;
				UTIL_TraceLine(start, pPlayer->RealEyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr); 

				if(tr.m_pEnt != pPlayer)
				{
					UTIL_TraceLine(start, pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr); 
				}

				if(tr.m_pEnt == pPlayer)
				{
					m_flNextDamage = gpGlobals->curtime + 0.1f;
					m_flNextFlame = gpGlobals->curtime;

					StartFlame();

					break;
				}
			}
		}
		else if(m_flNextDamage <= gpGlobals->curtime)
		{
			m_flNextDamage = gpGlobals->curtime + 0.2f;
			bool bFound = false;
			for(int i = 0; i < count; i++)
			{
				if(!pList[i] || !CanDamage(pList[i]))
					continue;

				CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

				if(pPlayer)
				{
					if(!pPlayer || !pPlayer->IsRebel() || pPlayer->IsDead())
						continue;

					trace_t tr;
					UTIL_TraceLine(start, pPlayer->RealEyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr); 

					if(tr.m_pEnt != pPlayer)
					{
						UTIL_TraceLine(start, pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr); 
					}

					if(tr.m_pEnt == pPlayer)
					{
						bFound = true;
						pPlayer->TakeDamage(CTakeDamageInfo(this, this, FLAMER_DAMAGE, DMG_BURN));

						if(!pPlayer->HasSlowdown(this))
							pPlayer->AddSlowdown(eo_flamer_slowdown.GetFloat(), eo_flamer_slowdown_duration.GetFloat(), this, true);
						else
							pPlayer->UpdateSlowdown(eo_flamer_slowdown.GetFloat(), eo_flamer_slowdown_duration.GetFloat(), this, true);

						pPlayer->Ignite(eo_flamer_afterburn_length.GetFloat(), false, 5.0f, false);
					}
					
				}
				else
				{
					CBaseEntity * pEnt = pList[i];

					trace_t tr;
					UTIL_TraceLine(GetEmitterPosition(), pEnt->WorldSpaceCenter(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

					if(tr.m_pEnt == pEnt)
					{
						pEnt->TakeDamage(CTakeDamageInfo(this, this, FLAMER_DAMAGE, DMG_BURN));

						IPhysicsObject * pPhys = pEnt->VPhysicsGetObject();

						if(pPhys)
						{
							pPhys->ApplyForceCenter(forw * eo_flamer_force.GetFloat());
						}
					}
				}
			}

			if(bFound)
				m_Event = 0;
			else
			{
				m_Event++;

				if(m_Event >= eo_flamer_counter.GetInt())
				{
					m_flNextDamage = 0.0f;
					m_Event = 0;
					StopFlame();
				}
			}
		}
	}
	/* --------------------------------------------------------------------------------------------- 
								Old flamer
	   --------------------------------------------------------------------------------------------- */
	else
	{
		// Calculate particle emitter
		Vector forw;
		AngleVectors(GetAbsAngles(), &forw);
		//Vector emitter = WorldSpaceCenter() + forw * 8;
		Vector emitter = GetEmitterPosition() + forw * 4;

		if(m_Event > 0 && m_Event <= FLAMER_DAMAGE_COUNTER && m_flNextDamage <= gpGlobals->curtime)
		{
			// Float it to get float distance
			float distance = (float)FLAMER_RANGE * (float)((float)m_Event/(float)FLAMER_DAMAGE_COUNTER);

			CBaseEntity * pList[1024];

			Ray_t ray;
			ray.Init(emitter + forw * 38, emitter + forw * distance, -FLAMER_VECTOR, FLAMER_VECTOR);
			int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);
			for(int i = 0; i < count; i++)
			{
				if(!pList[i])
					continue;
				
				if(!CanDamage(pList[i]))
					continue;

				bool bCanHit = false;

				CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
				trace_t tr;
				UTIL_TraceLine(emitter, pList[i]->RealEyePosition(), MASK_SHOT_HULL | CONTENTS_WATER
					, &filter, &tr);
		

				if(tr.fraction == 1.0f || tr.m_pEnt == pList[i])
					bCanHit = true;
				else
				{
					UTIL_TraceLine(emitter, pList[i]->WorldSpaceCenter(), MASK_SHOT_HULL | CONTENTS_WATER
					, &filter, &tr);

					if(tr.fraction == 1.0f || tr.m_pEnt == pList[i])
						bCanHit = true;
				}

				if(bCanHit)
				{
					pList[i]->TakeDamage(CTakeDamageInfo(this, this, FLAMER_DAMAGE, DMG_BURN));

					if(pList[i]->IsPlayer())
					{
						CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

						pPlayer->AddSlowdown(eo_flamer_slowdown.GetInt()/(m_Event+1),
							eo_flamer_slowdown_duration.GetFloat(),
							NULL, true, true);
						

					}
					else
					{
						IPhysicsObject * pPhys = pList[i]->VPhysicsGetObject();

						if(pPhys)
						{
							pPhys->ApplyForceCenter(forw * eo_flamer_force.GetFloat());
						}
					}
				}
			}

			m_flNextDamage = gpGlobals->curtime + FLAMER_DAMAGE_DELAY;

			m_Event++;
		}
		else if(m_Event > FLAMER_DAMAGE_COUNTER)
		{
			ResetSequence(LookupSequence("stopFire"));
			m_Event = 0;
		}

		// Flamer doesn't work underwater
		if(m_flNextFlame <= gpGlobals->curtime)
		{
			bool bFound = false;
			CBaseEntity * pList[1024];

			Ray_t ray;

			ray.Init(emitter + forw * 38, emitter + forw * FLAMER_RANGE, -FLAMER_VECTOR, FLAMER_VECTOR);
			int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);
			for(int i = 0; i < count; i++)
			{
				CBaseEntity * pEnt = pList[i];

				if(!pEnt)
					continue;

				if(pEnt->IsPlayer())
				{
					CBasePlayer * pPlayer = ToBasePlayer(pEnt);

					if(pPlayer->IsRebel() && pPlayer->IsAlive() && !pPlayer->IsInvisible())
					{
						bFound = true;
					}
				}
			}

			if(bFound)
			{
				if(!(UTIL_PointContents(GetEmitterPosition()) & (CONTENTS_WATER|CONTENTS_SLIME)))
				{
					//QAngle angle;

					//Vector vAng;
					//WorldToEntitySpace(Vector(1, 0, 0), &vAng);

					//VectorAngles(vAng, angle);
					//DispatchParticleEffect(FLAMER_PARTICLE, emitter, GetAbsAngles());
					//QAngle vecAngle;
					//VectorAngles(forw, vecAngle);

					DispatchParticleEffect(FLAMER_BURST, emitter, GetAbsAngles(), this);
					EmitSound(FLAMER_SOUND);

					m_Event = 1;
					m_flNextDamage = gpGlobals->curtime + FLAMER_DAMAGE_DELAY;

					ResetSequence(LookupSequence("beginFire"));
				}
				else
				{
					// Under water dispatch only particles
					DispatchParticleEffect(FLAMER_PARTICLE_UNDERWATER, emitter, GetAbsAngles());
				}

				m_flNextFlame = gpGlobals->curtime + eo_flamer_interval.GetFloat() + 
					random->RandomFloat(eo_flamer_interval_min.GetFloat(), eo_flamer_interval_max.GetFloat());
			}
		}
	}
	

	
	SetNextRun(gpGlobals->curtime + 0.07f);
}

void COverlordFlamer::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);

	m_Event = 0;
	m_flNextDamage = 0.0f;

	StopFlame();
}

bool COverlordFlamer::CanDamage(CBaseEntity * pEnt) const
{
	if(!pEnt)
		return false;

	// Do not damage breakables
	if( FClassnameIs( pEnt, "func_breakable" ) )
		return false;

	// Don't allow destroying dynamic traps
	if(COverlordTrap::EntityToTrap(pEnt))
		return false;

	return true;
}

void COverlordFlamer::StartFlame()
{
	ResetSequence(LookupSequence("beginFire"));

	EmitSound(FLAMER_SOUND_NEW);

	IGameEvent * event = gameeventmanager->CreateEvent("flamer_start");

	if(event)
	{
		event->SetInt("entindex", entindex());
		gameeventmanager->FireEvent(event);
	}
}

void COverlordFlamer::StopFlame()
{
	ResetSequence(LookupSequence("stopFire"));

	StopSound(FLAMER_SOUND_NEW);

	IGameEvent * event = gameeventmanager->CreateEvent("flamer_stop");

	if(event)
	{
		event->SetInt("entindex", entindex());
		gameeventmanager->FireEvent(event);
	}
}

float COverlordFlamer::GetFlamerDistance() const
{
	if(m_flNextFlame != 0.0f)
	{
		float time = gpGlobals->curtime - m_flNextFlame;

		if(time >= 0.6f)
		{
			return FLAMER_RANGE;
		}
		else
		{
			return FLAMER_RANGE * (time/0.6f);
		}
	}

	return FLAMER_RANGE;
}

#else
void COverlordFlamer::FireGameEvent(IGameEvent * event)
{
	BaseClass::FireGameEvent(event);

	if(event)
	{
		if(event->GetInt("entindex") != entindex())
			return;

		if(!Q_stricmp(event->GetName(), "flamer_start"))
		{
			ParticleProp()->Create(FLAMER_BURST_NEW, PATTACH_POINT_FOLLOW, LookupAttachment("dynamic_trap"));
		}
		else if(!Q_stricmp(event->GetName(), "flamer_stop"))
		{
			ParticleProp()->StopEmission();
		}
	}
}
void COverlordFlamer::GhostThink()
{
	BaseClass::GhostThink();

	Vector vec = FLAMER_VECTOR;
	vec.x = 48.0f;
	vec.z = 20.0f;

	Vector vWorld;
	EntityToWorldSpace(vec, &vWorld);

	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	Vector end = vWorld + forw * FLAMER_RANGE;

	trace_t tr;
	UTIL_TraceLine(vWorld, end, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	
	color32 col;
	col.r = 255;
	col.b = 0;
	col.g = 0;
	col.a = 255;
	DrawBeam(vWorld, tr.endpos, col);

	vec.y = -(FLAMER_VECTOR.y);
	EntityToWorldSpace(vec, &vWorld);

	end = vWorld + forw * FLAMER_RANGE;

	UTIL_TraceLine(vWorld, end, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	
	DrawBeam(vWorld, tr.endpos, col);

}

QAngle COverlordTrap::GetPanelAngle() const
{
	QAngle angle;
	VectorAngles(-GetBeamVector(), angle);

	return angle;
}

void COverlordTrap::DoGlowBlink(Color clr, float flBlinkTime /*= 0.1f*/)
{
	m_flBlinkDisable = gpGlobals->curtime + flBlinkTime;

	if(IsGlowRegistered())
		SetGlowColor(clr);
	else
		RegisterGlow(clr);
}
#endif

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordParalyser C_OverlordParalyser
#endif

#define PARALYSER_SOUND "traps.paralyser_paralyse"

ConVar eo_paralyser_distance("eo_paralyser_distance", "780.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_time("eo_paralyser_time", "2.05", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_slowdown("eo_paralyser_slowdown", "70", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_slowdown_time("eo_paralyser_slowdown_time", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_degree_distance_yaw("eo_paralyser_degree_distance_yaw", "3", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_degree_distance_pitch("eo_paralyser_degree_distance_pitch", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_activation_range("eo_paralyser_activation_range", "800", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_damage("eo_paralyser_damage", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_paralyser_health("eo_paralyser_health", "30", FCVAR_REPLICATED | FCVAR_CHEAT);

#define PARALYSER_RANGE eo_paralyser_distance.GetFloat()
#define PARALYSER_VECTOR Vector(2, 2, 2)

class COverlordParalyser : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordParalyser, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordParalyser();
	virtual ~COverlordParalyser();

	virtual bool CanBeDormant() const { return (m_LastPlayer.Get() == NULL); };
	virtual error_t CanBuild();

	virtual float  GetNormalMultiplier() const { return 1; };
	virtual float  GetInitialDormantRange() const { return eo_paralyser_activation_range.GetFloat(); };
#ifndef CLIENT_DLL
	virtual void Precache();
	virtual void RunTrap();
	virtual void InitializeTrap();
	
	virtual void DecayTrap();
	
	virtual void DisableTrap(float flTime);
#else
	virtual float BeamRange() const { return PARALYSER_RANGE; };

	virtual float GetArcDegrees() const { return eo_paralyser_degree_distance_yaw.GetFloat(); }
	virtual float GetPitchDeegres() const { return eo_paralyser_degree_distance_pitch.GetFloat(); };

	virtual float GetPanelDistanceMultiplier() const { return 0.75f; };
	virtual float GetPanelZDistance() const { return 0.0f; };
	virtual float GetPanelYDistance() const { return -3.0f; };
	virtual QAngle GetPanelAngle() const { QAngle angle = BaseClass::GetPanelAngle(); angle[YAW] -= 90; return angle; };
#endif
private:
	CNetworkVar(CHandle<CBasePlayer>, m_LastPlayer);
#ifndef CLIENT_DLL
	virtual void UpdateBeam();

	// We store our last paralysed player
	CHandle<CBeam> m_Beam;

	float m_flDeparalyse;

	bool m_bState;
#endif
};


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordParalyser, DT_OverlordParalyser )

BEGIN_NETWORK_TABLE( COverlordParalyser, DT_OverlordParalyser )
#ifndef CLIENT_DLL
	SendPropEHandle(SENDINFO(m_LastPlayer)),
#else
	RecvPropEHandle(RECVINFO(m_LastPlayer)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordParalyser)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_paralyser, COverlordParalyser, Paralyser, 35, 55, 
					"Trap which holds anybody who walks into its beam and afterwards slows down the player for a couple of seconds.",
					"Utility");

COverlordParalyser::COverlordParalyser()
{
#ifndef CLIENT_DLL
	m_LastPlayer = NULL;
	m_bState = false;
#endif
	SET_TRAP_MODEL(models/dynamic_traps/paralyser/paralyser.mdl);
}

COverlordParalyser::~COverlordParalyser()
{
#ifndef CLIENT_DLL

	if(m_Beam)
		UTIL_Remove(m_Beam);

	if(m_LastPlayer.Get())
	{
		m_LastPlayer.Get()->SetMoveType(MOVETYPE_WALK);
		m_LastPlayer.Get()->RemoveFrozenRef();
	}
#endif
}

error_t COverlordParalyser::CanBuild()
{
	Vector end, forw;
	AngleVectors(GetAbsAngles(), &forw);		
	VectorMA(GetEmitterPosition(), PARALYSER_RANGE, forw, end);

	trace_t tr;
	UTIL_TraceHull(GetEmitterPosition() + forw * 4, end, -PARALYSER_VECTOR, PARALYSER_VECTOR, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	if(tr.allsolid || tr.startsolid)
	{
		return CreateError("The paralyser will have no effect!");
	}

	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
		return CreateError("Rebel in range, cannot build!");
	}

	return BaseClass::CanBuild();
}
#ifndef CLIENT_DLL
void COverlordParalyser::Precache()
{ 
	BaseClass::Precache();
	PrecacheScriptSound(PARALYSER_SOUND); 
}

void COverlordParalyser::InitializeTrap()
{
	SetDestroyable(eo_paralyser_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	m_Beam = CBeam::BeamCreate("sprites/hydraspinalcord.vmt", 2);

	if(!m_Beam)
	{
		Warning("No laser beam crated!\n");
		return;
	}

	m_bUseDefenses = false;

	m_Beam->SetWidth( 3 );
	m_Beam->SetEndWidth( 2 );
	m_Beam->SetBrightness( 245 );
	m_Beam->SetColor( 80, 120, 150 );
	m_Beam->SetNoise( 1 );

	UpdateBeam();

	// This one is screwed... turning on turns it off and vice versa!
	m_Beam->TurnOn();

	ResetSequence(LookupSequence("idle"));

	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordParalyser::RunTrap()
{
	if((GetCycle() == 1.0f))
	{
		if(GetSequence() == LookupSequence("attackBegin"))
			ResetSequence(LookupSequence("attackIdle"));
		else if(GetSequence() == LookupSequence("attackEnd"))
			ResetSequence(LookupSequence("idle"));
	}

	StudioFrameAdvance();

	// Prepare vectors
	if(!m_bState)
	{
		Vector end, forw;
		AngleVectors(GetAbsAngles(), &forw);		

		Vector finalDir = forw;

		// Get our angles based on players
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
		if(pRebels)
		{
			float maxpitch = eo_paralyser_degree_distance_pitch.GetFloat();
			float maxyaw = eo_paralyser_degree_distance_yaw.GetFloat();
			for(int i = 0; i < pRebels->GetNumPlayers(); i++)
			{
				CBasePlayer * pPlayer = pRebels->GetPlayer(i);

				if(!pPlayer)
					continue;

				if((pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length() > PARALYSER_RANGE)
					continue;

				if(pPlayer->IsDead() || pPlayer->IsInvisible() || (pPlayer->GetFlags() & FL_FROZEN))
					continue;

				trace_t tr;
				UTIL_TraceHull(GetEmitterPosition() + forw * 6, pPlayer->WorldSpaceCenter(), -PARALYSER_VECTOR, PARALYSER_VECTOR, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

				if(tr.m_pEnt != pPlayer)
					continue;

				Vector dir = pPlayer->WorldSpaceCenter() - GetEmitterPosition();
				VectorNormalize(dir);

				Vector relDir, relForw;
				WorldToEntitySpace(forw, &relForw);
				WorldToEntitySpace(dir, &relDir);

				QAngle playerAngle, entityAngle;
				VectorAngles(forw, entityAngle);
				VectorAngles(dir, playerAngle);

				float pitch = fabs(UTIL_AngleDistance(playerAngle[PITCH], entityAngle[PITCH]));
				float yaw = fabs(UTIL_AngleDistance(playerAngle[YAW], entityAngle[YAW]));
				
				if((pitch <= maxpitch) &&
				(yaw <= maxyaw))
				{
					finalDir = dir;
					maxpitch = pitch;
					maxyaw = yaw;
				}
			}
		}



		VectorMA(GetEmitterPosition(), PARALYSER_RANGE, finalDir, end);

		trace_t tr;
		UTIL_TraceHull(GetEmitterPosition() + forw * 6, end, -PARALYSER_VECTOR, PARALYSER_VECTOR, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		// Paralyse a player. Don't paralyse already frozen
		CBasePlayer * pPlayer = ToBasePlayer(tr.m_pEnt);
		if(pPlayer && !pPlayer->IsInvisible() 
			&& !(pPlayer->GetFlags() & FL_FROZEN))
		{
			pPlayer->AddFrozenRef();
			pPlayer->SetAbsVelocity(Vector(0, 0, 0));
			// Emit some sound
			pPlayer->EmitSound(PARALYSER_SOUND);

			// Set movetype to prevent falling
			pPlayer->SetMoveType(MOVETYPE_FLY);

			// Damage
			pPlayer->OnTakeDamage(CTakeDamageInfo(this, this, eo_paralyser_damage.GetFloat(), DMG_GENERIC));

			ResetSequence(LookupSequence("attackBegin"));

			m_bState = true;
			m_LastPlayer = pPlayer;
			if(m_Beam)
			{
				Vector beamEnd;
				VectorMA(tr.endpos, PARALYSER_RANGE - (tr.endpos - GetEmitterPosition()).Length(), finalDir, beamEnd);

				trace_t tra;
				UTIL_TraceHull(tr.endpos, beamEnd, -PARALYSER_VECTOR, PARALYSER_VECTOR, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tra);

				m_Beam->PointsInit(GetEmitterPosition(), tra.endpos);
				m_Beam->RelinkBeam();

				m_Beam->TurnOff();
			}


			color32 paralyseColor = { 0, 0, 125, 185 };
			UTIL_ScreenFade(pPlayer,paralyseColor,2.0f,2.0f, FFADE_IN);

			m_flDeparalyse = gpGlobals->curtime + eo_paralyser_time.GetFloat();

			SetNextRun(gpGlobals->curtime + 0.01f);
			return;
		}

		SetNextRun(gpGlobals->curtime + 0.1f);

		
	}
	else if(m_flDeparalyse <= gpGlobals->curtime)
	{
		// Deparalyse the last player
		if(m_LastPlayer.Get())
		{
			// Revert move type
			m_LastPlayer.Get()->SetMoveType(MOVETYPE_WALK);
			// Slow him down
			m_LastPlayer.Get()->AddSlowdown(eo_paralyser_slowdown.GetInt(), 3.5f, NULL, true);

			// And deparalyse now
			m_LastPlayer.Get()->RemoveFrozenRef();

			m_LastPlayer = NULL;

			SetNextRun(gpGlobals->curtime + eo_paralyser_slowdown_time.GetFloat());
		}
		else
		{
			SetNextRun(gpGlobals->curtime + 0.1f);
		}
		if(m_Beam)
			m_Beam->TurnOn();

		m_bState = false;
		m_flDeparalyse = 0.0f;

		ResetSequence(LookupSequence("attackEnd"));

	}
	else
	{
		if(m_LastPlayer.Get())
		{
			m_LastPlayer.Get()->SetAbsVelocity(Vector(0, 0, 0));
		}

		SetNextRun(gpGlobals->curtime + 0.01f);
	}

	
}

void COverlordParalyser::DecayTrap()
{
	BaseClass::DecayTrap();
	UTIL_Remove(m_Beam);

	if(m_LastPlayer.Get())
	{
		m_LastPlayer.Get()->SetMoveType(MOVETYPE_WALK);
		m_LastPlayer.Get()->RemoveFrozenRef();
	}

}

void COverlordParalyser::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);
	
	if(m_LastPlayer.Get())
	{
		m_LastPlayer.Get()->SetMoveType(MOVETYPE_WALK);
		m_LastPlayer.Get()->RemoveFrozenRef();
	}

	if(m_Beam)
		m_Beam->TurnOn();

}

void COverlordParalyser::UpdateBeam()
{
	// Set position of the beam's start and end point
	const Vector center = GetEmitterPosition();
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	Vector end;
	VectorMA(center, MAX_TRACE_LENGTH, forw, end);

	trace_t tr;
	UTIL_TraceLine(center, end, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	end = tr.endpos;

	m_Beam->PointsInit(center, tr.endpos);
	m_Beam->RelinkBeam();

}
#endif

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordPigeon C_OverlordPigeon
#endif

#define PIGEON_HOMING_SOUND "k_lab.teleport_alarm"
#define PIGEON_GLOW "sprites/glow04_noz.vmt"
#define PIGEON_PARTICLE "PigeonEngine"

ConVar eo_pigeon_speed("eo_pigeon_speed", "1400",  FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_delay("eo_pigeon_delay", "0.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_health("eo_pigeon_health", "25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_activation_range("eo_pigeon_activation_range", "9124", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_pigeon_minimum_distance("eo_pigeon_minimum_distance", "300", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_pigeon_minimum_damage("eo_pigeon_minimum_damage", "20", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_damage("eo_pigeon_damage", "35", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_pigeon_maximum_damage_distance("eo_pigeon_maximum_damage_distance", "1024", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_additional_damage_distance("eo_pigeon_additional_damage_distance", "2048", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_pigeon_additional_damage("eo_pigeon_additional_damage", "15", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_pigeon_acceleration_time("eo_pigeon_acceleration_time", "1.15", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_pigeon_points("eo_pigeon_points", "10", FCVAR_REPLICATED | FCVAR_CHEAT);

#define PIGEON_VECTOR Vector(4, 4, 4)

class COverlordPigeon : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordPigeon, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif
	COverlordPigeon();
	virtual ~COverlordPigeon();
	
	virtual void Precache();

	virtual void InitializeTrap();
	//virtual error_t CanBuild();

	virtual bool  CanRestoreDuration() const { return !m_bState.Get(); };
	virtual bool  CanRestoreHealth() const { return !m_bState.Get(); };

	virtual float  GetNormalMultiplier() const { return 22; };

	virtual float GetInitialDormantRange() const { return eo_pigeon_activation_range.GetFloat(); };

	virtual bool  CanBeDormant() const { return !m_bState; };
#ifdef CLIENT_DLL
	virtual void   TrapThink();

	virtual bool   ShouldUseBeam() const { return true; };
	/*virtual const char * GetHint()
	{
		if(!m_bState.Get() || !C_BasePlayer::GetLocalPlayer()->IsRebel())
			return NULL;

		return "Shoot it down!";
	}*/
	virtual float GetRadiusRange() { return eo_pigeon_minimum_distance.GetFloat(); };

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.6f; };
#else
	virtual bool ShouldDecay() const { return (BaseClass::ShouldDecay() && !m_bState.Get()); };
	float		 GetPigeonSpeed() const;
	void		 PigeonTouch(CBaseEntity * pEntity);
	virtual void RunTrap();
	
	/*virtual void DecayTrap()
	{
		BaseClass::DecayTrap();

		UTIL_Remove(m_hGlow);
	}*/
	virtual void Event_Killed( const CTakeDamageInfo &info );

	virtual bool CanDisableTrap() const { return !m_bState; };

	virtual bool CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
		return true;
	}

	virtual int GetPointsOnDestroyed() const { return eo_pigeon_points.GetInt(); };
#endif
private:
#ifndef CLIENT_DLL
	void Explode();

	float m_flDelay;
	float m_flSoundRestart;
	float m_flTakeoffTime;
	float m_flGracePeriod;
	CHandle<CBasePlayer> m_Target;
	Vector m_vStart;
	//CHandle<CSprite> m_hGlow;
#endif
	CNetworkVar(bool, m_bState)
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordPigeon, DT_OverlordPigeon )

BEGIN_NETWORK_TABLE( COverlordPigeon, DT_OverlordPigeon )
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bState)),
#else
	RecvPropBool(RECVINFO(m_bState)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordPigeon)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_pigeon, COverlordPigeon, Pigeon, 60, 65, "A flying exploding trap which homes at the nearest rebel and carries out a suicidal charge after a short delay.",
					"Offense");

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordPigeon)

	DEFINE_ENTITYFUNC(PigeonTouch),

END_DATADESC()
#endif

COverlordPigeon::COverlordPigeon()
{
#ifndef CLIENT_DLL
	m_flSoundRestart = 0.0f;
	m_flDelay = 0.0f;
	m_vStart = vec3_origin;
	m_flTakeoffTime = 0.0f;
	m_flGracePeriod = 0.0f;
#endif
	SET_TRAP_MODEL(models/dynamic_traps/pigeon/dynamic_traps_pigeon.mdl);
}

COverlordPigeon::~COverlordPigeon()
{
#ifndef CLIENT_DLL
	StopSound(PIGEON_HOMING_SOUND);
//	UTIL_Remove(m_hGlow);
#endif

}

void COverlordPigeon::Precache()
{
	BaseClass::Precache(); 
	PrecacheScriptSound(PIGEON_HOMING_SOUND);
	//PrecacheModel(PIGEON_GLOW);
	PrecacheParticleSystem(PIGEON_PARTICLE);
}


/*error_t COverlordPigeon::CanBuild()
{
	// make sure we cannot home on a player when we are built
#ifndef CLIENT_DLL
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
#else
	C_Team * pRebels = GetGlobalTeam(TEAM_REBELS);
#endif

	if(!pRebels)
		return BaseClass::CanBuild();

	if(UTIL_PointContents(GetEmitterPosition()) & MASK_SHOT_HULL)
	{
		error_t error;
		error.canBuild = false;
		Q_strncpy(error.desc, "The Pigeon would have no effect!", ARRAYSIZE(error.desc));
		return error;
	}

	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer)
			continue;

		error_t error;

		trace_t tr;
		UTIL_TraceLine(GetEmitterPosition(), pPlayer->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt == pPlayer)
		{
			error.canBuild = false;
			Q_strncpy(error.desc, "Pigeon has a line of sight to a player, cannot build", ARRAYSIZE(error.desc));
			return error;
		}

		UTIL_TraceLine(GetEmitterPosition(), pPlayer->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt == pPlayer)
		{
			error.canBuild = false;
			Q_strncpy(error.desc, "Pigeon has a line of sight to a player, cannot build", ARRAYSIZE(error.desc));
			return error;
		}
	}

	return BaseClass::CanBuild();

}*/

#ifdef CLIENT_DLL
void COverlordPigeon::TrapThink()
{
	if(m_bState.Get())
	{
		// Start particle effects											
		if(LookupAttachment("dynamic_traps_emitter01") != -1)
			ParticleProp()->Create(PIGEON_PARTICLE, PATTACH_POINT_FOLLOW, "dynamic_traps_emitter01");
		if(LookupAttachment("dynamic_traps_emitter02") != -1)
			ParticleProp()->Create(PIGEON_PARTICLE, PATTACH_POINT_FOLLOW, "dynamic_traps_emitter02");
		if(LookupAttachment("dynamic_traps_emitter03") != -1)
			ParticleProp()->Create(PIGEON_PARTICLE, PATTACH_POINT_FOLLOW, "dynamic_traps_emitter03");

		SetNextClientThink(CLIENT_THINK_NEVER);
	}
}


#endif

void COverlordPigeon::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_pigeon_health.GetInt());

	/*m_hGlow = CSprite::SpriteCreate(PIGEON_GLOW, WorldSpaceCenter(), false);

	m_bUseDefenses = false;

	if(m_hGlow)
	{
	
		if(m_hGlow)
		{
			m_hGlow->SetTransparency( kRenderGlow, 255, 0, 0, 255, kRenderFxNoDissipation );
			m_hGlow->SetBrightness( 255, 0.2f );
			m_hGlow->SetScale( 1.2f);
		}
	}*/

	SetSolid(SOLID_VPHYSICS);
	SetMoveType(MOVETYPE_FLY);
	SetTouch(&COverlordPigeon::PigeonTouch);
#else
	SetNextClientThink(CLIENT_THINK_ALWAYS);
#endif
}

#ifndef CLIENT_DLL
float COverlordPigeon::GetPigeonSpeed() const
{
	float time = gpGlobals->curtime - m_flTakeoffTime;

	float ratio;
	if(time >= eo_pigeon_acceleration_time.GetFloat())
		ratio = 1.0f;
	else
		ratio = (time/eo_pigeon_acceleration_time.GetFloat());

	return eo_pigeon_speed.GetFloat() * ratio;
}

void COverlordPigeon::PigeonTouch(CBaseEntity * pEntity)
{
	if(m_flGracePeriod >= gpGlobals->curtime)
		return;

	if(m_bState)
		Explode();
}

void COverlordPigeon::RunTrap()
{
	// Check for water
	if((UTIL_PointContents(GetEmitterPosition()) & CONTENTS_SLIME) ||
		(UTIL_PointContents(GetEmitterPosition()) & CONTENTS_WATER) )
	{
		Vector org = GetAbsOrigin();
		QAngle ang = GetAbsAngles();

		CBaseEntity * pEnt = CreateEntityByName("prop_physics_multiplayer");

		if(pEnt)
		{
			pEnt->SetAbsOrigin(org);
			pEnt->SetAbsAngles(ang);
			pEnt->SetModel(GetTrapModel());
			pEnt->Spawn();
		}
		
		UTIL_Remove(this);
		return;
	}
	if(!m_bState)
	{
		// Find the nearest player we have linesight to
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		if(!pRebels)
			return;

		// Nearest player
		CBasePlayer * pNearest = NULL;
		float dist = FLT_MAX;
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer)
				continue;

			trace_t tr;
			UTIL_TraceHull(GetEmitterPosition(), pPlayer->WorldSpaceCenter(), -PIGEON_VECTOR, PIGEON_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

			if(tr.m_pEnt != pPlayer)
				continue;

			float playerdist = (pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length();
			if(playerdist <= eo_pigeon_minimum_distance.GetFloat())
				continue;

			if(playerdist < dist)
			{
				dist = playerdist;
				pNearest = pPlayer;
			}
		}

		if(pNearest)
		{
			if(m_flDelay == 0.0f)
			{
				m_flDelay = gpGlobals->curtime + eo_pigeon_delay.GetFloat();
			}
			else if(m_flDelay <= gpGlobals->curtime)
			{
				// Home on the player and return
				m_bState = true;
				m_Target = pNearest;
				m_flGracePeriod = gpGlobals->curtime + 0.2f;
				// Align yourself
				Vector dir = m_Target->WorldSpaceCenter() - GetAbsOrigin();

				VectorNormalize(dir);

				QAngle angles;
				VectorAngles(dir, angles);
				SetAbsAngles(angles);

				EmitSound(PIGEON_HOMING_SOUND);

				m_flSoundRestart = gpGlobals->curtime + 0.3f;

				ReleaseTrap();

				IGameEvent * event = gameeventmanager->CreateEvent("pigeon_fly");

				if(event)
				{
					event->SetInt("entindex", entindex());
					event->SetInt("userid", m_Target->GetUserID());
					gameeventmanager->FireEvent(event);
				}

				m_vStart = GetAbsOrigin();
				m_flTakeoffTime = gpGlobals->curtime;
				//DispatchParticleEffect(PIGEON_PARTICLE, PATTACH_ABSORIGIN_FOLLOW, this);
			}
		}
		else
		{
			if(m_flDelay <= gpGlobals->curtime)
			{
				m_flDelay = 0.0f;
			}
		}
	}
	else
	{

		// RESTART SOUND! Otherwise, if the piegon attacks outside of the PAS, the sound won't be played
		// when it gets closer
		if(m_flSoundRestart <= gpGlobals->curtime)
		{
			StopSound(PIGEON_HOMING_SOUND);
			EmitSound(PIGEON_HOMING_SOUND);

			m_flSoundRestart = gpGlobals->curtime + 0.3f;
		}

		const float speed = (gpGlobals->curtime - GetLastRun()) * GetPigeonSpeed();
		Vector dir(1, 0, 0);
		if(!m_Target || m_Target->IsDead() || m_Target->GetTeamNumber() != TEAM_REBELS)
		{
			// Fly in direct line
			AngleVectors(GetAbsAngles(), &dir);

			m_Target = NULL;
		}
		else
		{
			// Check whether the player's visible
			trace_t tr;
			UTIL_TraceLine(GetEmitterPosition(), m_Target->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt == m_Target)
			{
				// Fly towards the player
				dir = m_Target->WorldSpaceCenter() - GetAbsOrigin();
			}
			else
			{
				// Just fly straight until he comes back in view...
				AngleVectors(GetAbsAngles(), &dir);
			}
		}
		// Collision detection here

		VectorNormalize(dir);

		// Now move
		dir *= speed;

		// Align ourselves
		QAngle aDir;
		VectorAngles(dir, aDir);

		SetAbsAngles(aDir);
		SetAbsOrigin(GetAbsOrigin() + dir);
	}

	SetNextRun(gpGlobals->curtime + GENERAL_THINK);
}

void COverlordPigeon::Event_Killed(const CTakeDamageInfo &info)
{ 
	StopSound(PIGEON_HOMING_SOUND); 
	BaseClass::Event_Killed(info); 

	if(!IsDecaying())
		Explode(); 
}


void COverlordPigeon::Explode()
{
	// TODO: This is all so messy... rework and comment it
	float damage = eo_pigeon_damage.GetFloat();
	float dist = (GetEmitterPosition() - m_vStart).Length();

	if(dist <= 0.0f)
		dist = 0.0f;

	if(dist != 0.0f)
	{
		if(m_vStart != vec3_origin)
		{
			if(dist < (eo_pigeon_maximum_damage_distance.GetFloat()))
			{
				damage = damage * ( dist / eo_pigeon_maximum_damage_distance.GetFloat() );
			}
			else
			{
				// Count additional damage
				if(dist >= eo_pigeon_additional_damage_distance.GetFloat())
					damage += eo_pigeon_additional_damage.GetFloat();
				else
					damage += eo_pigeon_additional_damage.GetFloat() * (dist / eo_pigeon_additional_damage_distance.GetFloat());
			}

		}
	}

	if(damage < eo_pigeon_minimum_damage.GetFloat())
		damage = eo_pigeon_minimum_damage.GetFloat();

	UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 3.5f, 128, SHAKE_START);
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	
	StopSound(PIGEON_HOMING_SOUND);

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), this, damage, damage * 2, true, 30 );

	UTIL_Remove(this);
	SetNextRun(NEVER_RUN);
}
#endif

//====================================================================================================================================


#ifdef CLIENT_DLL
#define COverlordBomber C_OverlordBomber
#endif

#define BOMBER_SOUND "NPC_RollerMine.Tossed"

ConVar eo_bomber_search_distance("eo_bomber_search_distance", "2096", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_bomber_activation_range("eo_bomber_activation_range", "120", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_bomber_explosion_time("eo_bomber_explosion_time", "0.45", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_bomber_damage("eo_bomber_damage", "90", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_bomber_switch_distance("eo_bomber_switch_distance", "85", FCVAR_CHEAT | FCVAR_REPLICATED);

//ConVar eo_bomber_health("eo_bomber_health", "60", FCVAR_CHEAT | FCVAR_REPLICATED);

#define BOMBER_DAMAGE eo_bomber_damage.GetFloat()
#define BOMBER_EXPLOSION_TIME eo_bomber_explosion_time.GetFloat()

namespace
{
	struct BoobyTrapModels
	{
		BoobyTrapModels(const char * copy, int rhsHealth)
		{
			Q_strncpy(modelName, copy, ARRAYSIZE(modelName));
			health = rhsHealth;
		};
		char modelName[64];
		int health;
	};

	BoobyTrapModels s_BoobyTrapModels[] =
	{
		BoobyTrapModels("models/props_junk/wood_crate001a.mdl", 60),
		BoobyTrapModels("models/props_junk/wood_crate002a.mdl", 60),
		BoobyTrapModels("models/props_junk/wood_pallet001a.mdl", 40),
		BoobyTrapModels("models/props_debris/metal_panel01a.mdl", 0),
		BoobyTrapModels("models/props_debris/metal_panel02a.mdl", 0),
		BoobyTrapModels("models/props_c17/oildrum001.mdl", 0),
		//BoobyTrapModels("models/props_borealis/bluebarrel001.mdl", 0),
		//BoobyTrapModels("models/props_borealis/bluebarrel001_chunk01.mdl", 0),
	};
}
#ifdef CLIENT_DLL
class COverlordBomber : public COverlordTrap
#else
class COverlordBomber : public COverlordTrap
#endif
{
public:
	DECLARE_CLASS(COverlordBomber, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordBomber();
	virtual ~COverlordBomber();

	virtual void Precache();

	virtual bool  CanRestoreDuration() const { return !m_bExploding; };
	virtual void  ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
	{
		CBaseEntity::ImpactTrace(pTrace, iDamageType, pCustomImpactName);
	}

	virtual float GetNormalMultiplier() const { return 26; };

	virtual bool  UseLOSForInitialDormant() const { return false; };
	virtual bool  ShouldGib() const { return false; };

#ifdef CLIENT_DLL
	virtual bool ShouldUseBeam() const { return false; };
	virtual int  DrawModel(int flags);
	virtual void GhostThink();

	// Trap panel variables
	virtual float GetPanelZDistance() const { return 4.0f; };
#else
	virtual bool CanMark() const { return true; };

	virtual void InitializeTrap();

	virtual void DecayTrap();
	virtual void RunTrap();
	virtual void Event_Killed(const CTakeDamageInfo & info);

	virtual bool ShouldDecay() const { return (BaseClass::ShouldDecay() && !m_bExploding); };

	virtual bool CreateVPhysics() { VPhysicsInitNormal( SOLID_VPHYSICS, 0, false ); return true; };

	virtual bool CanDisableTrap() const { return !IsExploding(); };
	virtual bool ShouldFireZaps() const { return false; };
#endif
private:
	bool IsExploding() const { return m_bExploding.Get(); };
	CBaseEntity * FindNearestPhysicsEntity(bool * bSwap) const;
#ifndef CLIENT_DLL
	void Explode()
	{
		StopSound(BOMBER_SOUND);

		ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), this, BOMBER_DAMAGE, BOMBER_DAMAGE * 2, true, 190 );
	
		UTIL_Remove(this);
	}

	bool m_bIdle;
	float m_flExplode;
#else
	CHandle<C_BaseEntity> m_Glow;
	float m_flNextGhostUpdate;
#endif
	CNetworkVar(bool, m_bExploding);
};


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordBomber, DT_OverlordBomber )

BEGIN_NETWORK_TABLE( COverlordBomber, DT_OverlordBomber )
#ifndef CLIENT_DLL
	SendPropBool(SENDINFO(m_bExploding)),
#else
	RecvPropBool(RECVINFO(m_bExploding)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordBomber)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_bomber, COverlordBomber, Booby trap, 25, 75, "A bomb disguised as the nearest pushable object. Blows up when moved.",
					"Defense");

COverlordBomber::COverlordBomber()
{
	SET_TRAP_MODEL_FROM_VAR(s_BoobyTrapModels[0].modelName);
#ifdef CLIENT_DLL
	m_Glow = NULL;
	m_flNextGhostUpdate = 0.0f;
#endif
}
COverlordBomber::~COverlordBomber()
{
	// Just so we don't end up with the pigeon sound problem ;)
#ifndef CLIENT_DLL
	StopSound(BOMBER_SOUND);
	m_bUseDefenses = false;
#else
	if(m_Glow)
		m_Glow->DeregisterGlow();
#endif	
};
void COverlordBomber::Precache()
{
	BaseClass::Precache(); 
	PrecacheScriptSound(BOMBER_SOUND); 
	
	// Precache all of our models. Expensive but needed...
	for(int i = 0; i < ARRAYSIZE(s_BoobyTrapModels); i++)
	{
		PrecacheModel(s_BoobyTrapModels[i].modelName);
	}
};

#ifdef CLIENT_DLL
int COverlordBomber::DrawModel(int flags)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
	if(((pPlayer && pPlayer->IsOverlord()) /*|| IsExploding()*/))
	{
		if(GetRenderMode() != kRenderTransColor)
		{
			SetRenderMode(kRenderTransColor);
			SetRenderColor(230, 0, 0, 240);
		}
	}
	else if(GetRenderMode() != kRenderNormal)
	{
		SetRenderColor(255, 255, 255, 255);
		SetRenderMode(kRenderNormal);
	}
	return BaseClass::DrawModel(flags);
}

void COverlordBomber::GhostThink()
{
	BaseClass::GhostThink();

	if(m_flNextGhostUpdate <= gpGlobals->curtime)
	{
		bool bSwap;
		CBaseEntity * pSearch = FindNearestPhysicsEntity(&bSwap);

		if(bSwap)
		{
			if(m_Glow && (m_Glow != pSearch))
				m_Glow->DeregisterGlow();

			if(pSearch)
				pSearch->RegisterGlow(Color(255, 255, 255));

			m_Glow = pSearch;
		}
		else if(m_Glow)
		{
			m_Glow->DeregisterGlow();
			m_Glow = NULL;
		}

		if(pSearch)
		{
			if(Q_strcmp(modelinfo->GetModelName(GetModel()), modelinfo->GetModelName(pSearch->GetModel())))
			{
				SetModel(modelinfo->GetModelName(pSearch->GetModel()));
			}
		}
		else
		{
			SetModel(s_BoobyTrapModels[0].modelName);
		}
		
	
		m_flNextGhostUpdate = gpGlobals->curtime + 0.1f;
	}
}
#else
void COverlordBomber::InitializeTrap()
{
	// PHYSICED! Release!
	ReleaseTrap();

	//SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
	//	eo_traps_explosion_force.GetFloat(), true);
 
	bool bSwap;
	CBaseEntity * pNearest = FindNearestPhysicsEntity(&bSwap);

	if(pNearest)
	{
		SetModel(STRING(pNearest->GetModelName()));

		if(pNearest->m_takedamage == DAMAGE_YES)
			SetDestroyable(pNearest->GetHealth());
		else
			SetDestroyable(0);

		// Set our angles to the nearest entity's!
		if(bSwap)
		{	
			UTIL_Remove(pNearest);
			SetAbsAngles(pNearest->GetAbsAngles());
			SetAbsOrigin(pNearest->GetAbsOrigin());
			pNearest = NULL;
		}	
		//
	}
	else
	{
		// No nearby physic entities, select one random from the table
		int rModel = random->RandomInt(0, ARRAYSIZE(s_BoobyTrapModels) - 1);
		SetModel(s_BoobyTrapModels[rModel].modelName);

		SetDestroyable(s_BoobyTrapModels[rModel].health);
	}
	
	SetSolid(SOLID_VPHYSICS);

	SetMoveType(MOVETYPE_PUSH);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	CreateVPhysics();

	m_bExploding = false;

	m_bUseDefenses = false;

	if(!VPhysicsGetObject())
	{
		Warning("No physics object, deleting!\n");
		UTIL_Remove(this);
		SetNextRun(NEVER_RUN);
		return;
	}
	
	m_bIdle = false;

	SetNextRun(gpGlobals->curtime + 2.0f);
}

void COverlordBomber::DecayTrap()
{
	BaseClass::DecayTrap();

	if(!IsInInitialDormant())
	{
		CPhysicsProp * pPhys = static_cast<CPhysicsProp*>(CreateEntityByName("prop_physics_multiplayer"));

		if(pPhys)
		{
			UTIL_Remove(this);
			pPhys->SetAbsOrigin(GetAbsOrigin());
			pPhys->SetAbsAngles(GetAbsAngles());
			pPhys->SetModelName(GetModelName());
			pPhys->SetHealth(GetHealth());
			pPhys->m_takedamage = m_takedamage;
			pPhys->Spawn();
		}
	}
}
void COverlordBomber::RunTrap()
{
	if(!IsExploding())
	{
		AngularImpulse impulse;
		Vector velocity;

		if(VPhysicsGetObject())
		{
			VPhysicsGetObject()->GetVelocity(&velocity, &impulse);

			if((velocity.LengthSqr() != 0.0f || impulse.LengthSqr() != 0.0f) && m_bIdle)
			{
				bool bFound = false;
				// Make sure a player is in range
				for(int i = 1; i <= gpGlobals->maxClients; i++)
				{
					CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

					if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive())
						continue;

					if((pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_bomber_activation_range.GetFloat())
					{
						bFound = true;
						break;
					}
				}

				if(bFound)
				{
					m_bExploding = true;
					m_flExplode = gpGlobals->curtime + BOMBER_EXPLOSION_TIME;
					EmitSound(BOMBER_SOUND);
				}
			}
			else if(velocity.Length() == 0.0f && impulse.Length() == 0.0f && !m_bIdle)
			{
				m_bIdle = true;
			}
		}
		else
		{
			SetNextRun(gpGlobals->curtime + 500.0f);
			UTIL_Remove(this);
			return;
		}
	}
	else
	{
		if(m_flExplode <= gpGlobals->curtime)
		{
			Explode();
			return;
		}
	}

	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordBomber::Event_Killed(const CTakeDamageInfo & info)
{
	BaseClass::Event_Killed(info);

	if(!IsDecaying())
	{
		CPhysicsProp * pProp = static_cast<CPhysicsProp*>(CreateEntityByName("prop_physics"));

		if(pProp)
		{
			pProp->SetAbsOrigin(GetAbsOrigin());
			pProp->SetAbsAngles(GetAbsAngles());
			pProp->SetHealth(2);
			pProp->SetMaxHealth(2);
			pProp->m_takedamage = DAMAGE_YES;
			pProp->SetModel(STRING(GetModelName()));
			pProp->Spawn();


			pProp->Event_Killed(CTakeDamageInfo(this, this, 100000, DMG_GENERIC));
		}
	}
}
#endif

CBaseEntity * COverlordBomber::FindNearestPhysicsEntity(bool * bSwap) const
{
	const float SEARCH_DISTANCE = eo_bomber_search_distance.GetFloat();
	float flDist = SEARCH_DISTANCE*SEARCH_DISTANCE;
	CBaseEntity * pReturn = NULL;
#ifndef CLIENT_DLL
	{
		CBaseEntity * pSearch = NULL;
		while((pSearch = gEntList.FindEntityByClassname(pSearch, "prop_physics")) != NULL)
		{
			IPhysicsObject * pObj = pSearch->VPhysicsGetObject();

			if(!pObj)
				continue;

			if(!pObj->IsMoveable())
				continue;

			if(pSearch->GetParent())
				continue;

			if(flDist >= (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr())
			{
				flDist = (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr();
				pReturn = pSearch;
			}
		}
	}

	{
		CBaseEntity * pSearch = NULL;
		while((pSearch = gEntList.FindEntityByClassname(pSearch, "prop_physics_multiplayer")) != NULL)
		{
			IPhysicsObject * pObj = pSearch->VPhysicsGetObject();

			if(!pObj)
				continue;

			if(!pObj->IsMoveable())
				continue;

			if(pSearch->GetParent())
				continue;

			if(flDist >= (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr())
			{
				flDist = (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr();
				pReturn = pSearch;
			}
		}
	}

	{
		CBaseEntity * pSearch = NULL;
		while((pSearch = gEntList.FindEntityByClassname(pSearch, "prop_physics_respawnable")) != NULL)
		{
			IPhysicsObject * pObj = pSearch->VPhysicsGetObject();

			if(!pObj)
				continue;

			if(!pObj->IsMoveable())
				continue;

			if(pSearch->GetParent())
				continue;

			if(flDist >= (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr())
			{
				flDist = (GetAbsOrigin() - pSearch->GetAbsOrigin()).LengthSqr();
				pReturn = pSearch;
			}
		}
	}
#else
	C_BaseEntity * pList[1024];
	int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), GetAbsOrigin(), SEARCH_DISTANCE, 0);

	for(int i = 0; i < count; i++)
	{
		CBaseEntity * pEnt = pList[i];

		if(!pEnt)
			continue;	

		const char * classname = pEnt->GetClassname() + 6;
		if(Q_stricmp(classname, "CPhysicsPropMultiplayer"))
			continue;

		C_PhysicsProp * pPhys = static_cast<C_PhysicsProp*>(pEnt);

		if(pPhys->IsConstrained())
			continue;

		if((pEnt->GetAbsOrigin() - GetAbsOrigin()).LengthSqr() <= flDist)
		{
			flDist = (pEnt->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			pReturn = pEnt;
		}
		
		
	}
#endif

	if(pReturn && (bSwap != NULL))
	{
		if((pReturn->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_bomber_switch_distance.GetFloat())
			*bSwap = true;
		else
			*bSwap = false;
	}

	return pReturn;
}


//====================================================================================================================================


#ifdef CLIENT_DLL
#define COverlordSticky C_OverlordSticky
#endif

ConVar eo_sticky_distance("eo_sticky_distance", "145.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_initial_dormant_range("eo_sticky_initial_dormant_range", "165", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_explosion_time("eo_sticky_explosion_time", "4.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_damage("eo_sticky_damage", "80", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_health("eo_sticky_health", "15", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_points("eo_sticky_points", "30", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_sticky_slowdown("eo_sticky_slowdown", "100", FCVAR_REPLICATED | FCVAR_CHEAT);

#define STICKY_DAMAGE eo_sticky_damage.GetFloat()
#define STICKY_DISTANCE eo_sticky_distance.GetFloat()
#define STICKY_TIME eo_sticky_explosion_time.GetFloat()
#define STICKY_SOUND "traps.sticky_tic"
#define STICKY_GLOW "sprites/glow04_noz.vmt"

class COverlordSticky : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordSticky, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	COverlordSticky();
	virtual ~COverlordSticky();

	virtual void Precache();
	virtual error_t CanBuild();
	
	virtual bool CanBeDormant() const { return m_bJumping != true; };

	virtual float GetInitialDormantRange() const { return eo_sticky_initial_dormant_range.GetFloat(); };
	
	virtual bool  CanRestoreDuration() const { return !m_bJumping.Get(); };

	virtual float GetNormalMultiplier() const { return 4.0f; };
	virtual QAngle NormalToAngle(const Vector&normal) const
	{
		return AlignToTop(normal);
	}

	virtual Vector GetBeamVector() const;
#ifdef CLIENT_DLL
	virtual float GetRadiusRange() { return STICKY_DISTANCE; };

	virtual const char * GetHint();

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.85f; };
	virtual float GetPanelZDistance() const { return 2.0f; };
#else
	virtual int	ObjectCaps(void)
	{
		return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE);
	
	}

	virtual void InitializeTrap();
	virtual void RunTrap();
	virtual bool ShouldDecay() const { return (BaseClass::ShouldDecay() && !m_bJumping); };

	virtual int OnTakeDamage(const CTakeDamageInfo &info)
	{
		if( m_Target.Get() == info.GetAttacker())
			return 0;

		if(info.GetAttacker() && !info.GetAttacker()->IsPlayer())
			return 0;

		return BaseClass::OnTakeDamage(info);
	}

	virtual void DecayTrap()
	{
		BaseClass::DecayTrap();

		UTIL_Remove(m_hGlow);

		StopSound(STICKY_SOUND);
	}

	virtual bool CanDisableTrap() const { return !GetTarget(); };

	virtual int  GetPointsOnDestroyed() const { return eo_sticky_points.GetInt(); };
#endif
	CBasePlayer * GetTarget() const { return m_Target.Get(); };
	bool		  IsExploding() const { return m_bJumping.Get(); };
private:
#ifdef CLIENT_DLL
	virtual bool ShouldUseBeam() const { return false; };
#else
	void UpdatePosition();
	void Explode();

	CHandle<CSprite> m_hGlow;
	float m_flExplosion;
#endif
	CNetworkVar(CHandle<CBasePlayer>, m_Target);
	CNetworkVar(bool, m_bJumping);


};	

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordSticky)

	DEFINE_USEFUNC(Use),

END_DATADESC()
#endif
IMPLEMENT_NETWORKCLASS_ALIASED( OverlordSticky, DT_OverlordSticky )

BEGIN_NETWORK_TABLE( COverlordSticky, DT_OverlordSticky )
#ifndef CLIENT_DLL
	SendPropEHandle(SENDINFO(m_Target)),
	SendPropBool(SENDINFO(m_bJumping)),
#else
	RecvPropEHandle(RECVINFO(m_Target)),
	RecvPropEHandle(RECVINFO(m_bJumping)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordSticky)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_sticky, COverlordSticky, Sticky, 60, 50, "This grenade sticks to a player, explodes after 6 seconds damaging anyone in its range. Can be disabled by any player other than the carrier.",
					"Defense");


COverlordSticky::COverlordSticky()
{ 
	SET_TRAP_MODEL(models/dynamic_traps/stickybomb/stickybomb.mdl); 
#ifndef CLIENT_DLL
	m_hGlow = NULL;
#endif

}
COverlordSticky::~COverlordSticky()
{
#ifndef CLIENT_DLL
	if(m_Target.Get())
	{
		m_Target.Get()->RemoveSlowdown(this);
	}
	UTIL_Remove(m_hGlow);
#endif

	StopSound(STICKY_SOUND);

}

void COverlordSticky::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(STICKY_SOUND);

	PrecacheModel(STICKY_GLOW);
}

error_t COverlordSticky::CanBuild()
{
	if(IsPlayerInRadius(STICKY_DISTANCE + PLAYER_RADIUS))
	{
		return CreateError("Player in radius, cannot build");
	}

	if((UTIL_PointContents(GetEmitterPosition()) & MASK_SHOT_HULL) ||
		(UTIL_PointContents(WorldSpaceCenter()) & MASK_SHOT_HULL))
	{
		return CreateError("The trap would have no effect.");
	}
	

	return BaseClass::CanBuild();
}

Vector COverlordSticky::GetBeamVector() const
{
	Vector forw, left, up;
	AngleVectors(GetAbsAngles(), &forw, &left, &up);

	return -left;
}
#ifdef CLIENT_DLL
const char * COverlordSticky::GetHint()
{
	if(!IsExploding() || !C_BasePlayer::GetLocalPlayer()->IsRebel())
		return NULL;

	if(GetTarget() == C_BasePlayer::GetLocalPlayer())
		return "Let other players destroy the bomb.";

	return "Shoot or use to disable the bomb.";
}
#endif

#ifndef CLIENT_DLL

void COverlordSticky::InitializeTrap()
{
	m_bUseDefenses = false;

	SetSolid(SOLID_VPHYSICS);

	SetCollisionGroup(COLLISION_GROUP_DEBRIS);

	SetDestroyable(eo_sticky_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	SetUse(&COverlordSticky::Use);
}

void COverlordSticky::RunTrap()
{
	SetNextRun(gpGlobals->curtime + GENERAL_THINK);

	if(!m_bJumping)
	{
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		if(!pRebels)
			return;

		CBasePlayer * pTarget = NULL;
		float flDist = FLT_MAX;
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || pPlayer->IsInvisible())
				continue;

			float dist = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length();
			if(dist <= STICKY_DISTANCE)
			{
				trace_t tr;
				UTIL_TraceHull( GetEmitterPosition(), pPlayer->WorldSpaceCenter(), -Vector(2,2,2), Vector(2,2,2), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr );

				if(tr.m_pEnt != pPlayer || tr.startsolid)
					continue;

				// Check whether he is some other sticky's target
				bool bContinue = false;
				for(int j = 0; j < GetOverlordData()->GetMaxTraps(); j++)
				{
					COverlordTrap * pTrap = GetOverlordData()->GetTrap(j);

					if(!pTrap)
						continue;

					if(!Q_stricmp(pTrap->GetClassname(), "trap_sticky"))
					{
						COverlordSticky * pSticky = static_cast<COverlordSticky*>(pTrap);

						if(pSticky->GetTarget() == pPlayer)
						{
							bContinue = true;	
							break;
						}
					}
				}

				if(bContinue)
					continue;

				if(dist < flDist)
				{
					flDist = dist;
					pTarget = pPlayer;
				}
			}
		}

		if(pTarget)
		{
			// Jump here
			m_bJumping = true;
			m_Target = pTarget;
			
			EmitSound(STICKY_SOUND);

			m_Target.Get()->AddSlowdown(eo_sticky_slowdown.GetFloat(), -1.0f, this);

			m_hGlow = CSprite::SpriteCreate(STICKY_GLOW, GetAbsOrigin(), false );

			if(m_hGlow)
			{
				m_hGlow->SetAttachment( this, LookupAttachment( "light" ) );
				m_hGlow->SetTransparency( kRenderWorldGlow, 0, 0, 255, 240, kRenderFxNoDissipation );
				m_hGlow->SetBrightness( 255 );
				m_hGlow->SetScale( 0.85f);
			}

			m_flExplosion = gpGlobals->curtime + STICKY_TIME;
			SetExplodable(false);

			ReleaseTrap();

			UpdatePosition();

			return;
		}
	}
	else
	{
		UpdatePosition();

		// Initial checks
		if(!m_Target.Get() || m_Target.Get()->IsDead() || !m_Target.Get()->IsRebel())
		{
			Explode();
			return;
		}

		if(m_flExplosion < gpGlobals->curtime)
		{
			Explode();
			return;
		}
		
	}
}
void COverlordSticky::UpdatePosition()
{
	// Position first
	Vector vEntity(16, 0, 40);
	// Our target position in world coords
	Vector vWorld;

	VectorTransform(vEntity, m_Target.Get()->EntityToWorldTransform(), vWorld);	

	SetAbsOrigin(vWorld);

	// Angles
	
	QAngle final;
	QAngle ang = m_Target.Get()->GetAbsAngles();

	// Necessary to turn it around
	final.x = 90;
	final.y = ang.y;
	final.z = ang.z;

	SetAbsAngles(final);
}
void COverlordSticky::Explode()
{
	if(m_Target.Get())
	{
		m_Target.Get()->RemoveSlowdown(this);
	}
	m_Target = NULL;

	UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 3.5f, 180, SHAKE_START);
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	
	StopSound(STICKY_SOUND);

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), this, STICKY_DAMAGE, STICKY_DAMAGE * 2, true, NULL, CLASS_NONE, 90 );

	UTIL_Remove(this);
}

#endif

//====================================================================================================================================

/*#ifdef CLIENT_DLL
#define COverlordUnstealther C_OverlordUnstealther
#endif

ConVar eo_unstealther_distance("eo_unstealther_distance", "290.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_unstealther_initial_dormant_range("eo_unstealther_initial_dormant_range", "325", FCVAR_REPLICATED | FCVAR_CHEAT);

#define UNSTEALTHER_RADIUS eo_unstealther_distance.GetFloat()
#define UNSTEALTHER_SOUND "NPC_SScanner.FlyLoop"

class COverlordUnstealther : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordUnstealther, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordUnstealther() 
	{ 
		SET_TRAP_MODEL(models/dynamic_traps/unstealther/dynamic_traps_unstealther.mdl);
	};
	virtual ~COverlordUnstealther() 
	{ 
		StopSound(UNSTEALTHER_SOUND);
	};

	virtual void Precache()
	{
		BaseClass::Precache();

		PrecacheScriptSound(UNSTEALTHER_SOUND);
	}

	virtual error_t CanBuild();

	virtual void SetTrapDormant(bool bDormant)
	{
		BaseClass::SetTrapDormant(bDormant);

		if(bDormant)
			StopSound(UNSTEALTHER_SOUND);
		else
			EmitSound(UNSTEALTHER_SOUND);
			
	}

	virtual float GetInitialDormantRange() const { return eo_unstealther_initial_dormant_range.GetFloat(); };

	virtual float GetNormalMultiplier() const { return 0; };
	virtual QAngle NormalToAngle(const Vector &normal) const;
	
	virtual Vector GetEmitterPosition();
#ifndef CLIENT_DLL
	virtual void RunTrap();
	virtual void InitializeTrap() 
	{
		SetDestroyable(eo_traps_defaulthealth.GetInt());
		SetExplodable(20, 128, 90, true);
		ResetSequence( LookupSequence( "Idle" ) );

		EmitSound(UNSTEALTHER_SOUND);
	}

	virtual void DecayTrap()
	{
		BaseClass::DecayTrap();

		StopSound(UNSTEALTHER_SOUND);
	}
#else
	virtual Vector GetRadiusOrigin();
	virtual float GetRadiusRange() { return UNSTEALTHER_RADIUS; };

	virtual bool ShouldUseBeam() const { return false; };
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordUnstealther, DT_OverlordUnstealther )

BEGIN_NETWORK_TABLE( COverlordUnstealther, DT_OverlordUnstealther )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordUnstealther)
END_PREDICTION_DATA()
#endif

//LINK_TRAP_TO_ENTITY(trap_unstealther, COverlordUnstealther, Unstealther, 57, 60, "Uncloaks any invisible player in the radius and depletes their invisibility charge.");

error_t COverlordUnstealther::CanBuild()
{
	if(IsPlayerInRadius(UNSTEALTHER_RADIUS + PLAYER_RADIUS))
	{
		error_t error;
		error.canBuild = false;
		Q_strncpy(error.desc, "Player is in radius, cannot build.", ARRAYSIZE(error.desc));
		return error;
	}

	if(UTIL_PointContents(GetEmitterPosition()) & MASK_SHOT_HULL)
	{
		error_t error;
		error.canBuild = false;
		Q_strncpy(error.desc, "The trap would have no effect.", ARRAYSIZE(error.desc));
		return error;
	}

	return BaseClass::CanBuild();
}

#ifndef CLIENT_DLL
void COverlordUnstealther::RunTrap()
{
	StudioFrameAdvance();
	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), GetEmitterPosition(), UNSTEALTHER_RADIUS, MASK_ALL);

	if(count > 0)
	{
		for(int i = 0; i < count; i++)
		{
			CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

			if(!pPlayer)
				continue;

			if(!pPlayer->IsInvisible() || !pPlayer->IsRebel())
				continue;

			// Make sure we get line of sight
			trace_t tr;
			UTIL_TraceLine(GetEmitterPosition(), pPlayer->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
			if(tr.m_pEnt != pPlayer)
				continue;

			pPlayer->UnmakeInvisible();

			// Since current weapon must be the stealth gun we can safely do this
			if(pPlayer->GetPlayerClass() == CLASS_STEALTH)
			{
				CBaseCombatWeapon * pWeapon = pPlayer->GetActiveWeapon();
				if(pWeapon)
				{	
					pWeapon->m_iClip1 = (float)pWeapon->m_iClip1 * 0.20f;
				}
			}
			

		}
	}

	SetNextRun(gpGlobals->curtime + 0.1f);
}
#else
Vector COverlordUnstealther::GetRadiusOrigin()
{
	Vector vUp;
	VectorRotate(Vector(0, 0, 1), EntityToWorldTransform(), vUp);
	VectorNormalize(vUp);

	Vector center = WorldSpaceCenter() + vUp * 10;

	return center;
}
#endif
QAngle COverlordUnstealther::NormalToAngle(const Vector &normal) const
{
	return AlignToTop(normal);
}

Vector COverlordUnstealther::GetEmitterPosition()
{
	Vector vUp;
	VectorRotate(Vector(0, 0, 1), EntityToWorldTransform(), vUp);
	VectorNormalize(vUp);

	return GetAbsOrigin() + vUp * 20;
}*/

//====================================================================================================================================

#ifndef CLIENT_DLL

ConVar eo_traps_airstrike_round_magnitude("eo_traps_airstrike_round_magnitude", "90", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_airstrike_round_radius("eo_traps_airstrike_round_radius", "140", FCVAR_REPLICATED | FCVAR_CHEAT);

#define PROJECTILE_MODEL "models/dynamic_traps/shell/shell.mdl"

class COverlordProjectile : public CBaseAnimating, public IScorer
{
public:
	DECLARE_CLASS(COverlordProjectile, CBaseEntity);
	DECLARE_DATADESC();

	COverlordProjectile();
	virtual ~COverlordProjectile();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

	virtual CBasePlayer * GetScorer() {return ( m_Trap ? m_Trap->GetScorer() : NULL ); };
	virtual CBasePlayer * GetAssistant() { return NULL; };

	virtual bool CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
		return true;
	}

	virtual void SetTrapParent(COverlordTrap * pParent) { m_Trap = pParent; };

	virtual void Touch( CBaseEntity *pOther );
private:
	void BeamUpdate();

	CHandle<CBeam> m_Beam;
	CHandle<COverlordTrap> m_Trap;
};

BEGIN_DATADESC(COverlordProjectile)

DEFINE_ENTITYFUNC( Touch ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(overlord_projectile, COverlordProjectile);

COverlordProjectile::COverlordProjectile()
{
	m_Trap = NULL;
	m_Beam = NULL;
}

COverlordProjectile::~COverlordProjectile()
{
	if(m_Beam)
		UTIL_Remove(m_Beam);
}
void COverlordProjectile::Precache()
{
	PrecacheModel(PROJECTILE_MODEL);
	PrecacheModel("sprites/bluelaser1.vmt");
}
void COverlordProjectile::Spawn()
{
	Precache();

	m_takedamage = DAMAGE_YES;

	SetHealth(1);

	SetModel(PROJECTILE_MODEL);

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_DEFAULT );
	UTIL_SetSize( this, -Vector(24,24,24), Vector(24,24,24) );
	SetSolid( SOLID_BBOX );

	SetTouch( &COverlordProjectile::Touch );

	m_Beam = CBeam::BeamCreate("sprites/bluelaser1.vmt", 4.0f);

	BeamUpdate();

	if(m_Beam)
		m_Beam->TurnOff();

	SetThink(&COverlordProjectile::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
	//CreateVPhysics();
}

void COverlordProjectile::Think()
{
	BeamUpdate();

	SetNextThink(gpGlobals->curtime + 0.1f);
}

void COverlordProjectile::BeamUpdate()
{
	if(!m_Beam)
		return;

	m_Beam->SetStartPos(WorldSpaceCenter());
	
	// Calc end position
	Vector velocity = GetAbsVelocity();
	VectorNormalize(velocity);
	Vector end = WorldSpaceCenter() + velocity * MAX_TRACE_LENGTH;
	
	trace_t tr;
	UTIL_TraceLine(WorldSpaceCenter(), end, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	
	m_Beam->SetEndPos(tr.endpos);
	m_Beam->SetWidth(22);
	m_Beam->SetEndWidth(44);
	m_Beam->SetBrightness(255);;
	m_Beam->SetColor( 255, 0, 0 );
	m_Beam->SetNoise(0.1f);
	m_Beam->SetScrollRate(5.0f);
	m_Beam->RelinkBeam();
}

void COverlordProjectile::Touch( CBaseEntity *pOther )
{
	if(pOther && !pOther->IsSolid() )
		return;

	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), m_Trap, eo_traps_airstrike_round_magnitude.GetFloat(), 
		eo_traps_airstrike_round_radius.GetFloat(), true, eo_traps_airstrike_round_magnitude.GetFloat());
	
	UTIL_Remove(this);
}
#endif

#ifdef CLIENT_DLL
#define COverlordAirstrike C_OverlordAirstrike
#endif

ConVar eo_traps_airstrike_radius("eo_traps_airstrike_radius", "525", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_airstrike_dormant_range("eo_traps_airstrike_dormant_range", "550", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_airstrike_rounds("eo_traps_airstrike_rounds", "6", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_airstrike_interval("eo_traps_airstrike_interval", "2.75", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_airstrike_rounds_per_player("eo_traps_airstrike_rounds_per_player", "2", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_airstrike_per_player_delay("eo_traps_airstrike_per_player_delay", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_traps_airstrike_wave_delay("eo_traps_airstrike_wave_delay", "1.05", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_airstrike_health("eo_traps_airstrike_health", "15", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_traps_airstrike_minimum_height("eo_traps_airstrike_minimum_height", "400", FCVAR_REPLICATED | FCVAR_CHEAT);

#define AIRSTRIKE_SOUND "NPC_SScanner.FlyLoop"
#define AIRSTRIKE_RADIUS eo_traps_airstrike_radius.GetFloat()
//#define AIRSTRIKE_ROUND_MODEL "models/props_c17/oildrum001_explosive.mdl"

#ifndef CLIENT_DLL
namespace
{

class sAirstrikePlayer
{
public:
	sAirstrikePlayer() { m_Player = NULL; m_Rounds = 0; m_flNextRestart = 0.0f; };
	sAirstrikePlayer(CBasePlayer * pPlayer) { m_Player = pPlayer; m_Rounds = 0; m_flNextRestart = 0.0f; };

	bool operator==(const sAirstrikePlayer & rhs) const
	{
		return (m_Player == rhs.m_Player);
	}
	CHandle<CBasePlayer> m_Player;
	int m_Rounds;
	float m_flNextRestart;
};

};
#endif

class COverlordAirstrike : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordAirstrike, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordAirstrike() ;
	virtual ~COverlordAirstrike() 
	{ 
		StopSound(AIRSTRIKE_SOUND);
	};

	virtual void Precache();

	virtual error_t CanBuild();

	virtual QAngle  NormalToAngle(const Vector & normal) const;

	virtual Vector GetEmitterPosition()
	{
		Vector rUp;
		VectorRotate(Vector(0, 0, 1), EntityToWorldTransform(), rUp);

		return WorldSpaceCenter() + rUp * 16;
	}

	virtual float GetInitialDormantRange() const { return eo_traps_airstrike_dormant_range.GetFloat(); };
	virtual float GetNormalMultiplier() const { return 0.0f; };
	virtual bool  UseLOSForInitialDormant() const { return false; };
	virtual bool  Use2DForInitialDormant() const { return true; };

#ifdef CLIENT_DLL
	virtual bool   ShouldUseBeam() const { return false; };

	virtual Vector GetRadiusOrigin() { return GetEmitterPosition(); };
	virtual float GetRadiusRange() { return AIRSTRIKE_RADIUS; };

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.8f; };
	virtual float GetPanelZDistance() const { return -12.0f; };
#endif

#ifndef CLIENT_DLL
	virtual int  OnTakeDamage(const CTakeDamageInfo &info)
	{
		if(info.GetAttacker() && info.GetAttacker()->IsPlayer())
			return BaseClass::OnTakeDamage(info);

		return 0;
	}

	virtual void SetTrapDormant(bool bDormant);
	virtual void InitializeTrap();
	virtual void RunTrap();

	virtual void EnableTrap();
	virtual void DisableTrap(float flTime);

#endif
	virtual void DecayTrap() { BaseClass::DecayTrap(); StopSound(AIRSTRIKE_SOUND); };

private:
#ifndef CLIENT_DLL
	void DropRound(CBasePlayer * pPlayer);
	void ScanForPlayers();
#endif
	bool GetSkyboxVector(Vector pos, Vector & end);
#ifndef CLIENT_DLL
	CUtlVector<sAirstrikePlayer> m_Players;

	float m_flNextFall;
	int m_nRound;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordAirstrike, DT_OverlordAirstrike )

BEGIN_NETWORK_TABLE( COverlordAirstrike, DT_OverlordAirstrike )
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordAirstrike)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_airstrike, COverlordAirstrike, Artillery, 80, 60, 
					"Can only be built outdoors; calls periodical artillery strikes", "Defense");

COverlordAirstrike::COverlordAirstrike()
{
	SET_TRAP_MODEL(models/dynamic_traps/unstealther/dynamic_traps_unstealther.mdl);
#ifndef CLIENT_DLL
	m_nRound = 0;
	m_flNextFall = 0.0f;
#endif
}
void COverlordAirstrike::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(AIRSTRIKE_SOUND);
#ifndef CLIENT_DLL
	PrecacheModel(PROJECTILE_MODEL);
#endif
}

error_t COverlordAirstrike::CanBuild()
{
	Vector start = GetEmitterPosition();
	Vector end = start + Vector(0, 0, MAX_TRACE_LENGTH);

	trace_t tr;
	UTIL_TraceLine(start, end, (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_GRATE), this, COLLISION_GROUP_NONE, &tr);

	if(!(tr.surface.flags & SURF_SKY) || tr.startsolid)
	{
		return CreateError("Needs to have the sky directly above it");
	}

	if((tr.endpos - tr.startpos).Length() < eo_traps_airstrike_minimum_height.GetFloat())
	{
		return CreateError("The radar is too high");
	}

	// Make sure two radiuses do not overlap
	COverlordData * pData = GetOverlordData();
	for(int i = 0; i < pData->GetMaxTraps(); i++)
	{
		COverlordTrap * pTrap = pData->GetTrap(i);

		if(!pTrap)
			continue;

		COverlordAirstrike * pAirstrike = dynamic_cast<COverlordAirstrike*>(pTrap);

		if(!pAirstrike)
			continue;

		// The airstrike is in 2D so we ignore any height as to prevent two airstrike being called, one 
		// beneath the other
		Vector pos = GetEmitterPosition();
		pos.z = 0.0f;

		Vector vAir = pAirstrike->GetEmitterPosition();
		vAir.z = 0.0f;

		float distance = (pos - vAir).Length();

		if(distance <= (eo_traps_airstrike_radius.GetFloat() * 2))
		{
			return CreateError("Overlaps with another airstrike");
		}
	}

	if(IsPlayerInRadius(AIRSTRIKE_RADIUS+PLAYER_RADIUS, false, true))
	{
		return CreateError("Player in radius, cannot build.");
	}

	return BaseClass::CanBuild();
}

QAngle COverlordAirstrike::NormalToAngle(const Vector & normal) const
{
	return AlignToTop(normal);
}


#ifndef CLIENT_DLL
void COverlordAirstrike::SetTrapDormant(bool bDormant)
{
	BaseClass::SetTrapDormant(bDormant);

	if(bDormant)
		StopSound(AIRSTRIKE_SOUND);
	else
		EmitSound(AIRSTRIKE_SOUND);
}

void COverlordAirstrike::InitializeTrap()
{
	SetDestroyable(eo_traps_airstrike_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);
	ResetSequence( LookupSequence( "Idle" ) );

	if(!IsInInitialDormant())
		EmitSound(AIRSTRIKE_SOUND);
}

void COverlordAirstrike::RunTrap()
{
	StudioFrameAdvance();


	if(m_flNextFall <= gpGlobals->curtime)
	{
		ScanForPlayers();

		for(int i = 0; i < m_Players.Count(); i++)
		{
			sAirstrikePlayer & record = m_Players.Element(i);

			if(!record.m_Player || !record.m_Player->IsRebel())
			{
				m_Players.Remove(i);
				continue;
			}

			if((record.m_Rounds >= eo_traps_airstrike_rounds_per_player.GetInt()))
			{
				if((record.m_flNextRestart > gpGlobals->curtime))
				{
					continue;
				}
				else
				{
					record.m_Rounds = 0;
					record.m_flNextRestart = 0.0f;
				}
			}

			DropRound(record.m_Player);
			record.m_Rounds++;

			if(record.m_Rounds >= eo_traps_airstrike_rounds_per_player.GetInt())
			{
				record.m_flNextRestart = gpGlobals->curtime + eo_traps_airstrike_per_player_delay.GetFloat();
			}
			
			m_nRound++;
		}

		if(m_nRound >= eo_traps_airstrike_rounds.GetInt())
		{
			m_flNextFall = gpGlobals->curtime + eo_traps_airstrike_interval.GetFloat();
			m_nRound = 0;
			m_Players.Purge();
		}
		else
		{
			m_flNextFall = gpGlobals->curtime + eo_traps_airstrike_wave_delay.GetFloat();
		}
	}

	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordAirstrike::EnableTrap()
{
	BaseClass::EnableTrap();

	if(IsTrapDormant())
		return;

	EmitSound(AIRSTRIKE_SOUND);
}

void COverlordAirstrike::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);

	StopSound(AIRSTRIKE_SOUND);
}

void COverlordAirstrike::ScanForPlayers()
{
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels)
		return;

	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer || pPlayer->IsDead())
			continue;

		sAirstrikePlayer record(pPlayer);

		float dist = (pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length2D();
		if(dist > eo_traps_airstrike_radius.GetFloat())
		{
			if(m_Players.HasElement(record))
			{
				m_Players.Remove(m_Players.Find(record));
			}
			continue;
		}

		if(m_Players.HasElement(record))
			continue;

		m_Players.AddToTail(record);

	}
}

void COverlordAirstrike::DropRound(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return;


	Vector droppos;
	if(!GetSkyboxVector(pPlayer->RealEyePosition() + Vector(0, 0, 32) + pPlayer->GetAbsVelocity(), droppos))
		return;

	COverlordProjectile * pRound = static_cast<COverlordProjectile*>(CreateEntityByName("overlord_projectile"));

	if(pRound)
	{
		pRound->SetAbsOrigin(droppos - Vector(0, 0, 32));
		pRound->SetAbsAngles(QAngle(0, 0, 0));
		pRound->SetTrapParent(this);
		pRound->SetAbsVelocity(Vector(0, 0, -1150));
		pRound->Spawn();
	}
}
#endif

bool COverlordAirstrike::GetSkyboxVector(Vector pos, Vector & vec)
{
	Vector start = pos;
	Vector end = start + Vector(0, 0, 1) * MAX_TRACE_LENGTH;

	trace_t tr;
	UTIL_TraceLine(start, end, (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_GRATE), this, COLLISION_GROUP_NONE, &tr);

	if(tr.startsolid)
		return false;

	vec = tr.endpos;

	return (tr.surface.flags & SURF_SKY) != false;
}

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordInvisibler C_OverlordInvisibler
#endif

ConVar eo_invisibler_distance("eo_invisibler_distance", "195", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_invisibler_own_alpha("eo_invisibler_own_alpha", "105", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_invisibler_alpha("eo_invisibler_alpha", "35", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_invisibler_initial_dormant_range("eo_invisibler_initial_dormant_range", "1536", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_invisibler_mark_points("eo_invisibler_mark_points", "30", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordInvisibler : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordInvisibler, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordInvisibler();
	virtual ~COverlordInvisibler();


	virtual Vector GetRadiusOrigin() { return WorldSpaceCenter(); };
	virtual float GetRadiusRange() { return eo_invisibler_distance.GetFloat(); };
	virtual bool ShouldUseBeam() const { return false; };

	virtual QAngle  NormalToAngle(const Vector & normal) const
	{
		return AlignToTop(normal);
	}

	virtual float GetInitialDormantRange() const { return eo_invisibler_initial_dormant_range.GetFloat(); };
	virtual bool  UseLOSForInitialDormant() const { return false; };
	virtual void InitializeTrap();

#ifndef CLIENT_DLL
	virtual void RunTrap();

	virtual void DecayTrap();
	virtual void SetTrapDormant(bool bDormant);
	virtual void DisableTrap(float flTime);

	bool		 ShouldIgnore(COverlordTrap * pTrap) const { return pTrap == this ||
		!Q_stricmp(pTrap->GetClassname(), "trap_invisibler") ||
		!Q_stricmp(pTrap->GetClassname(), "trap_bomber"); };

	virtual int  GetPoints(PlayerClass_t playerClass) const 
	{ 
		if(playerClass == CLASS_ASSAULT)
			return eo_invisibler_mark_points.GetInt(); 

		return BaseClass::GetPoints(playerClass);
	};
#endif

private:
#ifndef CLIENT_DLL
	void StartCloak();
	void EndCloak();
	void CloakCheck();

	int GetAlpha();

	bool m_bCloakOn;

	CUtlVector<CHandle<COverlordTrap>> m_Cloaked;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordInvisibler, DT_OverlordInvisibler)

BEGIN_NETWORK_TABLE( COverlordInvisibler, DT_OverlordInvisibler )
#ifndef CLIENT_DLL
	
#else
	
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordInvisibler)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_invisibler, COverlordInvisibler, Invisibler, 60, 85, "Cloaks nearby traps.", "Utility");

COverlordInvisibler::COverlordInvisibler()
{
	SET_TRAP_MODEL(models/props_combine/breenlight.mdl);
}

COverlordInvisibler::~COverlordInvisibler()
{
#ifndef CLIENT_DLL
	EndCloak();
#endif
}

void COverlordInvisibler::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetRenderMode(kRenderTransColor);

	SetDestroyable(eo_traps_defaulthealth.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	SetNextRun(gpGlobals->curtime + 0.2f);

	// Never start initially in deactivated state
	if(IsInInitialDormant())
	{
		SetTrapDormant(false);
	}
#endif
}

#ifndef CLIENT_DLL
void COverlordInvisibler::RunTrap()
{
	if(!m_bCloakOn)
	{
		StartCloak();
	}

	CloakCheck();

	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordInvisibler::DecayTrap()
{
	BaseClass::DecayTrap();

	EndCloak();
}

void COverlordInvisibler::SetTrapDormant(bool bDormant)
{
	BaseClass::SetTrapDormant(bDormant);

	if(!bDormant)
		StartCloak();
	else
		EndCloak();
}

void COverlordInvisibler::DisableTrap(float flTime)
{
	BaseClass::DisableTrap(flTime);

	EndCloak();
}

void COverlordInvisibler::StartCloak()
{
	if(m_bCloakOn)
		return;

	SetRenderColorA(eo_invisibler_own_alpha.GetInt());

	m_bCloakOn = true;

	COverlordData & data = *GET_OVERLORD_DATA;

	for(int i = 0; i < data.GetMaxTraps(); i++)
	{
		COverlordTrap * pTrap = data.GetTrap(i);

		if(!pTrap || ShouldIgnore(pTrap))
			continue;

		if((pTrap->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_invisibler_distance.GetFloat())
		{
			CTrapFilter filter(this, COLLISION_GROUP_NONE);

			trace_t tr;
			UTIL_TraceLine(WorldSpaceCenter(), pTrap->WorldSpaceCenter(), MASK_SHOT_HULL | CONTENTS_WATER,
				&filter, &tr);

			if(tr.fraction == 1.0f)
			{
				pTrap->SetRenderMode(kRenderTransColor);
				pTrap->SetRenderColorA(GetAlpha());
				m_Cloaked.AddToTail(pTrap);
			}
		}
	}
}

void COverlordInvisibler::EndCloak()
{
	if(!m_bCloakOn)
		return;

	m_bCloakOn = false;

	for(int i = 0; i < m_Cloaked.Count(); i++)
	{
		COverlordTrap * pTrap = m_Cloaked[i];

		if(pTrap && Q_stricmp(pTrap->GetClassname(), "trap_invisibler"))
		{
			pTrap->SetRenderColorA(255);
		}
		else
		{
			m_Cloaked.Remove(i);
			i--;
		}
	}

	SetRenderColorA(255);

	m_Cloaked.Purge();
}

void COverlordInvisibler::CloakCheck()
{
	for(int i = 0; i < m_Cloaked.Count(); i++)
	{
		COverlordTrap * pTrap = m_Cloaked[i];

		bool bRemove = false;
		if(pTrap && !ShouldIgnore(pTrap))
		{
			if((pTrap->GetAbsOrigin() - GetAbsOrigin()).Length() > eo_invisibler_distance.GetFloat())
			{
				bRemove = true;
				pTrap->SetRenderColorA(255);
			}
			else
			{
				if(pTrap->GetRenderColor().a != GetAlpha())
				{
					pTrap->SetRenderColorA(GetAlpha());
				}
			}
		}
		else
			bRemove = true;

		if(bRemove)
		{
			m_Cloaked.Remove(i);
			i--;
		}
	}

	COverlordData & data = *GET_OVERLORD_DATA;

	for(int i = 0; i < data.GetTrapsAmount(); i++)
	{
		COverlordTrap * pTrap = data.GetTrap(i);

		if(pTrap && !ShouldIgnore(pTrap))
		{
			if((pTrap->GetAbsOrigin() - GetAbsOrigin()).Length() <= eo_invisibler_distance.GetFloat())
			{
				if(!m_Cloaked.HasElement(pTrap))
				{
					m_Cloaked.AddToTail(pTrap);
					pTrap->SetRenderMode(kRenderTransColor);
					pTrap->SetRenderColorA(GetAlpha());
				}
			}
		}
	}
}

int COverlordInvisibler::GetAlpha()
{
	float fraction = 1.0f - (float)((float)GetHealth() / (float)GetMaxHealth());

	int add = 255 - eo_invisibler_alpha.GetInt();

	return eo_invisibler_alpha.GetInt() + fraction * add;
}


#endif
//====================================================================================================================================


#ifdef CLIENT_DLL
#define COverlordMist C_OverlordMist
#endif

ConVar eo_mist_distance("eo_mist_distance", "210.0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_mist_alpha("eo_mist_alpha", "40", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mist_blind("eo_mist_blind", "170", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mist_initial_dormant_range("eo_mist_initial_dormant_range", "840", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mist_particle_delay("eo_mist_particle_delay", "1.05", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mist_close_time("eo_mist_close_time", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_mist_health("eo_mist_health", "160", FCVAR_REPLICATED | FCVAR_CHEAT);

#define MIST_DISTANCE eo_mist_distance.GetFloat()
#define MIST_PARTICLE "MistEffect"
//#define MIST_ALPHA eo_mist_alpha.GetInt()



class COverlordMist : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordMist, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordMist();
	virtual ~COverlordMist();

	virtual void Precache()
	{
		BaseClass::Precache();

		PrecacheParticleSystem(MIST_PARTICLE);
	}

	virtual void InitializeTrap();
	virtual void DecayTrap();
	virtual error_t CanBuild();

	virtual bool  CanBeDormant() const { return true; };
	virtual void  SetTrapDormant(bool bDormant);

	virtual float GetInitialDormantRange() const { return eo_mist_initial_dormant_range.GetFloat(); };

	virtual float GetNormalMultiplier() const { return 0; };
	virtual QAngle NormalToAngle(const Vector & normal) const
	{
		return AlignToTop(normal);
	}
#ifdef CLIENT_DLL
	virtual void TrapThink();

	virtual bool ShouldUseBeam() const { return false; };

	virtual Vector GetRadiusOrigin();
	virtual float GetRadiusRange() { return MIST_DISTANCE; };

	virtual float GetPanelDistanceMultiplier() const { return 0.75f; };
	virtual float GetPanelZDistance() const { return 1.0f; };
	virtual float GetPanelYDistance() const { return 0.0f; };
	virtual QAngle GetPanelAngle() const { QAngle angle = BaseClass::GetPanelAngle(); angle[PITCH] += 45; return angle; };
#else
	virtual void RunTrap();
#endif
private:
	CNetworkVar(float, m_flParticleTime);
#ifndef CLIENT_DLL
	void Blind(CBasePlayer * pPlayer);

	float m_flCloseTime;
#else
	CNewParticleEffect * m_hEffect;
#endif
	
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordMist, DT_OverlordMist)

BEGIN_NETWORK_TABLE( COverlordMist, DT_OverlordMist )
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flParticleTime), 0, SPROP_NOSCALE ),
#else
	RecvPropFloat(RECVINFO(m_flParticleTime)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordMist)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_mist, COverlordMist, Mist, 30, 60, "Emits a heavy mist occluding area around it as well as blinds anybody who gets close to it.",
					"Utility");

COverlordMist::COverlordMist()
{
	SET_TRAP_MODEL(models/dynamic_traps/mist/mist.mdl);
#ifndef CLIENT_DLL
	m_flCloseTime = 0.0f;
	m_flParticleTime = 0.0f;
#else
	m_hEffect = NULL;
#endif
}

COverlordMist::~COverlordMist()
{
#ifdef CLIENT_DLL
	if(m_hEffect)
	{
		ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}
#endif
}

void COverlordMist::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_mist_health.GetInt());

	m_flParticleTime = gpGlobals->curtime + eo_mist_particle_delay.GetFloat();
	m_flCloseTime = gpGlobals->curtime + GetTrapLifetime() - eo_mist_close_time.GetFloat();
	ResetSequence( LookupSequence( "open" ) );
#else
	SetNextClientThink(gpGlobals->curtime + 0.1f);
#endif
}

void COverlordMist::DecayTrap()
{
	BaseClass::DecayTrap();
#ifdef CLIENT_DLL
	if(m_hEffect)
	{
		ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}
#endif
}

error_t COverlordMist::CanBuild()
{
	/*if(IsPlayerInRadius(MIST_DISTANCE + PLAYER_RADIUS))
	{
		return CreateError("Player in radius, cannot build");
	}*/

	Vector up(0, 0, 1);
	Vector rUp;
	VectorRotate(up, EntityToWorldTransform(), rUp);

	Vector emitPos = WorldSpaceCenter() + rUp * 8;

	if(UTIL_PointContents(emitPos) & (CONTENTS_WATER|CONTENTS_SLIME))
	{
		return CreateError("Cannot build underwater");
	}

	Vector forw, end;
	AngleVectors(GetAbsAngles(), &forw);
	VectorMA(emitPos, MAX_TRACE_LENGTH, forw, end);

	trace_t trace;
	UTIL_TraceLine(emitPos, end, MASK_SHOT, this, COLLISION_GROUP_NONE, &trace);

	if(trace.allsolid || trace.startsolid || trace.fraction >= 1.0f)
	{
		return CreateError("The Mist would have no effect!");
	}

	return BaseClass::CanBuild();
}

void COverlordMist::SetTrapDormant(bool bDormant)
{
	if(bDormant)
	{

		if(m_flParticleTime <= gpGlobals->curtime)
		{
#ifdef CLIENT_DLL
			if(m_hEffect)
			{
				ParticleProp()->StopEmission(m_hEffect);
				m_hEffect = NULL;
			}
#endif
		}
		else
		{
#ifdef CLIENT_DLL
			SetNextClientThink(CLIENT_THINK_NEVER);
#endif
		}
		
	}
	else
	{
		if(m_flParticleTime <= GetDormantTime())
		{
#ifdef CLIENT_DLL
			if(!m_hEffect)
				m_hEffect = ParticleProp()->Create(MIST_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);
#endif
		}
		else
		{
			float remaining = m_flParticleTime.Get() - GetDormantTime();

			m_flParticleTime = gpGlobals->curtime + remaining;
#ifdef CLIENT_DLL
			SetNextClientThink(gpGlobals->curtime + 0.1f);
#endif
		}
#ifndef CLIENT_DLL
		m_flCloseTime = gpGlobals->curtime + GetTrapLifetime() - 1.5f;
#endif
	}

	BaseClass::SetTrapDormant(bDormant);
}

#ifdef CLIENT_DLL
void COverlordMist::TrapThink()
{
	if(!IsTrapDormant())
	{
		// Emit particles
		if((m_flParticleTime <= gpGlobals->curtime) && m_flParticleTime > 0.0f)
		{
			if(!m_hEffect)
				m_hEffect = ParticleProp()->Create(MIST_PARTICLE, PATTACH_ABSORIGIN_FOLLOW);
		
			SetNextClientThink(CLIENT_THINK_NEVER);
			return;
		}
	}

	SetNextClientThink(gpGlobals->curtime + 0.1f);
}

Vector COverlordMist::GetRadiusOrigin()
{
	Vector wUp(0, 0, 1);
	Vector rUp;

	VectorRotate(wUp, EntityToWorldTransform(), rUp);
	Vector temp = (WorldSpaceCenter() + rUp * 8);

	return temp;
}
#else
void COverlordMist::RunTrap()
{
	// Start closing
	if((m_flCloseTime <= gpGlobals->curtime) && (GetSequence() != LookupSequence("close")))
		ResetSequence( LookupSequence( "close" ) );

	// Move into attackidle
	if((GetSequence() == LookupSequence("open")) && (GetCycle() == 1.0f))
		ResetSequence( LookupSequence( "attackIdle" ) );

	StudioFrameAdvance();

	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(m_flParticleTime <= gpGlobals->curtime)
	{
		if(pRebels)
		{
			for(int i = 0; i < pRebels->GetNumPlayers(); i++)
			{
				CBasePlayer * pRebel = pRebels->GetPlayer(i);

				if(!pRebel)
					continue;

				if((pRebel->RealEyePosition() - GetEmitterPosition()).Length() <= MIST_DISTANCE)
				{
					trace_t tr;
					UTIL_TraceLine(GetEmitterPosition(), pRebel->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

					// Found a rebel
					if(tr.m_pEnt == pRebel)
					{
						Blind(pRebel);
					}
				}

			}
		}
	}


	SetNextRun(gpGlobals->curtime + 0.1f);
}

void COverlordMist::Blind(CBasePlayer * pPlayer)
{
	color32 black = { 0, 0, 0, eo_mist_blind.GetInt() };
	UTIL_ScreenFade(pPlayer, black, 2.0f, 3.5f, FFADE_IN);

	IGameEvent * event = gameeventmanager->CreateEvent( "eo_suppresshints" );

	if(event)
	{
		event->SetInt("userid", pPlayer->GetUserID());
		event->SetFloat("delay", 5.5f);

		gameeventmanager->FireEvent(event);
	}
}
#endif

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordDevourer C_OverlordDevourer
#endif

ConVar eo_devourer_range("eo_devourer_range", "640.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_range_grab("eo_devourer_range_grab", "42", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_damage("eo_devourer_damage", "20", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar eo_devourer_speed("eo_devourer_speed", "1000", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_health("eo_devourer_health", "70", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_dormant_range("eo_devourer_dormant_range", "700", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_retract_delay("eo_devourer_retract_delay", "0.75", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_devourer_release_points("eo_devourer_release_points", "30", FCVAR_REPLICATED | FCVAR_CHEAT);

#define DEVOURER_RANGE eo_devourer_range.GetFloat()
#define DEVOURER_RANGE_GRAB eo_devourer_range_grab.GetFloat()

#define DEVOURER_DAMAGE_PER_SECOND eo_devourer_damage.GetFloat()

#define DEVOURER_SOUND_ATTACK "traps.devourer_attack_sound"
#define DEVOURER_SOUND_ATTACK_IN "traps.devourer_attack_sound_in"
#define DEVOURER_SOUND_ATTACK_OUT "traps.devourer_attack_sound_out"
#define DEVOURER_SOUND_SNATCH "traps.devourer_snatch"

#define DEVOURER_VECTOR Vector(6.0f, 6.0f, 6.0f)
#define DEVOURER_VECTOR_SWEEP DEVOURER_VECTOR

ConVar eo_devourer_sound("eo_devourer_sound", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordDevourer : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordDevourer, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordDevourer();
	virtual ~COverlordDevourer();

	virtual void Precache();

	virtual void InitializeTrap();

	virtual error_t CanBuild();

	virtual Vector  GetEmitterPosition();
	virtual float   GetNormalMultiplier() const { return 0.65f; };

	CBasePlayer * GetTarget() const { return m_Target.Get(); };

	virtual float GetInitialDormantRange() const { return eo_devourer_dormant_range.GetFloat(); };
#ifdef CLIENT_DLL
	virtual void  GhostThink();

	virtual float BeamRange() const { return DEVOURER_RANGE; };
	virtual const char * GetHint()
	{
		C_BasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();
		if(!pLocal || !pLocal->IsRebel() || m_Target.Get() == pLocal || !m_Target.Get())
			return NULL;

		return "Destroy this trap to free the player!";
	}
	virtual void FireGameEvent(IGameEvent * event);

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.75f; };
	virtual float GetPanelZDistance() const { return 4.0f; };
#else
	virtual void Event_Killed(const CTakeDamageInfo &info);

	virtual bool ShouldDecay() const { return (BaseClass::ShouldDecay() && (m_iState == 0 || m_iState == 4)); }; 
	virtual void RunTrap();
	virtual void DecayTrap();

	virtual bool CanDisableTrap() const { return m_iState == 0; };
#endif
private:
#ifndef CLIENT_DLL
	void  CreateRagdoll(CHL2MP_Player * pPlayer);
	float GetDistanceToPlayer(CBasePlayer * pPlayer);
	Vector GetGrabPoint(CBasePlayer * pPlayer);

	bool ShouldGrab(CBasePlayer * pPlayer);
	void Freeze(CBasePlayer * pPlayer);
	void UnFreeze(CBasePlayer * pPlayer);
	
	void Sweep();
	void MoveToCatch();
	void MoveBack();
	void StrangleVictim();
	void Revert();

	void ReleasePlayer();

	bool TestPlayerPosition(Vector pos) const;

	Vector m_vStart;
	QAngle m_aStart;

	Vector m_GrabPosition;

	bool m_bFrozen;
	float m_flAccumulatedHealth;
	float m_flSoundTime;
	float m_flStartRetracting;

	int m_nRagdollModel;
#else
	CNewParticleEffect * m_hEffect;
#endif
	// 0 = waiting, 1 = moving for the victim, 2 = moving back with the victim, 3 = strangling, 4 = reverting
	CNetworkVar(int, m_iState);
	CNetworkVar(CHandle<CBasePlayer>, m_Target);
};


IMPLEMENT_NETWORKCLASS_ALIASED( OverlordDevourer, DT_OverlordDevourer )

BEGIN_NETWORK_TABLE( COverlordDevourer, DT_OverlordDevourer )
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_iState)),
	SendPropEHandle(SENDINFO(m_Target)),
#else
	RecvPropEHandle(RECVINFO(m_Target)),
	RecvPropInt(RECVINFO(m_iState)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordDevourer)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_devourer, COverlordDevourer, Devourer, 65, 80, "The Devourer grabs anyone in its range then pulls them back to slowly consume their flesh. Drains health.",
					"Offense");

COverlordDevourer::COverlordDevourer()
{
	//SET_TRAP_MODEL(models/props_interiors/refrigerator01a.mdl);
	SET_TRAP_MODEL(models/dynamic_traps/devourer/devourer.mdl);
#ifndef CLIENT_DLL
	m_iState = 0;
	m_Target = NULL;
	m_bFrozen = false;
	m_flAccumulatedHealth = 0.0f;
	m_flStartRetracting = 0.0f;
	m_nRagdollModel = 0;
#else
	m_hEffect = NULL;
#endif
}

COverlordDevourer::~COverlordDevourer()
{
#ifndef CLIENT_DLL
	ReleasePlayer();

	StopSound(DEVOURER_SOUND_ATTACK);
#endif
}

error_t COverlordDevourer::CanBuild()
{
	Vector end, forw;
	AngleVectors(GetAbsAngles(), &forw);

	VectorMA(GetEmitterPosition(), DEVOURER_RANGE, forw, end);
	trace_t tr;
	UTIL_TraceHull(GetEmitterPosition(), end, -DEVOURER_VECTOR, DEVOURER_VECTOR, MASK_SHOT_HULL, 
		this, COLLISION_GROUP_NONE, &tr);

	if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
	{
		return CreateError("Player in range!");
	}
	else if(tr.allsolid || tr.startsolid)
	{
		return CreateError("The Devourer will have no effect!");
	}

	return BaseClass::CanBuild();
}

Vector COverlordDevourer::GetEmitterPosition()
{
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	Vector emitter;
	if(GetAttachment( LookupAttachment("dynamic_trap"), emitter))
		return emitter + forw * 24;

	return WorldSpaceCenter() + forw * 26.0f;
}

void COverlordDevourer::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(DEVOURER_SOUND_ATTACK);
	PrecacheScriptSound(DEVOURER_SOUND_ATTACK_IN);
	PrecacheScriptSound(DEVOURER_SOUND_ATTACK_OUT);
	PrecacheScriptSound(DEVOURER_SOUND_SNATCH);

#ifndef CLIENT_DLL
	m_nRagdollModel = PrecacheModel("models/Gibs/Fast_Zombie_Legs.mdl");
#else
	PrecacheParticleSystem("BloodStream");
#endif
}

void COverlordDevourer::InitializeTrap()
{
#ifndef CLIENT_DLL
	ResetSequence( LookupSequence( "Idle" ) );
	SetDestroyable(eo_devourer_health.GetInt());
	m_vStart = GetAbsOrigin();
	m_aStart = GetAbsAngles();
#else
	ListenForGameEvent("devourer_strangle");
	ListenForGameEvent("devourer_release");
#endif
}


#ifdef CLIENT_DLL
void COverlordDevourer::GhostThink()
{
	Vector left, right;
	left = DEVOURER_VECTOR;
	right.x = left.x = 16;
	right.y = -left.y;
	right.z = left.z = 0;

	Vector wLeft, wRight;
	EntityToWorldSpace(left, &wLeft);
	EntityToWorldSpace(right, &wRight);

	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);
	
	// Left beam
	trace_t tr;
	UTIL_TraceLine(wLeft, wLeft + forw * FAN_RANGE, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	DrawBeam(wLeft, tr.endpos, Color(255, 0, 0, 255));

	// Right beam
	UTIL_TraceLine(wRight, wRight + forw * MAX_TRACE_LENGTH, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	DrawBeam(wRight, tr.endpos, Color(255, 0, 0, 255));
}

#else
void COverlordDevourer::Event_Killed( const CTakeDamageInfo &info )
{
	if(m_iState >= 2)
	{
		CBasePlayer * pAttacker = ToBasePlayer(info.GetAttacker());
		if(pAttacker)
			pAttacker->IncreaseOverlordDamage(eo_devourer_release_points.GetInt());
	}
	ReleasePlayer();

	BaseClass::Event_Killed(info);
}

void COverlordDevourer::DecayTrap()
{
	BaseClass::DecayTrap();

	ReleasePlayer();

	StopSound(DEVOURER_SOUND_ATTACK);
}

void COverlordDevourer::RunTrap()
{
	// Initial checks
	if(m_iState != 0 && m_iState != 4)
	{
		if(!m_Target.Get() || !m_Target.Get()->IsAlive() || !m_Target.Get()->IsRebel() || ((GetAbsOrigin() - m_Target.Get()->WorldSpaceCenter()).Length() > DEVOURER_RANGE))
		{
			ReleasePlayer();

			if(m_iState == 3)
				EmitSound(DEVOURER_SOUND_ATTACK_OUT);

			m_iState = 4;
			m_Target = NULL;
			StopSound(DEVOURER_SOUND_ATTACK);
		}
		else if(m_Target.Get() && (m_iState == 3 || m_iState == 2))
		{
			m_Target.Get()->SetAbsVelocity(Vector(0, 0, 0));
		}
	}

	float flNextUpdate = 0.03f;
	if(m_iState == 0)
		Sweep();
	else if(m_iState == 1)
	{
		MoveToCatch();
		//flNextUpdate = 0.05f;
	}
	else if(m_iState == 2)
	{
		MoveBack();
		//flNextUpdate = 0.05f;
	}
	else if(m_iState == 3)
	{
		StrangleVictim();
		//flNextUpdate = 0.05f;
	}
	else if(m_iState == 4)
		Revert();
	else
		Warning("Unhandled devourer state!\n");

	SetNextRun(gpGlobals->curtime + flNextUpdate);
}

float COverlordDevourer::GetDistanceToPlayer(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return FLT_MAX;

	Vector dir = GetGrabPoint(pPlayer) - GetEmitterPosition();
	return dir.Length();
}

Vector COverlordDevourer::GetGrabPoint(CBasePlayer * pPlayer)
{	
	if(!pPlayer)
		return vec3_origin;

	Vector delta = GetAbsOrigin() - pPlayer->WorldSpaceCenter();
	VectorNormalize(delta);
	delta *= 52;

	return (pPlayer->WorldSpaceCenter() + delta);
}

bool COverlordDevourer::ShouldGrab(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return false;

	if(GetDistanceToPlayer(pPlayer) <= (DEVOURER_RANGE_GRAB))
		return true;

	return false;
}

void COverlordDevourer::Freeze(CBasePlayer * pPlayer)
{
	if(m_bFrozen)
		return;

	if(!pPlayer)
		return;

	m_bFrozen = true;
	pPlayer->AddFrozenRef();
	pPlayer->SetAbsVelocity(Vector(0, 0, 0));
}

void COverlordDevourer::UnFreeze(CBasePlayer * pPlayer)
{
	if(!m_bFrozen)
		return;

	if(!pPlayer)
		return;

	m_bFrozen = false;
	pPlayer->RemoveFrozenRef();
}

void COverlordDevourer::Sweep()
{
	Vector end, forw;
	AngleVectors(GetAbsAngles(), &forw);

	VectorMA(GetEmitterPosition(), DEVOURER_RANGE, forw, end);
	trace_t tr;
	UTIL_TraceHull(GetEmitterPosition(), end, -DEVOURER_VECTOR_SWEEP, DEVOURER_VECTOR_SWEEP, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	//UTIL_TraceEntity(this, GetEmitterPosition() + (forw * 24), end, MASK_SHOT_HULL, &tr); 

	if(!tr.startsolid && tr.m_pEnt && tr.m_pEnt->IsPlayer() && tr.m_pEnt->IsAlive() && tr.m_pEnt->GetTeamNumber() == TEAM_REBELS)
	{
		CBasePlayer * pPlayer = ToBasePlayer(tr.m_pEnt);
		if(!pPlayer->IsInvisible())
		{
			// Check whether he is some other devourer's target
			for(int i = 0; i < GetOverlordData()->GetMaxTraps(); i++)
			{
				COverlordTrap * pTrap = GetOverlordData()->GetTrap(i);

				if(!pTrap)
					continue;

				if(!Q_stricmp(pTrap->GetClassname(), "trap_devourer"))
				{
					COverlordDevourer * pDev = static_cast<COverlordDevourer*>(pTrap);

					if(pDev->GetTarget() == pPlayer)
						return;
				}
			}
			
			
			m_Target = pPlayer;
			m_iState = 1;

			ResetSequence(LookupSequence("snatch"));
		}
	}
}

void COverlordDevourer::MoveToCatch()
{
	StudioFrameAdvance();

	Vector dir;
	AngleVectors(GetAbsAngles(), &dir);

	// Check whether player is in range
	Vector end;
	VectorMA(GetEmitterPosition(), DEVOURER_RANGE - (GetAbsOrigin() - m_vStart).Length(), dir, end);
	trace_t tr;
	UTIL_TraceHull(GetEmitterPosition(), end, -DEVOURER_VECTOR, DEVOURER_VECTOR, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	// Check whether we should revert
	if(tr.m_pEnt != m_Target.Get())
	{
		if(ShouldGrab(m_Target.Get()))
			UnFreeze(m_Target.Get());

		if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
		{
			m_Target = ToBasePlayer(tr.m_pEnt);
		}
		else
		{
			m_Target = NULL;
		}

		// Run next move to catch a new target
		if(m_Target.Get())
			MoveToCatch();

		return;
	}

	// Now move forward
	//float distToPlayer = GetDistanceToPlayer(m_Target.Get());


	// Grab!
	if(ShouldGrab(m_Target.Get()))
	{
		IGameEvent * event = gameeventmanager->CreateEvent("devourer_grab");
		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetInt("userid", m_Target.Get()->GetUserID());

			gameeventmanager->FireEvent(event);
		}

		QAngle ang;
		VectorAngles(-dir, ang);

		m_Target.Get()->SetLocalAngles(ang);

		Freeze(m_Target.Get());
		m_GrabPosition = m_Target.Get()->GetAbsOrigin();
		//m_Target.Get()->SetAbsAngles(GetAbsAngles());
		m_Target.Get()->SetMoveType( MOVETYPE_NOCLIP );
		m_iState++;

		// Correction in place
		float cycle = (1.0f - GetCycle()) - 0.08f;

		ResetSequence(LookupSequence("drag"));
		SetCycle(cycle);

		EmitSound(DEVOURER_SOUND_ATTACK_IN);
		EmitSound(DEVOURER_SOUND_SNATCH);
		m_flSoundTime = gpGlobals->curtime + enginesound->GetSoundDuration(DEVOURER_SOUND_ATTACK_IN);
		m_flStartRetracting = gpGlobals->curtime + eo_devourer_retract_delay.GetFloat();
	}


}

void COverlordDevourer::MoveBack()
{
	if(m_flSoundTime != 0.0f && m_flSoundTime <= gpGlobals->curtime)
	{
			EmitSound(DEVOURER_SOUND_ATTACK);
			m_flSoundTime = 0.0f;
	}

	if(m_flStartRetracting <= gpGlobals->curtime)
		StudioFrameAdvance();
	
	m_Target.Get()->SetAbsOrigin(GetEmitterPosition() - Vector(0, 0, 48));

	// We're back (with a victim)!
	if(GetCycle() == 1.0f)
	{
		IGameEvent * event = gameeventmanager->CreateEvent("devourer_strangle");
		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetInt("userid", m_Target.Get()->GetUserID());

			gameeventmanager->FireEvent(event);
		}

		m_Target.Get()->SetRenderMode( kRenderNormal );
		m_Target.Get()->SetRenderColorA(0);
		m_iState++;
		StopSound(DEVOURER_SOUND_ATTACK_IN);
	}
}

void COverlordDevourer::StrangleVictim()
{
	if(m_flSoundTime != 0.0f && m_flSoundTime <= gpGlobals->curtime)
	{
			EmitSound(DEVOURER_SOUND_ATTACK);
			m_flSoundTime = 0.0f; 
	}

	// Damage now
	const float damage = (gpGlobals->curtime - GetLastRun()) * (float)DEVOURER_DAMAGE_PER_SECOND;

	m_flAccumulatedHealth += damage; 

	if(m_flAccumulatedHealth >= 1.0f)
	{

		int health = int(m_flAccumulatedHealth);

		m_flAccumulatedHealth = m_flAccumulatedHealth - health;

		int newHealth = GetHealth() + health;
		
		m_Target.Get()->OnTakeDamage(CTakeDamageInfo( this, this, health, DMG_REMOVENORAGDOLL ));

		if(newHealth > GetMaxHealth())
			SetMaxHealth(newHealth);

		SetHealth(newHealth);

		// Check for a dead player and spawn legs
		if(!m_Target.Get()->IsAlive())
		{
			CHL2MP_Player * pPlayer = static_cast<CHL2MP_Player*>(m_Target.Get().Get());
			CreateRagdoll(pPlayer);
		}
	}

	// Overlays!
	for(int i = 1; i <= MAX_PLAYERS; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || pPlayer == GetTarget() || !(pPlayer->IsRebel() || pPlayer->IsOverlord()) || !pPlayer->IsAlive())
			continue;

		if(pPlayer->FInViewCone(WorldSpaceCenter()) && (WorldSpaceCenter() - pPlayer->RealEyePosition()).Length() <= 128)
		{
			trace_t tr;
			UTIL_TraceLine(pPlayer->RealEyePosition(), WorldSpaceCenter(), MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt == this || tr.m_pEnt == GetTarget() || tr.fraction == 1.0f)
			{
				CSingleUserRecipientFilter user(pPlayer);
				user.MakeReliable();
				UserMessageBegin( user, "ScreenOverlay" );
					WRITE_SHORT( 1 );
				MessageEnd();
			}
		}
	}
}



void COverlordDevourer::Revert()
{
	ResetSequence(LookupSequence("Idle"));
	StopSound(DEVOURER_SOUND_ATTACK_IN);
	StopSound(DEVOURER_SOUND_ATTACK);

	SetAbsOrigin(m_vStart);
	SetAbsAngles(m_aStart);

	m_flStartRetracting = 0.0f;
	m_Target = NULL;
	m_iState = 0;
	m_flSoundTime = 0.0f;
}

void COverlordDevourer::ReleasePlayer()
{
	IGameEvent * event = gameeventmanager->CreateEvent("devourer_release");
	if(event)
	{
		event->SetInt("entindex", entindex());
		event->SetInt("userid", (m_Target.Get() ? m_Target.Get()->GetUserID() : -1));

		gameeventmanager->FireEvent(event);
	}

	if(!m_Target.Get())
		return;

	UnFreeze(m_Target.Get());
	m_Target.Get()->SetMoveType( MOVETYPE_WALK );
	m_Target.Get()->SetRenderMode( kRenderNormal );
	m_Target.Get()->SetRenderColorA(255);


	// Move him a bit to stop the player from being stuck

	if(m_Target.Get()->IsAlive() && (m_iState == 2 || m_iState == 3))
	{
		Vector dir = m_Target.Get()->GetAbsOrigin() - m_vStart;
		VectorNormalize(dir);

		CBasePlayer * pPlayer = m_Target.Get();

		// Make it non-solid for this check!
		SetSolid(SOLID_NONE);

		Vector releasePos = pPlayer->GetAbsOrigin();

		
		Vector forw;
		AngleVectors(GetAbsAngles(), &forw);
		// Check maximum distance...
		trace_t tr;
		UTIL_TraceLine(GetEmitterPosition(), GetEmitterPosition() + forw * DEVOURER_RANGE, MASK_SOLID, pPlayer, 
			COLLISION_GROUP_NONE, &tr);

		int distance = (tr.endpos - tr.startpos).Length();
		bool bSafe = false;
		for(int i = 0; i <= distance; i++)
		{
			releasePos += forw;
			bSafe = TestPlayerPosition(releasePos);

			if(bSafe)
			{
				// Safety checks for displacements and such
				UTIL_TraceLine(releasePos, releasePos + Vector(0, 0, 28), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);

				if((tr.fraction != 1.0f) || tr.startsolid)
				{
					DevMsg("Traceline test failed\n");
					bSafe = false;
					continue;
				}
				
				UTIL_TraceLine(releasePos + Vector(0, 0, 28), releasePos, MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);

				if((tr.fraction != 1.0f) || tr.startsolid)
				{
					DevMsg("Traceline test failed\n");
					bSafe = false;
					continue;
				}
				
				DevMsg("Releasing at %i\n", i);
				break;
			}
		}

		if(!bSafe)
		{
			DevMsg("No safe, releasing at grab position\n");
			releasePos = m_GrabPosition;
		}

		pPlayer->SetAbsOrigin(releasePos);

		m_GrabPosition = vec3_origin;

		SetSolid(SOLID_VPHYSICS);

		//m_Target.Get()->SetAbsOrigin(m_Target.Get()->GetAbsOrigin() + dir);
	}
}

bool COverlordDevourer::TestPlayerPosition(Vector pos) const
{
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	trace_t tr;
	UTIL_TraceEntity(m_Target.Get(), pos, pos + forw, MASK_PLAYERSOLID, &tr);

	return (tr.fraction == 1.0f && !tr.startsolid);
}

void COverlordDevourer::CreateRagdoll(CHL2MP_Player * pPlayer)
{
	if(!pPlayer)
		return;

	if ( pPlayer->m_hRagdoll )
	{
		UTIL_RemoveImmediate( pPlayer->m_hRagdoll );
		pPlayer->m_hRagdoll = NULL;
	}

	CHL2MPRagdoll * pRagdoll = pRagdoll = static_cast<CHL2MPRagdoll*>(CreateEntityByName("hl2mp_ragdoll"));

	if(pRagdoll)
	{
		pRagdoll->m_hPlayer = pPlayer;
		pRagdoll->m_vecRagdollOrigin = pPlayer->GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = pPlayer->GetAbsVelocity();
		pRagdoll->m_nForceBone = pPlayer->m_nForceBone;
		pRagdoll->m_vecForce = vec3_origin;
		pRagdoll->SetAbsOrigin( pPlayer->GetAbsOrigin() );
		pRagdoll->m_nModelIndex = m_nRagdollModel;		

		pPlayer->m_hRagdoll.Set(pRagdoll);
	}
}
#endif

#ifdef CLIENT_DLL
void COverlordDevourer::FireGameEvent(IGameEvent * event)
{
	BaseClass::FireGameEvent(event);

	if(event)
	{
		if(!Q_stricmp(event->GetName(), "devourer_strangle"))
		{
			if(event->GetInt("entindex") != entindex())
				return;

			if(m_hEffect)
			{
				ParticleProp()->StopEmission(m_hEffect);
				m_hEffect = NULL;
			}

			m_hEffect = ParticleProp()->Create("BloodStream", PATTACH_POINT_FOLLOW, "dynamic_trap"); 
		}
		else if(!Q_stricmp(event->GetName(), "devourer_release"))
		{
			if(event->GetInt("entindex") != entindex())
				return;

			if(m_hEffect)
			{
				ParticleProp()->StopEmission(m_hEffect);
				m_hEffect = NULL;
			}
		}
	}
}
#endif

//=========================================================================================================================

#ifdef CLIENT_DLL
#define COverlordZapper C_OverlordZapper
#endif

ConVar eo_zapper_health("eo_zapper_health", "100", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_damage("eo_zapper_damage", "13.0", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_range("eo_zapper_range", "260", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_interval("eo_zapper_interval", "1.05", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_fulldamage_range("eo_zapper_fulldamage_range", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_loop_volume("eo_zapper_loop_volume", "0.32", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_initial_dormant_range("eo_zapper_initial_dormant_range", "380", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_beams_per_player("eo_zapper_beams_per_player", "8", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_view_kick_min("eo_zapper_view_kick_min", "-28", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_view_kick_max("eo_zapper_view_kick_max", "28", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_slowdown("eo_zapper_slowdown", "200", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_zapper_slowdown_length("eo_zapper_slowdown_length", "0.20", FCVAR_CHEAT | FCVAR_REPLICATED);

#define ZAPPER_RANGE eo_zapper_range.GetFloat()
#define ZAPPER_DAMAGE eo_zapper_damage.GetFloat()
#define ZAPPER_INTERVAL eo_zapper_interval.GetFloat()
#define ZAPPER_FULLDAMAGE eo_zapper_fulldamage_range.GetFloat()
#define ZAPPER_SOUND "ambient.electrical_zap_1"
#define ZAPPER_SOUND_LOOP "Town.d1_town_01_electric_loop"
#define ZAPPER_SPRITE "sprites/physbeam.vmt"

class COverlordZapper : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordZapper, COverlordTrap);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	COverlordZapper();
	virtual ~COverlordZapper();

	virtual void Precache();

	virtual void InitializeTrap();
	virtual error_t CanBuild();

	virtual void SetTrapDormant(bool bDormant)
	{
		BaseClass::SetTrapDormant(bDormant);

		if(bDormant)
			StopSound(ZAPPER_SOUND_LOOP);
		else
			EmitLoopSound();
	}

	virtual float GetInitialDormantRange() const { return eo_zapper_initial_dormant_range.GetFloat(); };

	virtual float GetNormalMultiplier() const { return 0; };
	virtual QAngle NormalToAngle(const Vector &normal) const
	{
		return AlignToTop(normal);
	}

	virtual Vector GetEmitterPosition() { return WorldSpaceCenter(); }
#ifndef CLIENT_DLL
	virtual void RunTrap();
#else
	virtual float GetPanelDistanceMultiplier() const { return 0.8f; };
	virtual float GetPanelZDistance() const { return -12.0f; };
	virtual float GetRadiusRange() { return ZAPPER_RANGE; };

	virtual bool ShouldUseBeam() const { return false; };

	virtual void FireGameEvent(IGameEvent * event);
#endif
private:
	void EmitLoopSound();
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordZapper, DT_OverlordZapper )

BEGIN_NETWORK_TABLE( COverlordZapper, DT_OverlordZapper )

END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordZapper)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_zapper, COverlordZapper, Tesla, 50, 60, "Zaps any players near it with energy beams. Deals additional damage to rebels close to it.",
					"Defense");

COverlordZapper::COverlordZapper()
{
	SET_TRAP_MODEL(models/dynamic_traps/tesla/tesla.mdl);
}

COverlordZapper::~COverlordZapper()
{
	StopSound(ZAPPER_SOUND);
	StopSound(ZAPPER_SOUND_LOOP);
}

void COverlordZapper::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(ZAPPER_SOUND);
	PrecacheScriptSound(ZAPPER_SOUND_LOOP);
	PrecacheModel(ZAPPER_SPRITE);
}

error_t COverlordZapper::CanBuild()
{
	if(IsPlayerInRadius(ZAPPER_RANGE + PLAYER_RADIUS))
	{
		return CreateError("Player in radius, cannot build");
	}

	return BaseClass::CanBuild();
}


void COverlordZapper::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_zapper_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);
	SetNextRun(gpGlobals->curtime + ZAPPER_INTERVAL);
#else
	ListenForGameEvent("zapper_discharge");
#endif
	EmitLoopSound();
}

void COverlordZapper::EmitLoopSound()
{
	// Emit the electricity sound
	EmitSound_t params;
	params.m_flVolume = eo_zapper_loop_volume.GetFloat();
	//params.m_SoundLevel = SNDLVL_40dB;
	params.m_pSoundName = ZAPPER_SOUND_LOOP;
	params.m_nFlags = SND_CHANGE_VOL;
	CPASFilter filter(WorldSpaceCenter());
	EmitSound(filter, entindex(), params);
}

#ifndef CLIENT_DLL
void COverlordZapper::RunTrap()
{
	CBaseEntity * pList[1024];
	int count = UTIL_EntitiesInSphere(pList, ARRAYSIZE(pList), WorldSpaceCenter(), ZAPPER_RANGE, MASK_ALL);

	bool bPlaySound = false;
	int beamCount = 0;
	for(int i = 0; i < count; i++)
	{
		CBaseEntity * pEnt = pList[i];

		if(!pEnt || !pEnt->IsPlayer())
			continue;

		CBasePlayer * pVictim = static_cast<CBasePlayer*>(pEnt);

		if(!pVictim->IsRebel() || !pVictim->IsAlive())
			continue;

		trace_t tr;

		CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
		UTIL_TraceLine(GetEmitterPosition(), pVictim->WorldSpaceCenter(), MASK_SHOT, &filter, &tr);

		if(tr.fraction == 1.0f)
		{
			// Calculate damage
			float damage = ZAPPER_DAMAGE;

			float dist = (GetEmitterPosition() - pVictim->WorldSpaceCenter()).Length();

			/*if(dist < (ZAPPER_RANGE/2))
			{
				float factor = 1 - (dist/(ZAPPER_RANGE/2));

				damage += ZAPPER_DAMAGE * factor;
			}*/

			float x = random->RandomInt(eo_zapper_view_kick_min.GetFloat(), eo_zapper_view_kick_max.GetFloat());
			float y = random->RandomInt(eo_zapper_view_kick_min.GetFloat(), eo_zapper_view_kick_max.GetFloat());
			float z = random->RandomInt(eo_zapper_view_kick_min.GetFloat(), eo_zapper_view_kick_max.GetFloat());

			
			if((dist > (ZAPPER_RANGE * ZAPPER_FULLDAMAGE)))
			{	
				x = x/2;
				y = y/2;
				z = z/2;
				damage = damage/2;
			}
			
			byte waterlevel = pVictim->GetWaterLevel();
			if(waterlevel != WL_NotInWater)
			{
				damage += damage * ((float)waterlevel * 0.1f);

				pVictim->AddSlowdown(eo_zapper_slowdown.GetFloat(), eo_zapper_slowdown_length.GetFloat() * (float)waterlevel,
					NULL);
			}
			// Check for mid-air people
			else if(!(pVictim->GetFlags() & FL_ONGROUND) && !pVictim->IsOnLadder() 
				&& (pVictim->GetMoveType() == MOVETYPE_WALK))
			{
				damage += damage * 0.4f;

				pVictim->AddSlowdown(eo_zapper_slowdown.GetFloat(), 1.0f,
					NULL);
				
			}

			//UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 1.0f, ZAPPER_RANGE, SHAKE_START);
			pVictim->ViewPunch(QAngle(x, y, z));

			CTakeDamageInfo takedamage(this, this, damage, DMG_SHOCK);
			pVictim->OnTakeDamage(takedamage);
			bPlaySound = true;

			beamCount++;
		}
	}

	if(beamCount > 0)
	{
		IGameEvent * event = gameeventmanager->CreateEvent("zapper_discharge");

		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetInt("count", beamCount);
			gameeventmanager->FireEvent(event);
		}
	}


	if(bPlaySound)
	{
		EmitSound(ZAPPER_SOUND);
	}

	SetNextRun(gpGlobals->curtime + ZAPPER_INTERVAL);
}
#else
void COverlordZapper::FireGameEvent(IGameEvent * event)
{
	BaseClass::FireGameEvent(event);

	static char sprite[] = ZAPPER_SPRITE;
	if(event)
	{
		if(!Q_stricmp(event->GetName(), "zapper_discharge"))
		{
			int entind = event->GetInt("entindex");

			if(entind == entindex())
			{
				int count = event->GetInt("count");

				CTeslaInfo teslaInfo;

				teslaInfo.m_vAngles.Init();

				teslaInfo.m_nEntIndex = entindex();
				teslaInfo.m_vPos = WorldSpaceCenter();

				teslaInfo.m_flRadius = ZAPPER_RANGE;

				teslaInfo.m_vColor.x = 255 / 255.0f;
				teslaInfo.m_vColor.y = 255 / 255.0f;
				teslaInfo.m_vColor.z = 255 / 255.0f;
				
				//float flAlpha = 255 / 255;

				teslaInfo.m_nBeams = count * eo_zapper_beams_per_player.GetInt();
				
				teslaInfo.m_flBeamWidth = 4;
				teslaInfo.m_flTimeVisible = eo_zapper_interval.GetFloat();

				teslaInfo.m_pszSpriteName = sprite;

				FX_Tesla(teslaInfo);
			}
		}
	}
}
#endif

//=========================================================================================================

#ifdef CLIENT_DLL
#define COverlordHomer C_OverlordHomer
#endif

ConVar eo_homer_health("eo_homer_health", "40", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_homer_warmup_time("eo_homer_warmup_time", "3.0", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_homer_initial_dormant_range("eo_homer_initial_dormant_range", "3096", FCVAR_CHEAT | FCVAR_REPLICATED);
ConVar eo_homer_warmup_speed("eo_homer_warmup_speed", "40", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_explosion_range("eo_homer_explosion_range", "24", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_move_speed("eo_homer_move_speed", "625", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_homer_lift_speed("eo_homer_lift_speed", "240", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_lift_distance("eo_homer_lift_distance", "52", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_homer_sound_interval_min("eo_homer_sound_interval_min", "3.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_sound_interval_max("eo_homer_sound_interval_max", "7.5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_damage("eo_homer_damage", "320", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_homer_magnitude("eo_homer_magnitude", "200", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_homer_points("eo_homer_points", "50", FCVAR_REPLICATED | FCVAR_CHEAT);

#define WARMUP_DURATION eo_homer_warmup_time.GetFloat()
#define WARMUP_SPEED eo_homer_warmup_speed.GetFloat()

#define HOMER_HULL_VECTOR Vector(18, 18, 18)

#define HOMER_HELLO_SOUND "traps.homer_hello"
#define HOMER_MOVE_SOUND "traps.homer_move"
#define HOMER_BEAM_MATERIAL "sprites/bluelaser1.vmt"
#define HOMER_SPOTLIGHT_MATERIAL "sprites/glow01.vmt"

#define HOMER_BEAM_GLOW "sprites/light_glow03.vmt"

class COverlordHomer : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordHomer, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	COverlordHomer();
	virtual ~COverlordHomer();

	virtual void Precache();

	virtual void InitializeTrap();

	//==================================================================================================
	virtual Vector GetEmitterPosition() 
	{
		Vector emitter;
		if(GetAttachment( LookupAttachment("trackLaser"), emitter))
			return emitter;

		return WorldSpaceCenter();
	};

	virtual float GetInitialDormantRange() const { return eo_homer_initial_dormant_range.GetFloat(); };
	virtual float  GetNormalMultiplier() const { return 29; };

	virtual bool  CanBeDormant() const { return m_iState == 0; };

	virtual QAngle  NormalToAngle(const Vector & normal) const
	{
		return AlignToTop(normal);
	}
	//==================================================================================================	
#ifndef CLIENT_DLL
	void HomerTouch( CBaseEntity * pOther );
	virtual void RunTrap();

	virtual bool		CanDisableTrap() const { return m_iState == 0; };
	virtual bool ShouldDecay() const { return BaseClass::ShouldDecay() && m_iState == 0; };

	
	virtual bool CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
		return true;
	}

	virtual int GetPointsOnDestroyed() const { return eo_homer_points.GetInt(); };
#else
	virtual float GetPanelDistanceMultiplier() const { return 0.60f; };
	virtual float GetPanelZDistance() const { return 6.0f; };
#endif

private:
#ifndef CLIENT_DLL
	void Sweep();
	void Warmup();
	void Move();

	CBasePlayer * FindTarget();
	bool		  HasLOS(CBasePlayer * pPlayer);

	void		  Explode(bool bCollision = false);

	void		  UpdateSoundTime() { m_flNextSound = gpGlobals->curtime + random->RandomFloat(eo_homer_sound_interval_min.GetFloat(),
						eo_homer_sound_interval_max.GetFloat()); };
	
	void CreateBeam();
	void UpdateBeam();

	bool CanMoveUp(float dist) const;

	float m_flStartWarmup;
	float m_flNextSound;
	float m_flGracePeriod;

	int m_iAttachment;

	int m_BeamHalo;
	int m_SpotlightHalo;
	CHandle<CBeam> m_Beam;
	CHandle<CBeam> m_Spotlight;
	//CHandle<CSprite> m_BeamSprite;
#endif
	CNetworkHandle(CBasePlayer, m_Target);
	CNetworkVar(int, m_iState);
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordHomer, DT_OverlordHomer )

BEGIN_NETWORK_TABLE( COverlordHomer, DT_OverlordHomer )
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_iState)),
	SendPropEHandle(SENDINFO(m_Target)),
#else
	RecvPropInt(RECVINFO(m_iState)),
	RecvPropEHandle(RECVINFO(m_Target)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordHomer)
END_PREDICTION_DATA()
#endif

#ifndef CLIENT_DLL
BEGIN_DATADESC(COverlordHomer)

	DEFINE_ENTITYFUNC(HomerTouch),

END_DATADESC()
#endif

// "Halt for a moment and let's have a talk..."
// "Good day! Please wait a minute!"
// "Good day sir!" *BLAM**
// "After some rest... time... for a discussion"
// "Remember to stop by"
// "Trivia! Category: Politics! Who was a nameless simpleton who was found dead, 
//  obliterated by an unidentified explosion? Time out!"

LINK_TRAP_TO_ENTITY(trap_homer, COverlordHomer, Homer, 95, 80, "Converted's poetic solution to all your rebel problems: 10 kilograms of explosives slowly beeping it's way towards them. Because there's no kill, like overkill!",
					"Offense");

COverlordHomer::COverlordHomer()
{
	SET_TRAP_MODEL(models/dynamic_traps/homer/homer.mdl);

	m_iState = 0;
	m_Target = NULL;

#ifndef CLIENT_DLL
	m_iAttachment = 0;
	//m_BeamSprite = NULL;
	m_Beam = NULL;
	m_Spotlight = NULL;
	m_flNextSound = 0.0f;
	m_flStartWarmup = 0.0f;
	m_flGracePeriod = 0.0f;
	m_SpotlightHalo = NULL;
	m_BeamHalo =  NULL;
#endif
}

COverlordHomer::~COverlordHomer()
{
#ifndef CLIENT_DLL
	if(m_Beam)
	{
		UTIL_Remove(m_Beam);
	}

	if(m_Spotlight)
		UTIL_Remove(m_Spotlight);
	//if(m_BeamSprite)
	//	UTIL_Remove(m_BeamSprite);
#endif
}

void COverlordHomer::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(HOMER_HELLO_SOUND);
	PrecacheScriptSound(HOMER_MOVE_SOUND);
#ifndef CLIENT_DLL
	m_BeamHalo = PrecacheModel(HOMER_BEAM_MATERIAL);
	m_SpotlightHalo = PrecacheModel(HOMER_SPOTLIGHT_MATERIAL);
#endif
}
void COverlordHomer::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_homer_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	SetNextRun(gpGlobals->curtime + 0.1f);

	SetMoveType(MOVETYPE_FLY, MOVECOLLIDE_DEFAULT);
	SetSolid(SOLID_VPHYSICS);

	// Reset state just to be sure
	m_iState = 0;
	m_flStartWarmup = 0.0f;

	m_iAttachment = LookupAttachment("trackLaser");
	SetTouch(&COverlordHomer::HomerTouch);
#endif
}

#ifndef CLIENT_DLL
void COverlordHomer::HomerTouch(CBaseEntity * pOther)
{
	if(m_flGracePeriod > gpGlobals->curtime)
		return;

	if(m_iState > 1)
	{
		if(ToBasePlayer(pOther))
			Explode();
		else
			Explode(true);
	}
}

void COverlordHomer::RunTrap()
{
	float flNextRun = 0.1f;
	if(m_iState == 0)
	{
		Sweep();
	}
	else if(m_iState == 1)
	{
		flNextRun = 0.08f;
		Warmup();
	}
	else if(m_iState == 2)
	{
		Move();
		flNextRun = 0.035f;
	}
	
	SetNextRun(gpGlobals->curtime + flNextRun);
}

void COverlordHomer::Sweep()
{
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels)
		return;

	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer || !pPlayer->IsAlive())
			continue;

		// Just start the warmup when a player is spotted, we will select
		// a target later
		if(HasLOS(pPlayer))
		{
			m_flStartWarmup = gpGlobals->curtime;
			ResetSequence(LookupSequence("activate"));
			m_iState = 1;
			EmitSound(HOMER_HELLO_SOUND);
			break;
		}
	}
}

void COverlordHomer::Warmup()
{
	StudioFrameAdvance();

	float dist = WARMUP_SPEED * (gpGlobals->curtime - GetLastRun());

	if(CanMoveUp(dist))
	{
		SetAbsOrigin(GetAbsOrigin() + Vector(0, 0, dist));
	}

	// ROtate towards nearest player
	CBasePlayer * pPlayer = FindTarget();

	if(pPlayer)
	{
		Vector dir = -(GetEmitterPosition() - pPlayer->WorldSpaceCenter());
		VectorNormalize(dir);

		QAngle aDir;
		VectorAngles(dir, aDir);
		QAngle aForw = GetAbsAngles();

		float xDiff = UTIL_AngleDistance(aDir.x, aForw.x);
		float yDiff = UTIL_AngleDistance(aDir.y, aForw.y);

		const float MAX_ROTATION = 12.0f;
		if(fabs(xDiff) >= MAX_ROTATION)
			xDiff = (xDiff > 0) ? MAX_ROTATION : -MAX_ROTATION;
		if(fabs(yDiff) >= MAX_ROTATION)
			yDiff = (yDiff > 0) ? MAX_ROTATION : -MAX_ROTATION;

		SetAbsAngles(aForw + QAngle(xDiff, yDiff, 0));
	}

	if((m_flStartWarmup + WARMUP_DURATION) <= gpGlobals->curtime)
	{
		UpdateSoundTime();		
		m_iState = 2;
		SetDestroyable(0);
		m_flGracePeriod = gpGlobals->curtime + 0.25f;

		/*m_BeamSprite = CSprite::SpriteCreate(HOMER_BEAM_GLOW, GetAbsOrigin(), false);
		if(m_BeamSprite)
		{
			m_BeamSprite->SetAttachment( this, m_iAttachment );
			m_BeamSprite->SetTransparency( kRenderGlow, 255, 0, 0, 255, kRenderFxNoDissipation );
			m_BeamSprite->SetBrightness( 255 );
			m_BeamSprite->SetScale( 8.0f );
			m_BeamSprite->TurnOff();
		}*/

		ResetSequence(LookupSequence("fly"));

		CreateBeam();
	}
}

void COverlordHomer::Move()
{
	// Check for water
	if((UTIL_PointContents(GetEmitterPosition()) & CONTENTS_SLIME) ||
		(UTIL_PointContents(GetEmitterPosition()) & CONTENTS_WATER) )
	{
		Vector org = GetAbsOrigin();
		QAngle ang = GetAbsAngles();

		CPhysicsProp * pEnt = static_cast<CPhysicsProp*>(CreateEntityByName("prop_physics_multiplayer"));

		if(pEnt)
		{
			pEnt->SetAbsOrigin(org);
			pEnt->SetAbsAngles(ang);
			pEnt->SetModel(GetTrapModel());
			pEnt->ResetSequence(pEnt->LookupSequence("activate"));
			pEnt->SetCycle(1.0f);
			pEnt->Spawn();
		}
		
		UTIL_Remove(this);
		return;
	}

	StudioFrameAdvance();

	if(!m_Target || !m_Target->IsRebel() || !m_Target->IsAlive() || !HasLOS(m_Target))
	{
		m_Target = FindTarget();
	}

	if(!m_Target)
	{
		Explode(true);
	}
	else
	{
		UpdateBeam();

		Vector eyePos = m_Target->WorldSpaceCenter();
		Vector pos = GetAbsOrigin();
		Vector dir = eyePos - pos;
		VectorNormalize(dir);

		if((GetEmitterPosition() - m_Target->WorldSpaceCenter()).Length() <= eo_homer_explosion_range.GetFloat())
		{
			Explode();
			return;
		}

		float speed = (gpGlobals->curtime - GetLastRun()) * eo_homer_move_speed.GetFloat();
		
		// Collision check!
		/*trace_t tr;
		UTIL_TraceHull(pos, pos + dir * speed, -HOMER_HULL_VECTOR, HOMER_HULL_VECTOR, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		if((tr.fraction < 1.0f) && (m_flGracePeriod <= gpGlobals->curtime))
		{
			if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
				Explode(false);
			else
				Explode(true);

			return;
		}*/

		QAngle ang;
		VectorAngles(dir, ang);
		SetAbsAngles(ang);

		pos = pos + dir * speed;

		float distance = eo_homer_lift_distance.GetFloat();
		speed = eo_homer_lift_speed.GetFloat() * (gpGlobals->curtime - GetLastRun());

		// Lift correction
		trace_t tr;
		UTIL_TraceHull(pos, pos - Vector(0, 0, 1) * distance, -Vector(2, 2, 2), Vector(2, 2, 2), MASK_SHOT_HULL,
			this, COLLISION_GROUP_NONE, &tr);

		if(tr.fraction < 1.0f)
		{
			if(CanMoveUp(speed))
				pos = pos + Vector(0, 0, 1) * speed;
		}

		SetAbsOrigin(pos);

		if(m_flNextSound != 0.0f && m_flNextSound <= gpGlobals->curtime)
		{
			EmitSound(HOMER_MOVE_SOUND);
			UpdateSoundTime();
		}
	}
}

CBasePlayer * COverlordHomer::FindTarget()
{
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels)
		return NULL;

	CBasePlayer * pReturn = NULL;
	float fDist = FLT_MAX;
	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer || !pPlayer->IsAlive())
			continue;

		if((pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length() < fDist)
		{
			if(!HasLOS(pPlayer))
				continue;

			fDist = (pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length();
			pReturn = pPlayer;
		}

	}

	if(pReturn)
		return pReturn;

	if(m_Target && m_Target->IsRebel() && m_Target->IsAlive())
	{
		return m_Target;
	}

	pReturn = NULL;
	fDist = FLT_MAX;
	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer || !pPlayer->IsAlive())
			continue;

		if((pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length() < fDist)
		{
			fDist = (pPlayer->WorldSpaceCenter() - GetEmitterPosition()).Length();
			pReturn = pPlayer;
		}

	}

	if(pReturn)
		return pReturn;

	return NULL;
}

bool COverlordHomer::HasLOS(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return false;

	trace_t tr;
	//UTIL_TraceLine(GetEmitterPosition(), pPlayer->WorldSpaceCenter(), MASK_SHOT_HULL, this,
	//	COLLISION_GROUP_NONE, &tr);

	UTIL_TraceHull(GetEmitterPosition(), pPlayer->WorldSpaceCenter(), 
		-Vector(3, 3, 3), Vector(3, 3, 3), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	if(tr.m_pEnt == pPlayer || tr.fraction >= 1.0f)
	{
		return true;
	}

	return false;
}

void COverlordHomer::Explode(bool bCollision /*= false*/)
{
	m_bUseDefenses = false;

	int damage = eo_homer_damage.GetInt();
	int magnitude = eo_homer_magnitude.GetInt();

	if(bCollision)
	{
		damage = damage/2;
		magnitude = magnitude/2;
	}

	ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), this, damage,
		magnitude, true, magnitude );

	UTIL_ScreenShake(GetAbsOrigin(), 90.0f, 150.0f, 3.5f, 128, SHAKE_START);
	CSoundEnt::InsertSound ( SOUND_COMBAT, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), EXPLOSION_VOLUME, 3.0 );	

	UTIL_Remove(this);
	SetNextRun(NEVER_RUN);
}

void COverlordHomer::CreateBeam()
{
	if(m_Beam)
	{
		UTIL_Remove(m_Beam);
		m_Beam = NULL;
	}

	m_Beam = CBeam::BeamCreate(HOMER_BEAM_MATERIAL, 1);
	if(m_Beam)
	{
		m_Beam->SetType(TE_BEAMPOINTS);
		m_Beam->SetWidth(12);
		m_Beam->SetEndWidth(12);
		m_Beam->SetBrightness(255);
		m_Beam->SetHaloTexture(m_BeamHalo);
		m_Beam->SetHaloScale(4.0f);
		m_Beam->SetColor( 255, 0, 0 );
		m_Beam->SetNoise(0.0f);
		m_Beam->SetScrollRate(150.0f);
		m_Beam->SetBeamFlags(FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM);
		m_Beam->TurnOff();
	}

	if(m_Spotlight)
	{
		UTIL_Remove(m_Spotlight);
		m_Spotlight = NULL;
	}

	m_Spotlight = CBeam::BeamCreate(HOMER_SPOTLIGHT_MATERIAL, 1);
	if(m_Spotlight)
	{
		Vector dir;
		AngleVectors(GetAbsAngles(), &dir);
		
		m_Spotlight->SetType(TE_BEAMPOINTS);
		m_Spotlight->SetHaloTexture(m_SpotlightHalo);
		m_Spotlight->SetHaloScale(4.0f);
		m_Spotlight->SetWidth(10.0f);
		m_Spotlight->SetEndWidth(16.0f);
		m_Spotlight->SetFadeLength(200.0f);
		m_Spotlight->SetNoise(0.0f);
		m_Spotlight->SetBrightness(60.0f);
		m_Spotlight->SetScrollRate(0.0f);
		m_Spotlight->SetFrame(0.0f);
		m_Spotlight->SetColor(255, 0, 0);
		m_Spotlight->SetBeamFlags(FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM);
		m_Spotlight->TurnOff();
	}

	
}

void COverlordHomer::UpdateBeam()
{
	Vector attach;
	GetAttachment(m_iAttachment, attach);
	if(m_Beam)
	{
		m_Beam->SetStartPos(attach);

		trace_t tr;
		UTIL_TraceLine(attach, m_Target->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr); 
		m_Beam->SetEndPos(tr.endpos);

		m_Beam->RelinkBeam();
	}

	if(m_Spotlight)
	{
		Vector dir;
		AngleVectors(GetAbsAngles(), &dir);

		m_Spotlight->SetStartPos(attach);

		CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);
		trace_t tr;
		UTIL_TraceLine(attach, attach + dir * 250.0f, MASK_SHOT, &filter, &tr);

		m_Spotlight->SetEndPos(tr.endpos);

		m_Spotlight->RelinkBeam();
	}
}

bool COverlordHomer::CanMoveUp(float dist) const
{
	trace_t tr;
	UTIL_TraceHull(WorldSpaceCenter(), WorldSpaceCenter() + Vector(0, 0, 32) + Vector(0, 0, dist), 
		-Vector(4, 4, 4), Vector(4, 4, 4), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	return tr.fraction >= 1.0f;
}

#endif

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordLink C_OverlordLink
#endif

ConVar eo_link_radius("eo_link_radius", "110", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_damage("eo_link_damage", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_damage_delay("eo_link_damage_delay", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_break_range("eo_link_break_range", "396", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_break_time("eo_link_break_time", "1.25", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_initial_dormant_range("eo_link_initial_dormant_range", "120", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_link_lifetime("eo_link_lifetime", "20", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_link_max_points("eo_link_max_points", "30", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_link_points("eo_link_points", "15", FCVAR_REPLICATED | FCVAR_CHEAT);

#define LINK_RADIUS eo_link_radius.GetFloat()
#define LINK_HALF_MODEL "models/dynamic_traps/linker/linkerhalf.mdl"

#define LINK_LOOP_SOUND "d3_citadel.shield_touch_loop"
#define LINK_BREAK_SOUND "NPC_CombineBall.KillImpact"

class COverlordLink : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordLink, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	COverlordLink();
	virtual ~COverlordLink();

#ifndef CLIENT_DLL
	virtual void Precache();
	virtual void DecayTrap();
#endif
	virtual void InitializeTrap();
	virtual error_t CanBuild();

	virtual float GetInitialDormantRange() const { return eo_link_initial_dormant_range.GetFloat(); };
	virtual bool  UseLOSForInitialDormant() const { return true; };
	virtual float GetNormalMultiplier() const { return 6.0f; };

	virtual Vector GetRadiusOrigin() { return WorldSpaceCenter(); };
	virtual float GetRadiusRange() { return LINK_RADIUS; };

	virtual bool  ShouldGib() const { return false; };

#ifndef CLIENT_DLL
	virtual void RunTrap();

	CBasePlayer * GetTarget1() const { return m_Target1; };
	CBasePlayer * GetTarget2() const { return m_Target2; };

	virtual bool ShouldDecay() const;

	virtual int  GetPointsOnDestroyed() { return eo_link_points.GetInt(); };
#else
	virtual float GetPanelDistanceMultiplier() const { return 0.625f; };
	virtual float GetPanelZDistance() const { return 0.0f; };
#endif

private:
#ifndef CLIENT_DLL
	void HandleLinked();
	CBasePlayer * GetNearestTarget() const;
	Vector GetAttachPosition(const CBasePlayer * pPlayer) const;
	bool AreTargetsInLOS() const;

	CHandle<CBasePlayer> m_Target1;
	CHandle<CBasePlayer> m_Target2;
	CHandle<CBaseAnimating> m_Twin;
	CHandle<CBeam> m_Beam;

	float m_flNextDamage;
	float m_flLOSFailTime;
	float m_flDeathTime;
	int m_iPointsSubtract;
#endif
	CNetworkVar(int, m_State);
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordLink, DT_OverlordLink )

BEGIN_NETWORK_TABLE( COverlordLink, DT_OverlordLink )
#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_State)),
#else
	RecvPropInt(RECVINFO(m_State)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordLink)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_link, COverlordLink, Tandem, 55, 60, "Links two rebels and deals damage as long as they are in LOS with each other",
					"Offense");

COverlordLink::COverlordLink()
{
	m_State = 0;
	SET_TRAP_MODEL(models/dynamic_traps/linker/linker.mdl);

#ifndef CLIENT_DLL
	m_Beam = NULL;
	m_Target1 = NULL;
	m_Target2 = NULL;
	m_Twin = NULL;
	m_flNextDamage = 0.0f;
	m_flLOSFailTime = 0.0f;
	m_flDeathTime = 0.0f;
	m_iPointsSubtract = 0;
#endif
}

COverlordLink::~COverlordLink()
{
#ifndef CLIENT_DLL
	if(m_Twin)
	{
		UTIL_Remove(m_Twin);
		m_Twin = NULL;
	}

	if(m_Beam)
	{
		UTIL_Remove(m_Beam);
		m_Beam = NULL;
	}

	// Give points
	if(m_State >= 1)
	{
		int points = eo_link_max_points.GetInt() - m_iPointsSubtract;

		bool bBothAlive = (m_Target1 && m_Target1->IsRebel() && m_Target1->IsAlive()) &&
			(m_Target2 && m_Target2->IsRebel() & m_Target2->IsAlive());

		if(bBothAlive)
		{
			if(m_Target1)
				m_Target1->IncreaseOverlordDamage(points);
			if(m_Target2)
				m_Target2->IncreaseOverlordDamage(points);
		}
	}

	StopSound(LINK_LOOP_SOUND);
#endif
}

#ifndef CLIENT_DLL
void COverlordLink::Precache()
{
	BaseClass::Precache();

	PrecacheModel(LINK_HALF_MODEL);
	PrecacheModel("sprites/laserbeam.vmt");
	PrecacheScriptSound(LINK_LOOP_SOUND);
	PrecacheScriptSound(LINK_BREAK_SOUND);
}

void COverlordLink::DecayTrap()
{
	BaseClass::DecayTrap();

	if(m_Twin)
	{
		UTIL_Remove(m_Twin);
		m_Twin = NULL;
	}

	if(m_Beam)
	{
		UTIL_Remove(m_Beam);
		m_Beam = NULL;
	}

	StopSound(LINK_LOOP_SOUND);
}

#endif

void COverlordLink::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(20);
	
	m_bUseDefenses = false;

	SetNextRun(gpGlobals->curtime + 0.1f);
#endif
}

error_t COverlordLink::CanBuild()
{
	if(IsPlayerInRadius(PLAYER_RADIUS + LINK_RADIUS))
	{
		return CreateError("Player is in radius");
	}

	return BaseClass::CanBuild();
}
#ifndef CLIENT_DLL

void COverlordLink::RunTrap()
{
	if(m_State == 0)
	{
		CBasePlayer * pPlayer = GetNearestTarget();
		if(pPlayer)
		{
			m_Target1 = pPlayer;
			m_State++;
			SetSolid(SOLID_NONE);	
		}
	}
	else if(m_State == 1)
	{
		if(!GetTarget1() || !GetTarget1()->IsAlive() || !GetTarget1()->IsRebel())
		{
			DecayTrap();
			return;
		}

		// Update our position
		SetAbsOrigin(GetAttachPosition(GetTarget1()));

		CBasePlayer * pPlayer = GetNearestTarget();

		if(pPlayer)
		{
			m_Target2 = pPlayer;
			SetModel(LINK_HALF_MODEL);
			m_Twin = static_cast<CBaseAnimating*>(CreateEntityByName("prop_dynamic"));

			if(!m_Twin)
			{
				Warning("Link: Twin creation error\n");
				UTIL_Remove(this);
				return;
			}

			m_Twin->SetModel(LINK_HALF_MODEL);
			m_Twin->SetAbsOrigin(GetAttachPosition(GetTarget2()));
			m_Twin->Spawn();
			m_Twin->ResetSequence(m_Twin->LookupSequence("attack"));

			m_Beam = CBeam::BeamCreate( "sprites/laserbeam.vmt", 2.0f);
			m_Beam->PointEntInit(WorldSpaceCenter(), this);
			m_Beam->SetStartEntity(this);
			m_Beam->SetEndEntity(m_Twin);
			m_Beam->SetWidth( 1 );
			m_Beam->SetEndWidth( 1 );
			m_Beam->SetBrightness( 125 );
			m_Beam->SetColor( 255, 255, 255 );
			m_Beam->RelinkBeam();
			m_Beam->SetNoise( 4 );
			ResetSequence(LookupSequence("attack"));
			EmitSound(LINK_LOOP_SOUND);

			m_State++;
			m_flDeathTime = gpGlobals->curtime + eo_link_lifetime.GetFloat();
			m_flNextDamage = gpGlobals->curtime + eo_link_damage_delay.GetFloat();
			SetDestroyable(0);
		}
	}
	else
	{
		HandleLinked();
	}

	SetNextRun(gpGlobals->curtime + 0.085f);
}

void COverlordLink::HandleLinked()
{
	if(!GetTarget1() || !GetTarget2() || !GetTarget1()->IsAlive() || !GetTarget1()->IsRebel() 
		|| !GetTarget2()->IsAlive() 
		|| !GetTarget2()->IsRebel())
	{
		DecayTrap();
		return;
	}

	if(!m_Twin)
	{
		Warning("No twin for the link!\n");
		UTIL_Remove(this);
		return;
	}

	// Update Position
	SetAbsOrigin(GetAttachPosition(GetTarget1()));
	m_Twin->SetAbsOrigin(GetAttachPosition(GetTarget2()));
	m_Twin->StudioFrameAdvance();

	StudioFrameAdvance();
	Vector dir = GetAbsOrigin() - m_Twin->GetAbsOrigin();
	VectorNormalize(dir);
	QAngle forw1, forw2;
	VectorAngles(dir, forw1);
	SetAbsAngles(forw1);

	VectorAngles(-dir, forw2);

	m_Twin->SetAbsAngles(forw2);

	if(AreTargetsInLOS())
	{
		if(m_flLOSFailTime != 0.0f)
		{
			m_flLOSFailTime = 0.0f;

			if(m_Beam)
			{
				EmitSound(LINK_LOOP_SOUND);
				m_Beam->TurnOff();
			}
		}

		if(m_flNextDamage <= gpGlobals->curtime)
		{
			CBasePlayer * pTarget1 = GetTarget1();
			CBasePlayer * pTarget2 = GetTarget2();
			float damage = eo_link_damage.GetFloat();

			float x = random->RandomFloat(-4.0f, 4.0f);
			float y = random->RandomFloat(-4.0f, 4.0f);
			float z = random->RandomFloat(-4.0f, 4.0f);
			QAngle angle(x, y, z);
			pTarget1->OnTakeDamage(CTakeDamageInfo(this, this, damage, DMG_SHOCK));
			pTarget2->OnTakeDamage(CTakeDamageInfo(this, this, damage, DMG_SHOCK));
			pTarget1->ViewPunch(angle);
			pTarget2->ViewPunch(angle);

			m_iPointsSubtract += damage;

			Ray_t ray;
			ray.Init(GetAbsOrigin(), m_Twin->GetAbsOrigin(), -Vector(2, 2, 2), Vector(2, 2, 2));
			CBaseEntity * pList[1024];
			int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

			for(int i = 0; i < count; i++)
			{
				CBasePlayer * pPlayer = ToBasePlayer(pList[i]);

				if(!pPlayer || pPlayer == GetTarget1() || pPlayer == GetTarget2())
					continue;

				pPlayer->OnTakeDamage(CTakeDamageInfo(this, this, damage, DMG_SHOCK));
				pPlayer->ViewPunch(angle);
			}

			m_flNextDamage = gpGlobals->curtime + eo_link_damage_delay.GetFloat();
		}
	}
	else
	{
		if(m_flLOSFailTime == 0.0f)
		{
			m_flLOSFailTime = gpGlobals->curtime;
		
			if(m_Beam)
			{
				StopSound(LINK_LOOP_SOUND);
				m_Beam->TurnOn();
			}
		}
		else if(((m_flLOSFailTime + eo_link_break_time.GetFloat()) <= gpGlobals->curtime) &&
				(GetAbsOrigin() - m_Twin->GetAbsOrigin()).Length() >= eo_link_break_range.GetFloat())
		{
			// Break the link here
			UTIL_Remove(this);
			EmitSound(LINK_BREAK_SOUND);
			return;

		}
	}
}
CBasePlayer * COverlordLink::GetNearestTarget() const
{
	float mindist = LINK_RADIUS;
	CBasePlayer * pNearest = NULL;
	
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsAlive() || !pPlayer->IsRebel() || GetTarget1() == pPlayer || GetTarget2() == pPlayer)
			continue;

		float dist = (GetAbsOrigin() - pPlayer->WorldSpaceCenter()).Length();

		if(mindist > dist)
		{
			trace_t tr;
			UTIL_TraceLine(GetAbsOrigin(), pPlayer->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			if(tr.m_pEnt != pPlayer)
				continue;

			bool bContinue = false;
			for(int j = 0; j < GetOverlordData()->GetMaxTraps(); j++)
			{
				COverlordTrap * pTrap = GetOverlordData()->GetTrap(j);

				if(!pTrap)
					continue;

				if(!Q_stricmp(pTrap->GetClassname(), "trap_link"))
				{
					COverlordLink * pLink = static_cast<COverlordLink*>(pTrap);

					if(pLink->GetTarget1() == pPlayer || pLink->GetTarget2() == pPlayer)
					{
						bContinue = true;
						break;
					}
				}
			}

			if(bContinue)
				continue;

			mindist = dist;
			pNearest = pPlayer;
		}
	}

	return pNearest;
}

bool COverlordLink::ShouldDecay() const
{
	if(m_State == 0 || m_State == 1)
		return BaseClass::ShouldDecay();

	return m_flDeathTime <= gpGlobals->curtime;
}

Vector COverlordLink::GetAttachPosition(const CBasePlayer * pPlayer) const
{
	if(!pPlayer)
		return vec3_origin;
	
	Vector ret(0, 0, 0);
	Vector local(12, 12, 32);
	pPlayer->EntityToWorldSpace(local, &ret);

	return ret;
}

bool COverlordLink::AreTargetsInLOS() const
{
	if(!GetTarget1() || !GetTarget2() || !m_Twin)
		return false;

	CTraceFilterNoNPCsOrPlayer filter(GetTarget1(), COLLISION_GROUP_NONE);
	trace_t tr;
	UTIL_TraceLine(GetTarget1()->RealEyePosition(), GetTarget2()->RealEyePosition(), MASK_SHOT, &filter, &tr);

	if(tr.m_pEnt == GetTarget2() || tr.fraction == 1.0f)
		return true;

	return false;
}
#endif

//====================================================================================================================================

#ifdef CLIENT_DLL
#define COverlordAddicter C_OverlordAddicter
#endif

ConVar eo_addicter_distance("eo_addicter_distance", "230.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_addicter_initial_dormant_range("eo_addicter_initial_dormant_range", "235.0", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_addicter_speed("eo_addicter_speed", "720", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_addicter_health("eo_addicter_health", "40", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_addicter_points("eo_addicter_points", "10", FCVAR_REPLICATED | FCVAR_CHEAT);

#define ADDICTER_RADIUS eo_addicter_distance.GetFloat()

#ifdef CLIENT_DLL
namespace
{
	const char g_AddicterSounds[][40] =
	{
		"Weapon_Pistol.Burst",
		"Weapon_Pistol.Single",
		"Weapon_Shotgun.Double",
		"Weapon_Shotgun.Single",
		"Weapon_Shotgun.Special1",
		"Weapon_357.Single",
		"Weapon_SMG1.Single",
		//LASER_ZAP_SOUND,
		PIGEON_HOMING_SOUND,
		BOMBER_SOUND,
		PARALYSER_SOUND,
		FLAMER_SOUND,
		FLAMER_SOUND_NEW,
		FAN_SOUND,
		AIRSTRIKE_SOUND,
		STICKY_SOUND,
		DEVOURER_SOUND_ATTACK,
		DEVOURER_SOUND_ATTACK_IN,
		DEVOURER_SOUND_SNATCH,
		DEVOURER_SOUND_ATTACK_OUT,
		HOMER_HELLO_SOUND,
		HOMER_MOVE_SOUND,
		MINE_LOOP,
		ZAPPER_SOUND,
		LINK_LOOP_SOUND, 
		LINK_BREAK_SOUND,
		//ZAPPER_SOUND_LOOP,
		"BaseExplosionEffect.Sound",
	};
}
#endif

#define ADDICTER_TURNOFF_SOUND "NPC_CombineMine.TurnOff"
ConVar eo_addicter_runtime("eo_addicter_runtime", "90.0", FCVAR_REPLICATED | FCVAR_CHEAT);

#define ADDICTER_RUNTIME eo_addicter_runtime.GetFloat()
//#define ADDICTER_CLIENT_THINK 0.8f

class COverlordAddicter : public COverlordTrap
{
public:
	DECLARE_CLASS(COverlordAddicter, COverlordTrap);
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	COverlordAddicter();
	virtual ~COverlordAddicter();


	virtual void Precache()
	{

		BaseClass::Precache();
#ifdef CLIENT_DLL
		for(int i = 0; i < ARRAYSIZE(g_AddicterSounds); i++)
		{
			PrecacheScriptSound(g_AddicterSounds[i]);
		}
		m_FireballIndex = PrecacheModel("sprites/zerogxplode.vmt");
#else
		PrecacheParticleSystem("AddicterEngine");
		PrecacheScriptSound(ADDICTER_TURNOFF_SOUND);
#endif

	}

	virtual void InitializeTrap();
	virtual error_t CanBuild();

	virtual bool  CanRestoreDuration() const { return m_Target == NULL; };
	virtual bool  CanRestoreHealth() const { return m_Target == NULL; };
	virtual bool CanBeDormant() const { return m_Target == NULL; };
	virtual float GetInitialDormantRange() const { return eo_addicter_initial_dormant_range.GetFloat(); };

	virtual float GetNormalMultiplier() const { return 3; };

	CBasePlayer * GetTarget() const { return m_Target.Get(); };

	virtual bool  ShouldGib() const { return false; };

	virtual QAngle  NormalToAngle(const Vector & normal) const
	{
		return AlignToTop(normal);
	}

#ifdef CLIENT_DLL
	virtual int DrawModel(int flags);
	virtual void TrapThink();

	virtual void FireGameEvent(IGameEvent * event);

	virtual float GetRadiusRange() { return ADDICTER_RADIUS; };

	virtual bool ShouldUseBeam() const { return false; };

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 0.625f; };
	virtual float GetPanelZDistance() const { return 0.0f; };
	virtual float GetPanelYDistance() const { return 0.0f; };
	virtual QAngle GetPanelAngle() const { QAngle angle = BaseClass::GetPanelAngle(); angle[YAW] += 90; return angle; };
#else
	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}
	virtual bool ShouldDecay() const { return (BaseClass::ShouldDecay() && m_flStickTime == 0.0f) ||
		(m_flStickTime != 0.0f && m_flStickTime <= gpGlobals->curtime); };

	virtual void RunTrap();

	virtual int OnTakeDamage(const CTakeDamageInfo & info);

	float GetDistanceToTarget() const;
	Vector GetTargetAttach() const;

	virtual int GetPointsOnDestroyed() const { return eo_addicter_points.GetInt(); };
#endif
	virtual void DecayTrap()
	{
		m_Target = NULL;

		EmitSound(ADDICTER_TURNOFF_SOUND);

		BaseClass::DecayTrap();

#ifdef CLIENT_DLL
	// Stop looping sounds
		for(int i = 0; i < ARRAYSIZE(g_AddicterSounds); i++)
			StopSound(g_AddicterSounds[i]);

		if(m_FakeTrap)
			m_FakeTrap->Remove();
#endif
	}
	

private:
#ifdef CLIENT_DLL
	bool IsTargetLocalPlayer() const { return (m_Target.Get() && (m_Target.Get() == C_BasePlayer::GetLocalPlayer())); };
	void SpawnFakeTrap(Vector position, Vector normal, CBaseEntity * pParent);
	void CreateParticleEffect();
	void DestroyParticleEffect();
#else
	void Sweep();
	void FlyToTarget();
	void StickToTarget();
#endif

#ifdef CLIENT_DLL
	int m_FireballIndex;
	float m_flDelay;

	CHandle<C_OverlordTrap> m_FakeTrap;
	float m_flRemoveTrap;
	float m_flDisableOverride;
	int m_LastHealth;

	CNewParticleEffect * m_hEffect;
#endif
	void FireRandomTrace(trace_t * tr) const;

	CNetworkVar(bool, m_bFlying);
	CNetworkHandle(CBasePlayer, m_Target);

#ifndef CLIENT_DLL
	float m_flStickTime;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordAddicter, DT_OverlordAddicter )

BEGIN_NETWORK_TABLE( COverlordAddicter, DT_OverlordAddicter )
#ifndef CLIENT_DLL
SendPropBool(SENDINFO(m_bFlying)),
SendPropEHandle(SENDINFO(m_Target)),
#else
RecvPropBool(RECVINFO(m_bFlying)),
RecvPropEHandle(RECVINFO(m_Target)),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordAddicter)
END_PREDICTION_DATA()
#endif

LINK_TRAP_TO_ENTITY(trap_addicter, COverlordAddicter, Addicter, 15, 70, "Sticks to one rebel for 100 seconds causing hallucinations.",
					"Utility");

COverlordAddicter::COverlordAddicter()
{
	SET_TRAP_MODEL(models/dynamic_traps/addictor/addictor.mdl);
	//SET_TRAP_MODEL(models/healthvial.mdl);

#ifndef CLIENT_DLL
	m_flStickTime = 0.0f;
	m_bFlying = false;
#else
	m_LastHealth = 0;
	m_flDelay = 0.0f;
	m_FakeTrap = NULL;
	m_flRemoveTrap = 0.0f;
	m_flDisableOverride = 0.0f;
	m_hEffect = NULL;
#endif
}

COverlordAddicter::~COverlordAddicter()
{
#ifdef CLIENT_DLL
	if(IsTargetLocalPlayer())
	{
		CBasePlayer * pLocal = CBasePlayer::GetLocalPlayer();
		// Stop looping sounds
		for(int i = 0; i < ARRAYSIZE(g_AddicterSounds); i++)
			StopSound(g_AddicterSounds[i]);

		if(m_FakeTrap)
			m_FakeTrap->Remove();

		if(pLocal)
		{
			pLocal->SetHealthOverride(-1);
		}
	}

	DestroyParticleEffect();
#endif
}

void COverlordAddicter::InitializeTrap()
{
#ifndef CLIENT_DLL
	SetDestroyable(eo_addicter_health.GetInt());
	SetExplodable(eo_traps_explosion_magnitude.GetFloat(), eo_traps_explosion_radius.GetFloat(), 
		eo_traps_explosion_force.GetFloat(), true);

	m_bUseDefenses = false;
	SetSolid(SOLID_VPHYSICS); 
	SetCollisionGroup(COLLISION_GROUP_DEBRIS);
	ResetSequence(LookupSequence("attack"));
#else
	ListenForGameEvent("addicter_fly");
	ListenForGameEvent("addicter_attach");
	SetNextClientThink(CLIENT_THINK_ALWAYS);
#endif
}

error_t COverlordAddicter::CanBuild()
{
	if(IsPlayerInRadius(ADDICTER_RADIUS + PLAYER_RADIUS))
	{
		return CreateError("Player is too close.");
	}


	return BaseClass::CanBuild();
}

#ifdef CLIENT_DLL
void COverlordAddicter::CreateParticleEffect()
{
	DestroyParticleEffect();

	m_hEffect = ParticleProp()->Create("AddicterEngine", PATTACH_POINT_FOLLOW, LookupAttachment("burnaExhaust"));
}

void COverlordAddicter::DestroyParticleEffect()
{
	if(m_hEffect)
	{
		ParticleProp()->StopEmission(m_hEffect);
		m_hEffect = NULL;
	}
}

extern void TE_Explosion( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float scale, int framerate, int flags, int radius, int magnitude, 
	const Vector* normal = NULL, unsigned char materialType = 'C', bool bShouldAffectRagdolls = true );


void COverlordAddicter::TrapThink()
{
	if(IsDecaying())
		return;

	if(m_flDelay == 0.0f)
		m_flDelay = gpGlobals->curtime;

	// Remove the trap if necessary
	if(m_FakeTrap && m_flRemoveTrap <= gpGlobals->curtime)
	{
		m_FakeTrap->Remove();
		m_FakeTrap = NULL;
		m_flRemoveTrap = 0.0f;
	}

	if(!IsTargetLocalPlayer())
		return;

	if(m_bFlying)
		return;

	CBasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();

	if(!pLocal)
		return;

	if(m_flDisableOverride > gpGlobals->curtime)
	{
		pLocal->SetHealthOverride(max(pLocal->GetHealthOverride() - (m_LastHealth - pLocal->GetHealth()), 1));

		m_LastHealth = pLocal->GetHealth();
	}
	else if(m_flDisableOverride != 0.0f)
	{
		pLocal->SetHealthOverride(-1);
		m_flDisableOverride = 0.0f;
		m_LastHealth = 0;
	}

	if(gpGlobals->curtime >= m_flDelay)
	{
		// Emit different sound etc. here

		// Sound or effect?
		int rand = random->RandomInt(0, 1);

		// Stop any looping sound here
		for(int i = 0; i < ARRAYSIZE(g_AddicterSounds); i++)
		{
			StopSound(g_AddicterSounds[i]);
		}

		if(rand == 0)
		{
			// Sound
			rand = random->RandomInt(0, ARRAYSIZE(g_AddicterSounds));
			EmitSound(g_AddicterSounds[rand]);
		}
		else if(rand == 1)
		{
			//Vector vRandom = RandomVector(-1, 1);
			//VectorNormalize(vRandom);

			trace_t tr;
			do
				FireRandomTrace(&tr);
			while(tr.surface.flags & SURF_SKY);

			
			
			Vector vEffect = tr.endpos;
			Vector vNormal = tr.plane.normal;
			QAngle aNormal;
			VectorAngles(vNormal, aNormal);
			// Effect
			rand = random->RandomInt(0, 2);
			CSingleUserRecipientFilter user(pLocal);
			switch(rand)
			{
			case 0:
				{
					TE_Explosion(user, 0.0f, &vEffect, m_FireballIndex, random->RandomInt(2, 8), 15, 0, 128, 128, &vNormal, 67, false);	
					//FX_Explosion(vEffect, vNormal, 'W');
					break;
				}
			case 1:
				{
					if(m_flRemoveTrap > gpGlobals->curtime)
						break;
					else
					{
						if(m_FakeTrap)
						{
							m_FakeTrap->Remove();
							m_FakeTrap = NULL;
							m_flRemoveTrap = 0.0f;
						}
					}

					Vector vDir = tr.endpos - m_Target->RealEyePosition();
					VectorNormalize(vDir);

					if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
					{
						Vector start = tr.endpos;
						Vector end;
						VectorMA(start, MAX_TRACE_LENGTH, vDir, end);

						UTIL_TraceLine(tr.endpos, end, MASK_SHOT, tr.m_pEnt, COLLISION_GROUP_NONE, &tr);

						// Clearly we are not destinied to build this trap, break
						if(tr.m_pEnt && tr.m_pEnt->IsPlayer())
							break;
					}
					SpawnFakeTrap(tr.endpos, tr.plane.normal, tr.m_pEnt);
					break;
				}

			case 2:
				{
					if(m_flDisableOverride > gpGlobals->curtime)
						break;

					if(pLocal->GetHealth() <= 1)
						break;

					int rand = random->RandomInt(1, pLocal->GetMaxHealth());
					pLocal->SetHealthOverride(rand);
					m_flDisableOverride = gpGlobals->curtime + random->RandomFloat(10.0f, 15.0f);
					m_LastHealth = pLocal->GetHealth();
					break;
				}
			}
		}
		m_flDelay = gpGlobals->curtime + random->RandomFloat(1.5f, 3.5f);
	}


	SetNextClientThink( CLIENT_THINK_ALWAYS );


}

void COverlordAddicter::FireGameEvent(IGameEvent * event)
{
	BaseClass::FireGameEvent(event);

	if(event)
	{
		if(!Q_stricmp(event->GetName(), "addicter_fly"))
		{
			if(event->GetInt("entindex") != entindex())
				return;

			CreateParticleEffect();
		}
		else if(!Q_stricmp(event->GetName(), "addicter_attach"))
		{
			if(event->GetInt("entindex") != entindex())
				return;

			DestroyParticleEffect();
		}
	}
}

void COverlordAddicter::SpawnFakeTrap(Vector position, Vector normal, CBaseEntity * pParent)
{
	if(m_FakeTrap)
	{
		m_FakeTrap->Remove();
		m_FakeTrap = NULL;
	}

	int index = random->RandomInt(0, NUM_OF_TRAPS-1);

	const C_OverlordTrap::STrapRecord & record = C_OverlordTrap::GetRecord(index);

	C_OverlordTrap * pTrap = static_cast<C_OverlordTrap*>(CreateEntityByName(record.entName));

	if(!pTrap || pTrap->InitializeAsClientEntity(NULL, RENDER_GROUP_OPAQUE_ENTITY) == false)
	{
		Warning("Failed to initialize a fake trap!\n");
		if(pTrap)
		{
			Warning("Removing the fake trap\n");
			pTrap->Remove();
		}

		return;
	}

	if(pParent)
		pTrap->SetParent(pParent);

	// NOT a ghost
	pTrap->SetGhost(false);
	
	pTrap->SetRenderMode(kRenderNormal);
		
	// using consoles build parameters
	pTrap->SetAbsOrigin(position + (normal * pTrap->GetNormalMultiplier()));
	// Calculate perpendicular angles
	pTrap->SetAbsAngles(pTrap->NormalToAngle(normal));

	pTrap->Spawn();

	// Set dormant to prevent any particle effects etc.
	pTrap->SetTrapDormant(true);
	m_FakeTrap = pTrap;

		// Initialize boobytrap's vphysics
	if(!Q_stricmp("trap_bomber", record.entName))
	{
		pTrap->SetParent(NULL);
		pTrap->CreateVPhysics();
		pTrap->SetMoveType(MOVETYPE_VPHYSICS);
	}

	m_flRemoveTrap = gpGlobals->curtime + random->RandomFloat(3.0f, 10.0f);
}	

int COverlordAddicter::DrawModel(int flags)
{
	if(IsTargetLocalPlayer() && !m_bFlying)
		return 0;
	else
		return BaseClass::DrawModel(flags);
}
#else
void COverlordAddicter::RunTrap()
{
	if(!m_Target)
	{
		Sweep();
	}
	else
	{
		if(!m_Target->IsAlive() || !m_Target->IsRebel())
		{
			UTIL_Remove(this);
			return;
		}

		if(m_bFlying)
			FlyToTarget();
		else
			StickToTarget();
	}

	SetNextRun(gpGlobals->curtime + 0.05f);
}

void COverlordAddicter::Sweep()
{
	CBasePlayer * pTarget = NULL;
	float dist = FLT_MAX;
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || pPlayer->IsInvisible() || !pPlayer->IsAlive() || !pPlayer->IsRebel())
			continue;

		float dis = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).Length();

		if(dis > ADDICTER_RADIUS)
			continue;

		// Has a line of sight?
		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), pPlayer->WorldSpaceCenter(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
		
		if(tr.m_pEnt != pPlayer)
			continue;

		// Check whether he is some other sticky's target
		bool bContinue = false;
		for(int j = 0; j < GetOverlordData()->GetMaxTraps(); j++)
		{
			COverlordTrap * pTrap = GetOverlordData()->GetTrap(j);

			if(!pTrap)
				continue;

			if(!Q_stricmp(pTrap->GetClassname(), "trap_addicter"))
			{
				COverlordAddicter * pAddicter = static_cast<COverlordAddicter*>(pTrap);

				if(pAddicter->GetTarget() == pPlayer)
				{
					bContinue = true;
					break;
				}
			}
		}

		if(bContinue)
			continue;


		if(dis < dist)
		{
			pTarget = pPlayer;
			dist = dis;
		}
	}

	if(pTarget)
	{
		m_Target = pTarget;
		m_flStickTime = gpGlobals->curtime + ADDICTER_RUNTIME;
		m_bFlying = true;
	
		SetDestroyable(false);
		ReleaseTrap();
		QAngle ang;
		VectorAngles(GetTargetAttach() - GetEmitterPosition(), ang);
		SetAbsAngles(ang);

		IGameEvent * event = gameeventmanager->CreateEvent("addicter_fly");

		if(event)
		{
			event->SetInt("entindex", entindex());
			event->SetInt("userid", pTarget->GetUserID());

			gameeventmanager->FireEvent(event);
		}
	}
}

void COverlordAddicter::FlyToTarget()
{
	if(!m_Target)
		return;

	const float minDist = 12.0f;

	Vector dir = GetTargetAttach() - GetEmitterPosition();

	VectorNormalize(dir);

	QAngle angle;
	VectorAngles(dir, angle);
	SetAbsAngles(angle);

	dir *= eo_addicter_speed.GetFloat() * (gpGlobals->curtime - GetLastRun());

	SetAbsOrigin(GetAbsOrigin() + dir);

	if(GetDistanceToTarget() <= minDist)
	{
		m_bFlying = false;
		StickToTarget();
	}
}

void COverlordAddicter::StickToTarget()
{
	if(!m_Target)
		return;

	StudioFrameAdvance();

	SetAbsOrigin(GetTargetAttach());
	AddEffects(EF_NOSHADOW);

	// Align the needle
	Vector dir = (m_Target->RealEyePosition() - GetAbsOrigin());
	VectorNormalize(dir);

	QAngle angle;
	VectorAngles(dir, angle);

	SetAbsAngles(angle);

	IGameEvent * event = gameeventmanager->CreateEvent("addicter_attach");

	if(event)
	{
		event->SetInt("entindex", entindex());
		event->SetInt("userid", m_Target->GetUserID());

		gameeventmanager->FireEvent(event);
	}
}

float COverlordAddicter::GetDistanceToTarget() const
{
	if(!m_Target)
		return -1.0f;

	return (const_cast<COverlordAddicter*>(this)->GetEmitterPosition() - GetTargetAttach()).Length();
}

Vector COverlordAddicter::GetTargetAttach() const
{
	if(!m_Target)
		return vec3_origin;

	Vector relPos(-18, 0, 60);
	Vector globPos;

	m_Target->EntityToWorldSpace(relPos, &globPos);

	return globPos;

}
int COverlordAddicter::OnTakeDamage(const CTakeDamageInfo & info)
{
	if(m_Target.Get() && info.GetAttacker() == m_Target.Get())
		return 0;

	if(info.GetAttacker() && !info.GetAttacker()->IsPlayer())
		return 0;

	return BaseClass::OnTakeDamage(info);

}
#endif

void COverlordAddicter::FireRandomTrace(trace_t * tr) const
{
	if(!tr)
		return;

	Vector vRandom(random->RandomFloat(-0.1f, 1.0f), 
			random->RandomFloat(-0.75f, 0.75f), 
			random->RandomFloat(-0.5f, 0.5f));
	VectorNormalize(vRandom);

	//vRandom.x = clamp(vRandom.x, 0.1, 1.0f);
	//VectorNormalize(vRandom);

	Vector vDir;
	Vector vDest;

	VectorRotate(vRandom, m_Target->EntityToWorldTransform(), vDir);
	VectorNormalize(vDir);
	VectorMA(m_Target->RealEyePosition(), MAX_TRACE_LENGTH, vDir, vDest);

	UTIL_TraceLine(m_Target->RealEyePosition(), vDest, MASK_SOLID, m_Target, COLLISION_GROUP_NONE, tr);
}

