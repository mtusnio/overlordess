//==============Overlord mod=====================
//	Shows the amount of spawns left
//===============================================

#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "hl2mp_gamerules.h"

using namespace vgui;

#include <vgui/ILocalize.h>

#include "hudelement.h"
#include "hud_numericdisplay.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class COverlordSpawns : public CHudElement, public CHudNumericDisplay
{
public:
	DECLARE_CLASS_SIMPLE(COverlordSpawns, CHudNumericDisplay);

	COverlordSpawns(const char *pElementName);
	virtual ~COverlordSpawns() { };

	virtual void Init();
	virtual void Reset();

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);

	virtual void OnThink();
private:
	int m_iSpawns;
};

DECLARE_HUDELEMENT( COverlordSpawns );

COverlordSpawns::COverlordSpawns(const char *pElementName) :
 CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudSpawns")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_REBEL | HIDEHUD_NEEDSUIT );
	m_iSpawns = 0;
 }

 void COverlordSpawns::Init()
{
	Reset();
}

void COverlordSpawns::Reset()
{
	if(GET_OVERLORD_DATA)
		m_iSpawns = GET_OVERLORD_DATA->GetSpawnsAmount();

	wchar_t * displaystring = g_pVGuiLocalize->Find("#EO_SPAWNS");
	
	if(displaystring)
	{
		SetLabelText(displaystring);
	}
	else
	{
		SetLabelText(L"SPAWNS");
	}
	SetDisplayValue(m_iSpawns);
}

void COverlordSpawns::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hTextFont = pScheme->GetFont("DefaultVerySmall");
	m_hNumberFont = m_hSmallNumberFont;
}

void COverlordSpawns::OnThink()
{
	if(!GET_OVERLORD_DATA || m_iSpawns == GET_OVERLORD_DATA->GetSpawnsAmount())
		return;

	m_iSpawns = GET_OVERLORD_DATA->GetSpawnsAmount();

	SetDisplayValue(m_iSpawns);
}