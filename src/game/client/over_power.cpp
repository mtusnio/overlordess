//==============Overlord mod=====================
//	Power display
//	
//===============================================

#include "cbase.h"
#include "hl2mp_gamerules.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

using namespace vgui;

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "hudelement.h"
#include "hud_numericdisplay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class COverlordPower : public CHudElement, public CHudNumericDisplay
{
public:
	DECLARE_CLASS_SIMPLE(COverlordPower, CHudNumericDisplay);

	COverlordPower(const char *pElementName);
	virtual ~COverlordPower() { };

	virtual void Init();
	virtual void Reset();

	void MsgFunc_ResChanged(bf_read &msg);

	virtual void UpdatePower();
	virtual void UpdatePower(int power);

	virtual void Paint();
private:
	void CreateText2String();

	CPanelAnimationVarAliasType( float, text2_xpos, "text2_xpos", "16", "proportional_float" );
	CPanelAnimationVarAliasType( float, text2_ypos, "text2_ypos", "30", "proportional_float" );

	int m_iPower;
	char m_Text2[32];
};

DECLARE_HUDELEMENT( COverlordPower );
DECLARE_HUD_MESSAGE( COverlordPower, ResChanged );

COverlordPower::COverlordPower( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudPower")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_OVERLORD | HIDEHUD_NEEDSUIT );

	m_iPower = 0;
}

void COverlordPower::Init()
{
	HOOK_HUD_MESSAGE(COverlordPower, ResChanged);

	Reset();
}

void COverlordPower::Reset()
{
	GET_OVERLORD_DATA_ASSERT(pData);
	m_iPower = pData->GetPower();

	wchar_t * displaystring = g_pVGuiLocalize->Find("#EO_POWER");
	
	if(displaystring)
	{
		SetLabelText(displaystring);
	}
	else
	{
		SetLabelText(L"POWER");
	}

	SetDisplayValue(m_iPower);
}

void COverlordPower::MsgFunc_ResChanged(bf_read &msg)
{
	int power = msg.ReadShort();

	UpdatePower(power);
	CreateText2String();
}

void COverlordPower::UpdatePower()
{
	GET_OVERLORD_DATA_ASSERT(pData);

	int power = pData->GetPower();

	if(power == m_iPower)
		return;

	if(power < 0)
		power = 0;

	m_iPower = power;

	SetDisplayValue(m_iPower);
}

void COverlordPower::UpdatePower(int power)
{
	if(power == m_iPower)
		return;

	if(power < 0)
		power = 0;

	m_iPower = power;

	SetDisplayValue(m_iPower);
}

void COverlordPower::Paint()
{
	BaseClass::Paint();
		
	surface()->DrawSetTextColor(GetFgColor());
	surface()->DrawSetTextFont( m_hTextFont );
	
	wchar_t unicode[256];

	g_pVGuiLocalize->ConvertANSIToUnicode( m_Text2, unicode, sizeof( unicode ) );

	surface()->DrawSetTextPos( text2_xpos, text2_ypos);
	surface()->DrawUnicodeString( unicode );

}

void COverlordPower::CreateText2String()
{
	COverlordData * pData = GET_OVERLORD_DATA;

	if(!pData)
		return;

	Q_snprintf(m_Text2, ARRAYSIZE(m_Text2), "%i/%.1f s", pData->GetCurrentPowerGain(), pData->GetPowerUpdateTime(m_iPower));
}