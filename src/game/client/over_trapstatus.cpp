//==============Overlord mod=====================
//	Trap status menu
//===============================================

#include "cbase.h"

#include "over_trapstatus.h"
#include "overlord_consolemanager.h"

#include "over_progressbar.h"
#include "vgui/IInput.h"
#include <vgui_controls/Label.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui/ILocalize.h>

using namespace vgui;

COverlordTrapStatus::COverlordTrapStatus(IViewPort * pViewPort) :
vgui::EditablePanel(NULL, PANEL_TRAPSTATUS )
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	//SetZPos(1000);

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_HealthBar = new vgui::COverlordProgressBar(this, "HealthBar");
	m_DurationBar = new vgui::COverlordProgressBar(this, "DurationBar");
	m_StatusLabel = new vgui::Label(this, "StatusLabel", "");
		
	SetProportional(true);
	LoadControlSettings("Resource/UI/TrapStatus.res");
	InvalidateLayout();
}

COverlordTrapStatus::~COverlordTrapStatus()
{

}

void COverlordTrapStatus::OnThink()
{
	if(!m_pTrap)
	{
		SetVisible(false);
		return;
	}

	if(m_HealthBar)
	{
		float percent = (float)m_pTrap->GetHealth()/(float)m_pTrap->GetMaxHealth();
		m_HealthBar->SetFullColor(GetHealthColor(percent));
		m_HealthBar->SetPercentage(percent);
	}
	if(m_DurationBar)
	{
		float percent = m_pTrap->GetRemainingLifetime()/m_pTrap->GetTrapDefaultLifetime();
		m_DurationBar->SetFullColor(GetDurationColor(percent));
		m_DurationBar->SetPercentage(percent);
	}
	if(m_StatusLabel)
	{
		char text[32];
		if(m_pTrap->IsInInitialDormant())
			Q_strncpy(text, "Waiting", ARRAYSIZE(text));
		else if(m_pTrap->IsTrapDormant())
			Q_strncpy(text, "Dormant", ARRAYSIZE(text));
		else if(!m_pTrap->IsTrapEnabled())
			Q_strncpy(text, "DISABLED", ARRAYSIZE(text));
		else
			Q_strncpy(text, "Active", ARRAYSIZE(text));
		m_StatusLabel->SetText(text);
	}
	

	// Move it to the cursor
	int x, y;
	input()->GetCursorPosition(x, y);

	SetPos(x + 3, y - 2 - GetTall());
}

void COverlordTrapStatus::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundType(2);
}

void COverlordTrapStatus::ShowPanel(bool bShow)
{
	if(IsVisible() == bShow)
		return;

	if(bShow)
	{
		SetVisible(true);
		C_OverlordTrap * pTrap = g_ConsoleManager.GetHoverTrap();

		if(!pTrap)
		{
			Warning("Panel status shown but no trap was found\n");
			SetVisible(false);
			return;
		}

		m_pTrap = pTrap;
	}
	else
	{
		SetVisible(false);
	}
}

Color COverlordTrapStatus::GetHealthColor(float fraction)
{
	fraction = clamp(fraction, 0.0f, 1.0f);
	const int ALPHA = 140;

	if(fraction == 0.5f)
	{
		return Color(255, 255, 0, ALPHA);
	}
	else if(fraction > 0.5f)
	{
		Color clr;

		float redfraction = (1.0f - fraction)/0.5f;

		clr.SetColor(redfraction * 255.0f, 255, 0, ALPHA);

		return clr;
	}
	else
	{
		Color clr;

		float greenfraction = fraction/0.5f;

		clr.SetColor(255, greenfraction * 255.0f, 0, ALPHA);

		return clr;
	}
}

Color COverlordTrapStatus::GetDurationColor(float fraction)
{
	fraction = clamp(fraction, 0.0f, 1.0f);
	return Color(0, 0, 225, 140);
}

void COverlordTrapStatus::GetDurationBarText(char * text, int size)
{
	if(!m_pTrap)
		return;

	if(m_pTrap->IsTrapDormant())
	{
		if(m_pTrap->IsInInitialDormant())
		{
			Q_strncpy(text, "Waiting...", size);
		}
		else
		{
			Q_strncpy(text, "Dormant", size);
		}
	}
	else
	{
		Q_strncpy(text, "Active", size);
	}
}
