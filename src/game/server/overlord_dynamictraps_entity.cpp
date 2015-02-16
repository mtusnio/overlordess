//==============Overlord mod=====================
//	Dynamic traps map spawning entity
//===============================================
#include "cbase.h"
#include "hl2mp_gamerules.h"
#include "overlord_dynamictraps.h"
#include "GameEventListener.h"

#define NOT_INCLUDE_IN_LIST 2

class COverlordTrapEntity : public CBaseEntity, public CGameEventListener
{
public:
	DECLARE_CLASS(COverlordTrapEntity, CBaseEntity);
	DECLARE_DATADESC();

	COverlordTrapEntity();
	virtual ~COverlordTrapEntity();

	virtual void Spawn();
	virtual void Activate();

	virtual void Think();

	virtual void FireGameEvent( IGameEvent *event );
private:
	virtual void BuildTrap();

	float m_Pitch;
	float m_Yaw;
	float m_flSpawnDelay;
	int m_Count;
	const char * m_szTrapName;

	CHandle<COverlordTrap> m_Trap;
};

BEGIN_DATADESC(COverlordTrapEntity)

DEFINE_KEYFIELD(m_szTrapName, FIELD_STRING, "TrapName"),
DEFINE_KEYFIELD(m_Pitch, FIELD_FLOAT, "PitchRot"),
DEFINE_KEYFIELD(m_Yaw, FIELD_FLOAT, "YawRot"),
DEFINE_KEYFIELD(m_flSpawnDelay, FIELD_FLOAT, "RespawnDelay"),
DEFINE_KEYFIELD(m_Count, FIELD_INTEGER, "RespawnCount"),

END_DATADESC()

LINK_ENTITY_TO_CLASS(overlord_dynamictrap, COverlordTrapEntity);

COverlordTrapEntity::COverlordTrapEntity()
{
	
}

COverlordTrapEntity::~COverlordTrapEntity()
{
}

void COverlordTrapEntity::Spawn()
{
	BaseClass::Spawn();

	if(m_Count > 0)
	{
		ListenForGameEvent( "trap_decayed" );
		ListenForGameEvent( "trap_destroyed" );
	}
}

void COverlordTrapEntity::Activate()
{
	BaseClass::Activate();

	BuildTrap();

	SetNextThink(TICK_NEVER_THINK);
}

void COverlordTrapEntity::Think()
{
	if(m_Count <= 0)
	{
		Warning("In overlord_trap think with 0 count!\n");
		StopListeningForAllEvents();
		SetNextThink(TICK_NEVER_THINK);
		return;
	}

	BuildTrap();

	m_Count--;
	SetNextThink(TICK_NEVER_THINK);

	if(m_Count <= 0)
	{
		Warning("Negative count in overlord_dynamic think...\n");
		StopListeningForAllEvents();
	}
	
}

void COverlordTrapEntity::BuildTrap()
{
	if(!m_szTrapName)
		return;

	Vector dir;
	AngleVectors(GetAbsAngles(), &dir);
	VectorNormalize(dir);

	Vector pos = GetAbsOrigin() + dir * 4.0f;

	trace_t tr;
	UTIL_TraceLine(pos, pos - dir * MAX_TRACE_LENGTH, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	if(tr.fraction == 1.0f)
	{
		Warning("overlord_dynamictrap cannot be built\n");
		return;
	}

	if(GetOverlordData()->GetTrapsAmount() >= GetOverlordData()->GetMaxTraps())
		return;

	COverlordTrap * pEnt = dynamic_cast<COverlordTrap*>(CreateEntityByName(m_szTrapName));

	if(!pEnt)
	{
		Warning("couldn't create the trap\n");
		return;
	}

	pEnt->SetAbsOrigin(tr.endpos + tr.plane.normal * pEnt->GetNormalMultiplier());
	QAngle ang = pEnt->NormalToAngle(tr.plane.normal);
	ang[YAW] += m_Yaw;
	ang[PITCH] += m_Pitch;
	pEnt->SetAbsAngles(ang);

	pEnt->Spawn();

	m_Trap = pEnt;

	if(!HasSpawnFlags(NOT_INCLUDE_IN_LIST))
		GetOverlordData()->AddTrap(pEnt);
}

void COverlordTrapEntity::FireGameEvent(IGameEvent * event)
{
	if(event)
	{
		if(!Q_stricmp(event->GetName(), "trap_decayed"))
		{
			if(!m_Trap || m_Trap->entindex() == event->GetInt("entindex"))
			{
				SetNextThink(gpGlobals->curtime + m_flSpawnDelay);
			}
		}
		else if(!Q_stricmp(event->GetName(), "trap_destroyed"))
		{
			if(!m_Trap || m_Trap->entindex() == event->GetInt("entindex"))
			{
				SetNextThink(gpGlobals->curtime + m_flSpawnDelay);
			}
		}
	}
}