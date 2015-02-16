//==============Overlord mod=====================
//	Brush which emits particles inside of it
//===============================================

#include "cbase.h"
#include "overlord_particlebrush.h"
#include "particle_parse.h"

LINK_ENTITY_TO_CLASS(func_over_particlebrush, COverlordParticleBrush);

BEGIN_DATADESC(COverlordParticleBrush)

	DEFINE_KEYFIELD(m_ParticleSystem, FIELD_STRING, "ParticleSystem"),

	DEFINE_KEYFIELD(m_minTime, FIELD_FLOAT, "MinimumTime"),
	DEFINE_KEYFIELD(m_maxTime, FIELD_FLOAT, "MaximumTime"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(COverlordParticleBrush, DT_OverlordParticleBrush)

	SendPropInt(SENDINFO(m_spawnflags)),
	SendPropStringT(SENDINFO(m_ParticleSystem)),

	SendPropFloat(SENDINFO(m_minTime)),
	SendPropFloat(SENDINFO(m_maxTime)),
END_SEND_TABLE()

COverlordParticleBrush::COverlordParticleBrush()
{
}

COverlordParticleBrush::~COverlordParticleBrush()
{

}

void COverlordParticleBrush::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(STRING(m_ParticleSystem.Get()));
}


void COverlordParticleBrush::Spawn()
{
	// Copied from COverlordArea
	BaseClass::Spawn();

	Precache();

	SetSolid(SOLID_BBOX);

	AddEffects(EF_NODRAW);

	AddSolidFlags( FSOLID_NOT_SOLID );

	AddSolidFlags( FSOLID_TRIGGER );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetModel(STRING(GetModelName()));

	SetThink(&COverlordParticleBrush::Think);

	if(HasSpawnFlags(SF_CLIENTSIDE))
		SetNextThink(TICK_NEVER_THINK);
	else
		SetNextThink(gpGlobals->curtime + random->RandomFloat(m_minTime, m_maxTime));

	// No particle system!
	if(m_ParticleSystem.Get() == NULL_STRING)
	{
		SetNextThink(TICK_NEVER_THINK);
		Warning("Cannot init particle system for the brush\n");
	}
}

void COverlordParticleBrush::Think()
{
	// Run it a couple of time to make sure we finally find something
	for(int numOfTests = 0; numOfTests <= 8; numOfTests++)
	{
		Vector vPercent = RandomVector(0.0f, 1.0f);

		Vector vPos = WorldAlignMins() + (WorldAlignMaxs() - WorldAlignMins()) * vPercent;
		Vector vFinal = vPos + GetAbsOrigin();
		//EntityToWorldSpace(vPos, &vFinal);
		if(!(enginetrace->GetPointContents_Collideable(GetCollideable(), vFinal) & CONTENTS_EMPTY))
		{
			DispatchParticleEffect(STRING(m_ParticleSystem.Get()), vFinal, GetAbsAngles());
			break;
		}
	}

	SetNextThink(gpGlobals->curtime + random->RandomFloat(m_minTime, m_maxTime));
}

// Send it if it's in PVS if it's supposed to be client-side
int COverlordParticleBrush::UpdateTransmitState( void )
{
	if(HasSpawnFlags(SF_CLIENTSIDE))
		return SetTransmitState( FL_EDICT_PVSCHECK );

	return BaseClass::UpdateTransmitState();
}