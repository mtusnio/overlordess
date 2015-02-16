//==============Overlord mod=====================
//	Sentrygun controlled by the overlord
//	
//===============================================

#include "cbase.h"
#include "overlord_remotesentry.h"
#include "overlord_basemodule.h"
#include "overlord_barrel.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"

#include "ilagcompensationmanager.h"

#include "tier0/vprof.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SENTRY_THINK 0.05f

BEGIN_DATADESC(COverlordSentry)

	DEFINE_THINKFUNC(Think),

	//DEFINE_TURRET(),

	//DEFINE_KEYFIELD(m_iszModuleName, FIELD_STRING, "Module's name"),

	DEFINE_BARRELS(),


END_DATADESC()

LINK_ENTITY_TO_CLASS(point_oversentry, COverlordSentry);

IMPLEMENT_SERVERCLASS_ST(COverlordSentry, DT_OverlordSentry)

	
END_SEND_TABLE()

COverlordSentry::COverlordSentry()
{
	
}

COverlordSentry::~COverlordSentry()
{

}

void COverlordSentry::Spawn()
{
	BaseClass::Spawn();

	SetThink(&COverlordSentry::Think);
	SetNextThink(gpGlobals->curtime + SENTRY_THINK);
}

void COverlordSentry::Activate()
{
	BaseClass::Activate();

	// We need this in order to construct the COverlordBaseTurret, which
	// does not inherit from CBaseEntity
	CREATE_TURRET();
}

void COverlordSentry::Think()
{
	if(GetOverlordData()->IsInVehicle())
	{
		// Used dynamic_cast, now changed in order to save
		// CPU cycles
		
		if(GetOverlordData()->GetCamera() == this )
		{
			CBasePlayer * pPlayer = GetOverlordData()->GetOverlord();

			if(pPlayer)
			{
				int button = pPlayer->m_nButtons;

				if(button & IN_ATTACK)
				{
					Fire();
				}

				if(button & IN_DUCK)
				{
					Fire();
				}
			}
		}
	}

	SetNextThink(gpGlobals->curtime + SENTRY_THINK);
}

void COverlordSentry::Fire()
{
	if(!HasSpawnFlags(SF_CONCENTRATE_BARRELS))
	{
		Vector vecforward;
		AngleVectors(GetAbsAngles(), &vecforward);

		VectorNormalize(vecforward);

		for (int i=0; i < MAX_BARRELS; i++)
		{
	
			if(!m_pBarrel[i] || !m_pBarrel[i]->CanFire())
				continue;
			m_pBarrel[i]->Fire(vecforward);
		}
	}
	else
	{
		Vector location;
		Vector forw;
		AngleVectors(EyeAngles(), &forw);

		VectorMA(EyePosition(), MAX_TRACE_LENGTH, forw, location);
		trace_t tr;
		UTIL_TraceLine(EyePosition(), location, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		location = tr.endpos;

		for (int i=0; i < MAX_BARRELS; i++)
		{
			if(!m_pBarrel[i] || !m_pBarrel[i]->CanFire())
				continue;

			m_pBarrel[i]->FireToLocation(location);
		}
	}
}











