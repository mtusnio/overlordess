//==============Overlord mod=====================
// Selection menu displaying selected player's
// statistics
//===============================================


#include "cbase.h"
#include <game/client/iviewport.h>
#include "over_selection.h"
#include "vgui_avatarimage.h"
#include "overlord_consolemanager.h"
#include "c_hl2mp_player.h"
#include "vgui/IInput.h"

#include <vgui/ILocalize.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/Button.h>

#define X_AVATAR_POS ((GetWide()/2) - 15)
#define Y_AVATAR_POS 7

#define X_AVATAR_SIZE 32
#define Y_AVATAR_SIZE X_AVATAR_SIZE

#define X_RICHTEXT_POS 3
#define Y_RICHTEXT_POS 40

#define X_RICHTEXT_SIZE 152
#define Y_RICHTEXT_SIZE 82 

using namespace vgui;

COverlordSelection::COverlordSelection(IViewPort * pViewPort):
vgui::EditablePanel(NULL, PANEL_SELECTION )
{
	m_pViewPort = pViewPort;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);

	SetProportional(true);

	InvalidateLayout();

	m_Avatar = new CAvatarImagePanel(this, "avatar");
	m_RichText = new vgui::RichText(this, "stats");

	LoadControlSettings("Resource/UI/PlayerPopup.res");
}

COverlordSelection::~COverlordSelection()
{
	if(m_Avatar)
		delete m_Avatar;

	if(m_RichText)
		delete m_RichText;
}

void COverlordSelection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("BaseBorder"));

	BuildLabels(pScheme);

	SetPaintBackgroundType(2);
}

void COverlordSelection::OnThink()
{
	// No player disappear
	if(!g_ConsoleManager.IsPlayerSelected())
	{
		ShowPanel(false);
		return;
	}

	int x, y;
	input()->GetCursorPosition(x, y);

	SetPos(x + 3, y - GetTall() - 2);

	UpdateLabels();
}


void COverlordSelection::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		SetVisible(true);
		UpdateLabels();
	}
	else
		SetVisible(false);
}

void COverlordSelection::UpdateLabels()
{
	C_HL2MP_Player * pSelected = ToHL2MPPlayer(g_ConsoleManager.GetSelectedPlayer());
	
	if(!pSelected)
	{
		Warning("No player selected, can't build labels!\n");
		return;
	}

	if(m_Avatar)
	{
		m_Avatar->SetPlayer(pSelected);
	}
	else
	{
		Warning("No avatar is present!\n");
	}

	if(m_RichText)
	{
		// Creating all stats to display...

		// Buffer
		char buffer[128];

		// Create the string 
		Q_snprintf(buffer, ARRAYSIZE(buffer), 
			"Name: %s\nClass: %s\nHealth: %i\n", 
			pSelected->GetPlayerName(), pSelected->GetPlayerClassName(), pSelected->GetHealth());

		m_RichText->SetText(buffer);

		Q_snprintf(buffer, ARRAYSIZE(buffer), "PPK: %i", pSelected->GetPowerPerKill());
		m_RichText->InsertString(buffer);
	}
	else
	{
		Warning("No richtext!\n");
	}
}

void COverlordSelection::BuildLabels(vgui::IScheme *pScheme)
{
	if(m_Avatar)
	{	
		m_Avatar->SetBorder(pScheme->GetBorder("BaseBorder"));
		m_Avatar->SetTall(32);
		m_Avatar->SetWide(32);
	}

	if(m_RichText)
	{
		m_RichText->SetFont(pScheme->GetFont("DefaultVerySmall"));

		m_RichText->SetVerticalScrollbar(false);
	}
}