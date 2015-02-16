//==============Overlord mod=====================
// Overlordess class menu
//	
//===============================================

#include "cbase.h"
#include "over_ovclassmenu.h"
#include <FileSystem.h>
#include <vgui_controls/RichText.h>

using namespace vgui;

#define TANK_DESC "tankdesc"
#define PHASING_DESC "phasingdesc"
#define TECH_DESC "techdesc"

COverlordOvClassMenu::COverlordOvClassMenu(IViewPort *pViewPort) : vgui::Frame(NULL, PANEL_OVCLASS )
{
	SetScheme("OverlordessScheme");

	SetTitle("", true);

	SetMoveable(false);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	SetProportional(true);

	SetSizeable(false);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);


	LoadControlSettings("resource/ui/OvClassMenu.res");
	InvalidateLayout();

	m_bInited = false;
}

COverlordOvClassMenu::~COverlordOvClassMenu()
{
}

void COverlordOvClassMenu::OnThink()
{
	BaseClass::OnThink();

	// A workaround for the "no cursor" bug when joining a server 
	// and closing the MotD
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

}

void COverlordOvClassMenu::Reset()
{
	// Load the text files
	
}

void COverlordOvClassMenu::ShowPanel(bool state)
{
	if (BaseClass::IsVisible() == state)
		return;

//	SetKeyBoardInputEnabled(false);

	if (state)
	{
		Activate();

		/*SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(true);
		SetVisible(true);*/
	}
	else
	{
		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(false);
		SetVisible(false);
	}
}

void COverlordOvClassMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	IBorder * border = pScheme->GetBorder("BaseBorder");

	Panel * panel = FindChildByName(TANK_DESC);

	if(panel)
		panel->SetBorder(border);

	panel = NULL;
	panel = FindChildByName(PHASING_DESC);

	if(panel)
		panel->SetBorder(border);

	panel = NULL;
	panel = FindChildByName(TECH_DESC);

	if(panel)
		panel->SetBorder(border);

	SetBorder(border);

	if(!m_bInited)
	{
		InitRichText(TANK_DESC);
		InitRichText(PHASING_DESC);
		InitRichText(TECH_DESC);
		m_bInited = true;
	}
}

void COverlordOvClassMenu::OnCommand(const char *command)
{
	engine->ServerCmd(VarArgs("eo_selectclass %s\n", command));

	Close();
}

void COverlordOvClassMenu::InitRichText(const char * chrichtext)
{
	const int SIZE = 1024;
	char path[32];

	RichText * richtext = static_cast<RichText*>(FindChildByName(chrichtext));

	if(!richtext)
		return;

	Q_snprintf(path, ARRAYSIZE(path), "resource/class_%s.txt", chrichtext);

	FileHandle_t f = g_pFullFileSystem->Open(path, "rb");
	if(f && richtext)
	{
		char text[SIZE];

		int size = min(g_pFullFileSystem->Size(f)+1, ARRAYSIZE(text));

		g_pFullFileSystem->Read(text, size, f);

		char buff[SIZE];
		Q_strncpy(buff, text, size);

		richtext->SetText(buff);
	}

	g_pFullFileSystem->Close(f);
}