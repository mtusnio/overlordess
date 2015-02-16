//==============Overlord mod=====================
//	Base class for class weapons
//	
//===============================================

#include "cbase.h"
#if 0
#include "overlord_baseclass.h"

#ifdef CLIENT_DLL
#include "hud.h"
#include "over_hint.h"
#endif


#define RELOAD_DELAY 0.3f
#define SECONDARY_DELAY 0.2f
#define SECONDARY_MODESWITCH 1.0f
#define COUNT_NUMBER (SECONDARY_MODESWITCH / SECONDARY_DELAY)

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordClassWeap, DT_OverlordClassWeap)

BEGIN_NETWORK_TABLE( COverlordClassWeap, DT_OverlordClassWeap)

END_NETWORK_TABLE()


BEGIN_PREDICTION_DATA( COverlordClassWeap)
#ifdef CLIENT_DLL
	//DEFINE_PRED_FIELD(m_chMode, FIELD_STRING, FTYPEDESC_INSENDTABLE), 
	//DEFINE_PRED_FIELD(m_chTarget, FIELD_STRING, FTYPEDESC_INSENDTABLE), 
#endif
END_PREDICTION_DATA()


COverlordClassWeap::COverlordClassWeap()
{

}

COverlordClassWeap::~COverlordClassWeap()
{

}

void COverlordClassWeap::DefaultTouch(CBaseEntity * pOther)
{
	CBasePlayer * pPlayer = ToBasePlayer(pOther);
	if(!pPlayer)
		return;

	if(!(GetClassWeapon() & pPlayer->GetPlayerClass()))
		return;


	BaseClass::DefaultTouch(pOther);
}

// Maybe switching between modes should be done entirely client-side?
#if 0
// Ok, we got rid of this and changed the weapon display to hint display, no need
// to have this dirty function

void COverlordClassWeap::UpdateDisplay()
{
#ifdef CLIENT_DLL
	/*CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	
	if(!pPlayer)
		return;

	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();				 
	UserMessageBegin( user, "DisplayChanged");
	MessageEnd();*/

	//CHudElement * pElement = gHUD.FindElement("COverlordHint");

	//if(!pElement)
		//return;

	//COverlordHint * pDisp = GET_HUDELEMENT(COverlordHint);

	//if(!pDisp)
	//	return;

	//pDisp->UpdateDisplay();
#endif
}

// Call this at the end of derived class' function
void COverlordClassWeap::UpdateStrings()
{
//#ifdef CLIENT_DLL
	UpdateDisplay();
//#endif
}
#endif
#endif
