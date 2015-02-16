//==============Overlord mod=====================
// Hint events we use to display hints
// Add the function pointer to "CreateEvents"
// in the manager
//===============================================

#include "cbase.h"
#include "overlord_hintevents.h"
#include "hl2mp_gamerules.h"
#include "overlord_consolemanager.h"
#include "c_overlord_remotesentry.h"
#include "c_team.h"

bool AccessedConsole()
{
	GET_OVERLORD_DATA_RETURN(pData, false);

	return pData->IsInVehicle();
}

bool AccessedSentryGun()
{
	GET_OVERLORD_DATA_RETURN(pData, false);

	return (pData->IsInVehicle() && dynamic_cast<C_OverlordSentry*>(pData->GetCamera()));
}

bool BuildMenuOpen()
{
	if(g_ConsoleManager.IsInBuildMode() && !g_ConsoleManager.GetGhost())
		return true;

	return false;
}

bool BuildingTrap()
{
	if(g_ConsoleManager.IsInBuildMode() && g_ConsoleManager.GetGhost())
		return true;

	return false;
}

bool PlayerKilled()
{
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		C_BasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer)
			continue;
	
		if(pPlayer->IsOverlord())
			continue;

		if(pPlayer->IsRebel() && !pPlayer->IsAlive())
			return true;

		if(pPlayer->GetTeamNumber() == TEAM_SPECTATOR && pPlayer->GetDeathTime() != 0.0f)
			return true;

	}
	return false;
}