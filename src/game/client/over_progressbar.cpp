//==============Overlord mod=====================
//	Progres bar class
//===============================================

#include "cbase.h"
#include "over_progressbar.h"



namespace vgui
{
DECLARE_BUILD_FACTORY(COverlordProgressBar);

COverlordProgressBar::COverlordProgressBar(vgui::Panel * parent, const char * name) :
vgui::Panel(parent, name)
{
	Init();

	m_IconFull = NULL;
	m_IconEmpty = NULL;
	m_Spacing = -1;
	m_BarType = CHud::HUDPB_HORIZONTAL;
}

COverlordProgressBar::COverlordProgressBar(vgui::Panel *parent, const char * name, int outlineSpace, int type) :
vgui::Panel(parent, name)
{
	Init();

	m_IconFull = NULL;
	m_IconEmpty = NULL;
	m_Spacing = outlineSpace;
	m_BarType = type;
}

COverlordProgressBar::COverlordProgressBar(vgui::Panel *parent, const char * name,
										   CHudTexture * iconFull, CHudTexture * iconEmpty, int type) :
vgui::Panel(parent, name)
{
	Init();

	m_BarType = type;
	m_IconFull = iconFull;
	m_IconEmpty = iconEmpty;
	m_Spacing = -1;
}

COverlordProgressBar::~COverlordProgressBar()
{
}

void COverlordProgressBar::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	if(!inResourceData)
		return;

	SetFullColor(inResourceData->GetColor("fullColor"));
	SetEmptyColor(inResourceData->GetColor("emptyColor"));
	SetOutlineColor(inResourceData->GetColor("outlineColor"));
	SetSpacing(inResourceData->GetInt("outlineSpace", 0));

	const char * type = inResourceData->GetString("barType", "horizontal");

	if(!Q_stricmp(type, "horizontal"))
	{
		SetType(CHud::HUDPB_HORIZONTAL);
	}
	else if(!Q_stricmp(type, "horizontalinv"))
	{
		SetType(CHud::HUDPB_HORIZONTAL_INV);
	}
	else if(!Q_stricmp(type, "vertical"))
	{
		SetType(CHud::HUDPB_VERTICAL);
	}

}
void COverlordProgressBar::Paint()
{
	if(m_IconFull && m_IconEmpty)
	{
		gHUD.DrawIconProgressBar(0, 0, m_IconFull, m_IconEmpty, m_flPercentage, m_FullColor, m_BarType); 
	}
	else if(m_Spacing >= 0)
	{
		gHUD.DrawProgressBarWithOutline(0, 0, vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), m_Spacing), 
			GetWide(), GetTall(), m_flPercentage, m_FullColor, m_EmptyColor, m_OutlineColor, m_BarType);
	}
}

void COverlordProgressBar::Init()
{
	m_flPercentage = 1.0f;
	m_FullColor = Color(255, 0, 0, 255);
	m_EmptyColor = Color(0, 0, 0, 255);
	m_OutlineColor = Color(255, 255, 255, 255);
}


}