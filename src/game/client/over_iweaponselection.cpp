//==============Overlord mod=====================
// Interface for weapon selection
// panels
//===============================================

#include "cbase.h"
#include "over_iweaponselection.h"
#include "hud_element_helper.h"
//#include "weapon_selection.h"
#include "over_powerselection.h"

#define SEND_COMMAND(command) IWeaponSelection * pSelection = IWeaponSelection::GetCurrentSelection(); \
							  if(pSelection) \
						       pSelection->HandleCommand(#command) 

// Global function which grabs any weapon selection interface we are currently using
IWeaponSelection * IWeaponSelection::GetCurrentSelection()
{
	// Dynamic casting for now...
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(pPlayer->IsOverlord())
		return dynamic_cast<IWeaponSelection*>(gHUD.FindElement("COverlordPowerSelection"));
	else
		return dynamic_cast<IWeaponSelection*>(gHUD.FindElement("CHudWeaponSelection"));
}


//==================================================
//					Handle all slot commands
//==================================================
CON_COMMAND(slot0, "slot0")
{
	SEND_COMMAND(slot0);
}
CON_COMMAND(slot1, "slot1")
{
	SEND_COMMAND(slot1);
}
CON_COMMAND(slot2, "slot2")
{
	SEND_COMMAND(slot2);
}
CON_COMMAND(slot3, "slot3")
{
	SEND_COMMAND(slot3);
}
CON_COMMAND(slot4, "slot4")
{
	SEND_COMMAND(slot4);
}
CON_COMMAND(slot5, "slot5")
{
	SEND_COMMAND(slot5);
}
CON_COMMAND(slot6, "slot6")
{
	SEND_COMMAND(slot6);
}
CON_COMMAND(slot7, "slot7")
{
	SEND_COMMAND(slot7);
}
CON_COMMAND(slot8, "slot8")
{
	SEND_COMMAND(slot8);
}
CON_COMMAND(slot9, "slot9")
{
	SEND_COMMAND(slot9);
}
CON_COMMAND(cancelselect, "cancelselect")
{
	SEND_COMMAND(cancelselect);
}
CON_COMMAND(invnext, "invnext")
{
	SEND_COMMAND(invnext);
}
CON_COMMAND(invprev, "invprev")
{
	SEND_COMMAND(invprev);
}
CON_COMMAND(lastinv, "lastinv")
{
	SEND_COMMAND(lastinv);
}
