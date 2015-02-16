//==============Overlord mod=====================
//	Automatic turret
//	
//===============================================

#include "cbase.h"
#include "overlord_turret.h"
#include "overlord_barrel.h"
#include "hl2mp_gamerules.h"
#include "team.h"

#include "tier0/vprof.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TURRET_THINK m_flAlign
#define PLAYER_HULL 72
#define PLAYER_HULL_THRESHOLD (PLAYER_HULL - 8)
#define HULL_BOUNDS Vector(1, 1, 1)
#define ANGLE_TRESHOLD 0.1f

#define EYE_POSITION_UPPER 1
#define EYE_POSITION_LOWER -1

#define PITCH_AXIS (!MovesOnRelativePitch() ? CreateAxis(Vector(0, 1, 0)) : Vector(0, 1, 0))
#define YAW_AXIS   (!MovesOnRelativeYaw() ? CreateAxis(Vector(0, 0, 1)) : Vector(0, 0, 1))

ConVar eo_turret_range_tolerance("eo_turret_range_tolerance", "64.0", FCVAR_REPLICATED | FCVAR_CHEAT);

BEGIN_DATADESC(COverlordTurret)

	DEFINE_THINKFUNC(Think),

	DEFINE_KEYFIELD(m_iYaw, FIELD_FLOAT, "MaxYaw"), 
	DEFINE_KEYFIELD(m_iPitch, FIELD_FLOAT, "MaxPitch"), 
	DEFINE_KEYFIELD(m_flMoveSpeed, FIELD_FLOAT, "MoveSpeed"), 
	DEFINE_KEYFIELD(m_flDist, FIELD_FLOAT, "DistanceFromCamera"),

	DEFINE_KEYFIELD(m_iszBarrelName[0], FIELD_STRING, "BarrelName1"), 
	DEFINE_KEYFIELD(m_iszBarrelName[1], FIELD_STRING, "BarrelName2"), 
	DEFINE_KEYFIELD(m_iszBarrelName[2], FIELD_STRING, "BarrelName3"), 
	DEFINE_KEYFIELD(m_iszBarrelName[3], FIELD_STRING, "BarrelName4"), 
	DEFINE_KEYFIELD(m_iszBarrelName[4], FIELD_STRING, "BarrelName5"), 
	DEFINE_KEYFIELD(m_iszBarrelName[5], FIELD_STRING, "BarrelName6"), 
	DEFINE_KEYFIELD(m_iszBarrelName[6], FIELD_STRING, "BarrelName7"), 
	DEFINE_KEYFIELD(m_iszBarrelName[7], FIELD_STRING, "BarrelName8"),


	//DEFINE_KEYFIELD(m_iszModuleName, FIELD_STRING, "ModuleName"),
	DEFINE_KEYFIELD(m_flRadius, FIELD_FLOAT, "Radius"),
	DEFINE_KEYFIELD(m_flAlign, FIELD_FLOAT, "UpdateTime"),
	DEFINE_KEYFIELD(m_flTrackDelay, FIELD_FLOAT, "TrackingDelay"),

	//DECLARE_MODULE(),

END_DATADESC()

LINK_ENTITY_TO_CLASS(point_overturret, COverlordTurret)

COverlordTurret::COverlordTurret()
{
	m_LastMove = LAST_MOVE_LEFT;
	m_flShootDelay = 0.0f;
}

COverlordTurret::~COverlordTurret()
{

}

void COverlordTurret::Think()
{
	BaseClass::Think();

	if(IsActivated())
		RunModule();

	SetNextThink(gpGlobals->curtime + TURRET_THINK);
}

void COverlordTurret::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/editor/camera.mdl");
}

void COverlordTurret::Spawn()
{
	BaseClass::Spawn();

	Precache();

	SetSolid(SOLID_BBOX);

	AddSolidFlags( FSOLID_NOT_SOLID );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetModel("models/editor/camera.mdl");

	SetThink(&COverlordTurret::Think);

	SetNextThink(gpGlobals->curtime + TURRET_THINK);

}

void COverlordTurret::Activate()
{
	BaseClass::Activate();

	// We need this in order to construct the COverlordBaseTurret, which
	// does not inherit from CBaseEntity
	ResolveNames();

	m_aInit = GetAbsAngles();

	CreateBoundries();
	//if(COverlordBaseTurret::GetYawSpeed() < ANGLE_TRESHOLD)
		//SetMoveSpeed(ANGLE_TRESHOLD);
	
}

bool COverlordTurret::CanTarget(CBasePlayer * pTarget) const
{
	if(!pTarget)
		return false;

	if(!pTarget->IsPlayingRebel() || pTarget->IsInvisible())
		return false;
		
	// Don't target people that we can't even rotate to
	Vector dir = pTarget->RealEyePosition() - GetShootVector();
	QAngle senToPlayer;
	VectorAngles(dir, senToPlayer);
	if(!IsWithinBounds(senToPlayer))
		return false;

	// Can see him?
	trace_t tr;
	UTIL_TraceLine(GetShootVector(), GetHeadPosition(pTarget), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	// Check whether he's just not hiding behind breakables 
	if(tr.m_pEnt && ((tr.m_pEnt->m_takedamage == DAMAGE_YES) ||
		FClassnameIs(tr.m_pEnt, "prop_physics") ||
		FClassnameIs(tr.m_pEnt, "prop_physics_multiplayer") ||
		FClassnameIs(tr.m_pEnt, "prop_physics_respawnable")))
	{
		Vector startPos = tr.endpos;
		CBaseEntity * pIgnore = tr.m_pEnt;
		while(true)
		{
			trace_t trace;
			UTIL_TraceLine(startPos, GetHeadPosition(pTarget), MASK_SHOT, pIgnore, COLLISION_GROUP_NONE, &trace);
				
			if(trace.m_pEnt == pTarget)
				return true;

			if(!trace.startsolid || !trace.allsolid)
				break;

			// If it's a breakable - repeat
			if(trace.m_pEnt && !trace.m_pEnt->IsPlayer() &&
				((trace.m_pEnt->m_takedamage == DAMAGE_YES) ||
				FClassnameIs(trace.m_pEnt, "prop_physics") ||
				FClassnameIs(trace.m_pEnt, "prop_physics_multiplayer") ||
				FClassnameIs(trace.m_pEnt, "prop_physics_respawnable")))
			{
				startPos = trace.endpos;
				pIgnore = trace.m_pEnt;
				continue;
			}
			else
			{
				return false;
			}
		}
	}

	if(!(tr.fraction == 1.0f || tr.m_pEnt == pTarget))
		return false;

	return true;
}

bool COverlordTurret::ShouldFire(const trace_t & trace)
{
	//DevMsg("PRE: m_flShootDelay %f, curtime: %f\n", m_flShootDelay, gpGlobals->curtime);
	if(!GetTarget())
	{
		return false;
	}

	if(trace.m_pEnt == GetTarget())
	{
		if(m_flTrackDelay > 0.0f)
		{
			if(m_flShootDelay == 0.0f)
				m_flShootDelay = gpGlobals->curtime + m_flTrackDelay;

			if(m_flTrackDelay > 0.0f)
			{
				if(m_flShootDelay <= gpGlobals->curtime)
					return true;
				else
					return false;
			}
		}
		return true;
	}


	if(trace.m_pEnt && ((trace.m_pEnt->m_takedamage == DAMAGE_YES) || 
		FClassnameIs(trace.m_pEnt, "prop_physics") ||
		FClassnameIs(trace.m_pEnt, "prop_physics_multiplayer") ||
		FClassnameIs(trace.m_pEnt, "prop_physics_respawnable")))
	{
		// First try a trace to the player
		trace_t tr;
		UTIL_TraceLine(trace.startpos, GetHeadPosition(GetTarget()), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		// The player IS visible, so move instead of shooting
		if(tr.fraction >= 1.0f || tr.m_pEnt == GetTarget())
			return false;

		// Now calculate the angles
		Vector cameraDir;
		AngleVectors(GetAbsAngles(), &cameraDir);
		VectorNormalize(cameraDir);

		Vector traceDir = trace.endpos - trace.startpos;
		VectorNormalize(traceDir);

		float dot = DotProduct(cameraDir, traceDir);

		if(dot > 0.939f) // cos(20)
		{
			// Check the range difference!
			float diff = (GetShootVector()-trace.endpos).Length() -
				(GetShootVector()-GetHeadPosition(GetTarget())).Length();
				

			if(diff <= eo_turret_range_tolerance.GetFloat())
			{
				if(m_flTrackDelay > 0.0f && m_flShootDelay == 0.0f)
					m_flShootDelay = gpGlobals->curtime + m_flTrackDelay;

				if(m_flTrackDelay > 0.0f)
				{
					if(m_flShootDelay <= gpGlobals->curtime)
						return true;
					else
						return false;
				}
				
				return true;
			}

			return false;
		}

		return false;
	}

	if(m_flTrackDelay > 0.0f)
	{
		trace_t tr;
		UTIL_TraceLine(GetShootVector(), GetHeadPosition(GetTarget()), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if(!tr.startsolid && !tr.allsolid && tr.m_pEnt != GetTarget())
		{
			if(!tr.m_pEnt || tr.m_pEnt->m_takedamage != DAMAGE_YES)
				m_flShootDelay = 0.0f;
		}
	}

	return false;
}
void COverlordTurret::RunModule()
{
	CBasePlayer * pPlayer = NULL;

	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels || pRebels->GetNumPlayers() <= 0)
	{
		SetTarget(NULL);
		return;
	}

	// Check target settings here

	// Maximum distance
	float distance = m_flRadius;
	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pTarget = pRebels->GetPlayer(i);
		if(!pTarget)
			continue;

		if(!CanTarget(pTarget))
			continue;

		float tmpDist = GetDistance(pTarget);
		if(tmpDist < distance)
		{
			distance = tmpDist;
			pPlayer = pTarget;
		}

	}

	// We want to reset the target if it can't be targeted 
	/*if(!pPlayer || (GetTarget() && !CanTarget(pPlayer)))
	{
		SetTarget(NULL);
		return;
	}*/

	if(pPlayer != GetTarget())
		SetTarget(pPlayer);

	AlignTurret();
}

void COverlordTurret::AlignTurret()
{
	VPROF_BUDGET( "COverlordTurret::AlignTurret", VPROF_BUDGETGROUP_GAME );
	if(!GetTarget())
		return;

	const Vector camPos = GetShootVector();
	Vector camForw, vecDest;
	trace_t tr;

	
	AngleVectors(GetAbsAngles(), &camForw);

	VectorMA(camPos, m_flRadius/*MAX_TRACE_LENGTH (camPos - m_pTarget->GetAbsOrigin()).Length()*/, camForw, vecDest);

	trace_t trace;

	UTIL_TraceLine(camPos, vecDest, MASK_SHOT, this, COLLISION_GROUP_PLAYER, &trace);

	if(ShouldFire(trace))
		Fire();
	else //if (m_pTarget)
	{
		//UTIL_TraceLine(camPos, vecDest, MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr);
		// Target's position
		const Vector targPos = GetTarget()->GetAbsOrigin();
		
		// Relative positons in 2D system
		Vector relTarget = targPos - camPos;
		relTarget.z = 0.0f;

		Vector relTrace = vecDest - camPos;
		relTrace.z = 0.0f;

		// Now normalize the target vectors, so that we may use them safely
		VectorNormalize(relTrace);
		VectorNormalize(relTarget);

		// Angle needed to establish the X axis relativity
		QAngle angTarget;
		VectorAngles(relTarget, angTarget);
		
		// Rotate it so that it is placed relatively to the X axis
		Vector finalTrace;
		VectorRotate(relTrace, -angTarget, finalTrace);

		// As the X axis is our target, we can presume that anything above it needs to rotate
		// clockwise, and below it counterclockwise
		if(finalTrace.y > 0.0f)
		{
			RotateRight();
		}
		/*else*/ if(finalTrace.y < 0.0f)
		{
			RotateLeft();
		}
		// Only happens if we are pointing in the opposite direction, we eliminate the
		// shaky movement with this condition, as it will not rotate if above the target,
		// but unable to move down and point directly
		/*else if(finalTrace.x < 0.0f)
		{

			if(GetLastYawMove() == LAST_MOVE_LEFT)
			{
				RotateLeft();
			}
			else if(GetLastYawMove() == LAST_MOVE_RIGHT)
			{			
				RotateRight();
			}
		}*/

		//VectorMA(GetAbsOrigin(), (targsrc-vecsrc).Length(), vecforward, vecsrc);

			// This vector gives us position of the enemy if he was in front of the sentry,
			// in the same distance

		Vector relative;
		VectorMA(camPos, (camPos - GetTarget()->GetAbsOrigin()).Length(), camForw, relative);

		const float zdiff = relative.z - GetHeadPosition(GetTarget()).z;
			
		// The pitch should be changed to match the player's eye position, that's why
		// we use such tresholds
		if(zdiff >= EYE_POSITION_UPPER)
		{
			//DevMsg("Rel: %f Targ: %f Diff: %f Moves: Down\n", relative.z, targPos.z, zdiff);

			MoveDown();
		}
		/*else*/ if(zdiff <= EYE_POSITION_LOWER)
		{
			//DevMsg("Rel: %f Targ: %f Diff: %f Moves: Up\n", relative.z, targPos.z, zdiff);

			MoveUp();
		}
		/*else
		{
			DevMsg("Rel: %f Targ: %f Diff: %f\n", relative.z, targPos.z, zdiff);
		}*/

		if(HasSpawnFlags(SF_ALWAYS_FIRE))
			Fire();
		
	}

	//DevMsg("POST: m_flShootDelay %f, curtime: %f\n", m_flShootDelay, gpGlobals->curtime);

}

void COverlordTurret::Deactivate()
{
	if(!IsActivated())
		return;

	BaseClass::Deactivate();

	ResetTurret();
}

void COverlordTurret::ResolveNames()
{
	CBaseEntity * pEnt = NULL;
	int i;
	for(i=0; i < MAX_BARRELS; i++)
	{
		if(m_iszBarrelName[i])
		{
			pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszBarrelName[i]);

			if(!pEnt)
			{
				m_pBarrel[i] = NULL;
				m_iszBarrelName[i] = NULL;
				continue;
			}

			m_pBarrel[i] = dynamic_cast<COverlordBarrel*>(pEnt);

			if(!m_pBarrel[i])
			{
				m_pBarrel[i] = NULL;
				m_iszBarrelName[i] = NULL;
				continue;
			}
		}
		else
		{
			m_pBarrel[i] = NULL;
			continue;
		}
	}
}

void COverlordTurret::Fire()
{
	Vector vecforward;
	AngleVectors(GetAbsAngles(), &vecforward);

	VectorNormalize(vecforward);
	
	if(HasSpawnFlags(SF_CONCENTRATE_BARRELS) && GetTarget())
	{
		Vector location;
		VectorMA(GetShootVector(), (GetShootVector() - GetTarget()->GetAbsOrigin()).Length(), vecforward, location);
		for (int i = 0; i < MAX_BARRELS; i++)
		{
			if(m_pBarrel[i])
				m_pBarrel[i]->FireToLocation(location);
		}
	}
	else
	{
		for (int i = 0; i < MAX_BARRELS; i++)
		{
			if(m_pBarrel[i])
				m_pBarrel[i]->Fire(vecforward);
		}
	}
}


void COverlordTurret::CreateBoundries()
{
	// Maximum boundries first
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, GetMaxPitch()/2, PITCH_AXIS);
		RotateAngle(yaw, GetMaxYaw()/2, YAW_AXIS);

		m_aMaximum.x = (pitch.x > yaw.x) ? pitch.x : yaw.x;
		m_aMaximum.y = (pitch.y > yaw.y) ? pitch.y : yaw.y;
		m_aMaximum.z = (pitch.z > yaw.z) ? pitch.z : yaw.z;
	}
	// Minimum boundries
	{
		QAngle pitch = m_aInit;
		QAngle yaw = m_aInit;
		RotateAngle(pitch, GetMaxPitch()/2, -PITCH_AXIS);
		RotateAngle(yaw, GetMaxYaw()/2, -YAW_AXIS);

		m_aMinimum.x = (pitch.x < yaw.x) ? pitch.x : yaw.x;
		m_aMinimum.y = (pitch.y < yaw.y) ? pitch.y : yaw.y;
		m_aMinimum.z = (pitch.z < yaw.z) ? pitch.z : yaw.z;
	}
}

float COverlordTurret::GetYawSpeed() const
{
	const float defaultspeed = GetMoveSpeed();
	
	if(GetTarget())
	{
		QAngle targ;
		QAngle traceAng;

		GetTurretTargetAngles(traceAng, targ);

		float dist = targ.y - traceAng.y;

		if(dist < 0.0f)
			dist = -dist;

		if(dist > 180.0f)
			dist = 360.0f - dist;

		if(dist < ANGLE_TRESHOLD)
			return 0.0f;
		else if(dist <= defaultspeed)
			return dist;
		else
			return defaultspeed;
	}

	return defaultspeed;


}

float COverlordTurret::GetPitchSpeed() const
{
	const float defaultspeed = GetMoveSpeed();
	
	if(GetTarget())
	{
		QAngle targ;
		QAngle traceAng;
		GetTurretTargetAngles(targ, traceAng);

		float dist = targ.x - traceAng.x;

		if(dist < 0.0f)
			dist = -dist;

		if(dist > 180.0f)
			dist = 360.0f - dist;

		//DevMsg("Targ: %f TraceAng: %f Diff: %f\n", targ.x, traceAng.x, dist);

		if(dist < ANGLE_TRESHOLD)
			return 0.0f;
		else if(dist <= defaultspeed)
			return dist;
		else
			return defaultspeed;
	}

	return defaultspeed;


}

void COverlordTurret::RotateLeft()
{
	RotateTurret(BASETURRET_LEFT, GetYawSpeed(), YAW_AXIS);
	m_LastMove = LAST_MOVE_LEFT;
}

void COverlordTurret::RotateRight()
{
	RotateTurret(BASETURRET_RIGHT, -GetYawSpeed(), YAW_AXIS);
	m_LastMove = LAST_MOVE_RIGHT;
}	

void COverlordTurret::MoveUp()
{
	RotateTurret(BASETURRET_UP, -GetPitchSpeed(), PITCH_AXIS);
}

void COverlordTurret::MoveDown()
{
	RotateTurret(BASETURRET_DOWN, GetPitchSpeed(), PITCH_AXIS);
}

void COverlordTurret::RotateTurret(MoveType_t move, float speed, Vector axis)
{
	if(speed == 0)
		return;

	QAngle angle;
	
	RotateAngle(angle, speed, axis);

	if((move == BASETURRET_LEFT || move == BASETURRET_RIGHT) && (GetMaxYaw() >= 360 || GetMaxYaw() < 0))
		SetAbsAngles(angle);
	else if((move == BASETURRET_UP || move == BASETURRET_DOWN) && (GetMaxPitch() >= 360 || GetMaxPitch() < 0))
		SetAbsAngles(angle);
	else if(IsWithinBounds(angle))
		SetAbsAngles(angle);
}

void COverlordTurret::RotateAngle(QAngle & angle, float flAngle, const Vector & axis) const
{
	// Rotation axis
	Vector rotaxis = axis;

	///////////////////
	// Now the rotation
	///////////////////
	Quaternion q;
	
	// Rotate the quaternion and create a matrix from it
	AxisAngleQuaternion(rotaxis, flAngle, q);
	matrix3x4_t matrix;
	QuaternionMatrix(q, vec3_origin, matrix);

	// Local matrix
	matrix3x4_t localToWorld;

	// Make local or world transformation
	ConcatTransforms(EntityToWorldTransform(), matrix, localToWorld);

	// Local angles
	MatrixAngles(localToWorld, angle);
}
bool COverlordTurret::IsWithinBounds(const QAngle & angle) const
{
	float yawdist = UTIL_AngleDistance(angle.y, m_aInit.y);
	float pitchdist = UTIL_AngleDistance(angle.x, m_aInit.x);

	
	if(yawdist < 0)
		yawdist = -yawdist;
	if(pitchdist < 0)
		pitchdist = -pitchdist;

	//DevMsg("yaw: %f, pitch: %f\n", yawdist, pitchdist);
	if(yawdist > (float)(GetMaxYaw()/2))
		return false;
	
	if(pitchdist > (float)(GetMaxPitch()/2))
		return false;

	return true;

	
}

bool COverlordTurret::MovesOnRelativePitch() 
{ 
	return !HasSpawnFlags(SF_WORLD_PITCH); 
}

bool COverlordTurret::MovesOnRelativeYaw() 
{ 
	return !HasSpawnFlags(SF_WORLD_YAW); 
}



// Obsolete version, kept for future reference
#if 0
		// We need to normalize vectors, put them in a matrix
		// which x axis is based on the target's normalized vector,
		// and then check where the sentry is currently pointing
		UTIL_TraceLine(vecsrc, vecdest, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		// Vecsrc - where we are pointing
		// Targsrc - where we should point
		vecsrc = tr.endpos;
		const Vector targsrc = m_pTarget->GetAbsOrigin();

		// Source - our position (without z axis), used mostly to determine targ nad trace
		// Targ - position of the target, relatively to our position
		// Trace - place where the trace hit, relatively to our position
		Vector source(GetAbsOrigin().x, GetAbsOrigin().y, 0);
		Vector targ(m_pTarget->GetAbsOrigin().x - source.x , m_pTarget->GetAbsOrigin().y - source.y, 0);
		Vector trace(vecsrc.x - source.x, vecsrc.y - source.y, 0);
		
		// Vector normalizing is essential
		VectorNormalize(targ);
		VectorNormalize(trace);
		
		// Creates matrix with the turret's position, for this we use targ and trace vectors
		Vector zero(0, 0, 0), yaxis, zaxis;
		QAngle rightangle(0, 0, 90);
		QAngle rightanglez(0, 90, 0);
		VectorRotate(targ, rightangle, yaxis);
		VectorRotate(targ, rightanglez, zaxis);
		matrix3x4_t matrix(targ, yaxis, zaxis, zero);
		VectorITransform(trace, matrix, trace);

#endif