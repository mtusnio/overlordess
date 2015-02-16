//==============Overlord mod=====================
//	Powerup panel base class
//===============================================

#include "cbase.h"
#include "over_poweruppanel.h"
#include "overlord_powerups_shared.h"
#include "over_progressbar.h"
#include "overlord_data.h"
#include <vgui_controls/Button.h>

#define MAX_WIDTH 260

// Buton values
#define WIDTH_VALUE 80
#define HEIGHT_VALUE 10
#define SPACE_Y 5
#define SPACE_X 5
#define MARGIN_Y 15

COverlordPowerupPanel::COverlordPowerupPanel(IViewPort * pViewPort) :
BaseClass(NULL, PANEL_POWERUPPANEL )
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "ClientScheme"));

	SetTitle("", true);

	SetMoveable(true);
	SetProportional(true);
	SetSizeable(false);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);

	m_bInited = false;
}

COverlordPowerupPanel::~COverlordPowerupPanel()
{
	for(int i = 0; i < m_PowerupButtons.Count(); i++)
	{
		vgui::Button * button = m_PowerupButtons[i];

		if(!button)
			continue;

		delete button;
	}
}

void COverlordPowerupPanel::InitializePowerups()
{
	if(m_bInited)
		return;

	m_bInited = true;

	const int width = GetScaledValue(WIDTH_VALUE);
	const int height = GetScaledValue(HEIGHT_VALUE);
	const int spaceY = GetScaledValue(SPACE_Y);
	const int spaceX = GetScaledValue(SPACE_X);

	SetPos(0, 0);

	int panelHeight, panelWidth;
	FindButtonPosition(g_PowerupsList.Count()-1, panelWidth, panelHeight);
	panelHeight += height + spaceY;
	panelWidth += width + spaceX;
	SetTall(panelHeight);
	SetWide(panelWidth);

	m_ProgressBar = new vgui::COverlordProgressBar(this, "", 2);
	if(m_ProgressBar)
	{
		m_ProgressBar->SetTall(GetScaledValue(MARGIN_Y - 4));
		m_ProgressBar->SetWide(panelWidth - GetScaledValue(2 * SPACE_X));
		m_ProgressBar->SetPos(GetScaledValue(SPACE_X), GetScaledValue(MARGIN_Y) - m_ProgressBar->GetTall());
	}

	for(int i = 0; i < g_PowerupsList.Count(); i++)
	{
		COverlordPowerup * pPowerup = g_PowerupsList[i];
		
		if(!pPowerup)
		{
			g_PowerupsList.Remove(i);
			i--;
			continue;
		}

		vgui::Button * button = new vgui::Button(this, "", "");

		if(button)
		{		
			m_PowerupButtons.AddToTail(button);
			// Init the button
			
			int x, y;
			FindButtonPosition(i, x, y);

			button->SetPos(x, y);
			button->SetWide(width);
			button->SetTall(height);

			char command[32];
			Q_snprintf(command, ARRAYSIZE(command), "powerup %s start", pPowerup->GetName());
			button->SetCommand(command);
			button->SetText(pPowerup->GetDisplayName());
		}
	}

	SetPos(ScreenWidth()/2 - GetWide()/2, ScreenHeight()/2 - GetTall()/2);
}

void COverlordPowerupPanel::ShowPanel(bool bState)
{
	if(IsVisible() == bState)
		return;

	InitializePowerups();

	SetVisible(bState);
}

void COverlordPowerupPanel::OnThink()
{
	if(m_ProgressBar)
		m_ProgressBar->SetPercentage(GetOverlordData()->GetFerocityPercantage());

	ApplyColors();
}

void COverlordPowerupPanel::ApplySchemeSettings(vgui::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void COverlordPowerupPanel::OnCommand(const char * command)
{
	engine->ServerCmd(command);
	Close();
}

void COverlordPowerupPanel::FindButtonPosition(int index, int & x, int & y)
{
	const int width = GetScaledValue(WIDTH_VALUE);
	const int height = GetScaledValue(HEIGHT_VALUE);
	const int maxWidth = GetScaledValue(MAX_WIDTH);
	const int spaceX = GetScaledValue(SPACE_X);
	const int spaceY = GetScaledValue(SPACE_Y);
	const int marginY = GetScaledValue(MARGIN_Y);

	int maxPerLine = 0;
	int start = spaceX;
	while(start <= maxWidth)
	{
		start += spaceX + width;
		maxPerLine++;
	}


	int heightCounter = (spaceY + index * (spaceY + width)) / maxWidth;
	y = marginY + spaceY + heightCounter * ( spaceY + height );

	while(index >= maxPerLine)
		index -= maxPerLine;

	x = spaceX + index * ( spaceX + width);

}

void COverlordPowerupPanel::ApplyColors()
{
	const Color cooldown(0, 0, 255, 255);
	const Color ready(0, 255, 0, 255);
	const Color noferocity(255, 0, 0, 255);
	const Color activated(255, 215, 0, 255);

	for(int i = 0; i < m_PowerupButtons.Count(); i++)
	{
		// Indexes in the global list and on the buttons match
		COverlordPowerup * pPowerup = g_PowerupsList[i];
		vgui::Button * button = m_PowerupButtons[i];
		if(!pPowerup || !button)
		{
			continue;
		}

		if(pPowerup->IsOnCooldown())
			button->SetBgColor(cooldown);
		else if(pPowerup->IsActive())
			button->SetBgColor(activated);
		else if(pPowerup->GetCost() > GetOverlordData()->GetFerocity())
			button->SetBgColor(noferocity);
		else
			button->SetBgColor(ready);
		
	}
}