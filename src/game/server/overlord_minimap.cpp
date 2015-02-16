//==============Overlord mod=====================
//	Overlord's interactive minimap
//	
//===============================================

#include "cbase.h"
#include "overlord_minimap.h"
#include "env_sphere.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define BLIP_SIZE 20
//#define PROPORTION 20.0f

#define FLAG_START_OFF 2

BEGIN_DATADESC(COverlordMinimap)

	DEFINE_KEYFIELD(m_iScale, FIELD_INTEGER, "Scale"),
	DEFINE_KEYFIELD(m_iSize, FIELD_INTEGER, "SphereSize"),
	DEFINE_KEYFIELD(m_flUpdate, FIELD_FLOAT, "Update"), 
	
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable), 
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable), 

	DEFINE_THINKFUNC(Think),

END_DATADESC()

LINK_ENTITY_TO_CLASS(overlord_minimap, COverlordMinimap);

void COverlordMinimap::Spawn()
{
	BaseClass::Spawn();

	SetThink(&COverlordMinimap::Think);

	if(!HasSpawnFlags(FLAG_START_OFF))
		SetNextThink(gpGlobals->curtime + m_flUpdate);
	else
		SetNextThink(TICK_NEVER_THINK);
}

void COverlordMinimap::Think()
{
	//BaseClass::Think();

	Vector vecdest, playerpos;

	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);
		if(!pPlayer)
			continue;

		if(pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
			continue;

		playerpos = pPlayer->GetAbsOrigin();
		vecdest = GetAbsOrigin() + (playerpos / m_iScale);
		//vecdest = GetAbsOrigin();
		MakeSphere(vecdest, m_iSize, pPlayer->IsOverlord(), m_flUpdate);
	}

	SetNextThink(gpGlobals->curtime + m_flUpdate);

}