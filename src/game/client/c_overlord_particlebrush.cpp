//==============Overlord mod=====================
//	Brush which emits particles inside of it
//===============================================

#include "cbase.h"
#include "c_overlord_particlebrush.h"

IMPLEMENT_CLIENTCLASS_DT( C_OverlordParticleBrush, DT_OverlordParticleBrush, COverlordParticleBrush )

	RecvPropInt(RECVINFO(m_spawnflags)),
	RecvPropString(RECVINFO(m_ParticleSystem)),

	RecvPropFloat(RECVINFO(m_minTime)),
	RecvPropFloat(RECVINFO(m_maxTime)),

END_RECV_TABLE()

C_OverlordParticleBrush::C_OverlordParticleBrush()
{
	// Init this here just in case...
	Q_strncpy(m_ParticleSystem, "\0", ARRAYSIZE(m_ParticleSystem));

	m_bInitialised = false;
}

C_OverlordParticleBrush::~C_OverlordParticleBrush()
{
}

void C_OverlordParticleBrush::Spawn()
{
	BaseClass::Spawn();

	SetThink(&C_OverlordParticleBrush::ClientThink);
	SetNextClientThink(CLIENT_THINK_NEVER);
}

// HACK: We cannot have this in Spawn function as the data is sent with a small delay
void C_OverlordParticleBrush::PostDataUpdate( DataUpdateType_t updateType )
{
	// Just check it once to initialise...
	if(!m_bInitialised)
	{
		m_bInitialised = true;

		if(m_spawnflags & SF_CLIENTSIDE)
		{
			SetNextClientThink(gpGlobals->curtime + random->RandomFloat(m_minTime, m_maxTime));
		}
		else
			SetNextClientThink(CLIENT_THINK_NEVER);

		if(m_ParticleSystem[0] == '\0')
		{
			SetNextClientThink(CLIENT_THINK_NEVER);
			Warning("CLIENT: Cannot init particle system for the brush\n");
		}
	}

	BaseClass::PostDataUpdate(updateType);
}

void C_OverlordParticleBrush::ClientThink()
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
			DispatchParticleEffect(m_ParticleSystem, vFinal, GetAbsAngles());
			break;
		}
	}

	SetNextClientThink(gpGlobals->curtime + random->RandomFloat(m_minTime, m_maxTime));
}