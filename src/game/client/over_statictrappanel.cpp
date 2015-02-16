//==============Overlord mod=====================
//	Static trap description popup
//===============================================

#include "cbase.h"
#include "over_statictrappanel.h"
#include "overlord_consolemanager.h"
#include "c_overlord_basemodule.h"
#include "view_scene.h"
#include "vgui_controls/Label.h"
#include "hud.h"

#define PANEL_WIDTH 80
#define PANEL_HEIGHT 32

COverlordStaticTrapPanel::COverlordStaticTrapPanel(IViewPort * pViewPort) :
vgui::Panel(NULL, PANEL_STATICTRAPPANEL )
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	SetZPos(1000);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetVisible(false);

	m_pModule = NULL;
	m_pName = NULL;
	m_pDuration = NULL;
	m_pCost = NULL;
	m_ProgressBar = NULL;
}

COverlordStaticTrapPanel::~COverlordStaticTrapPanel()
{
	if(m_pName)
		delete m_pName;

	if(m_pDuration)
		delete m_pDuration;

	if(m_pCost)
		delete m_pCost;

	if(m_ProgressBar)
		delete m_ProgressBar;
}

void COverlordStaticTrapPanel::ApplySchemeSettings(vgui::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), PANEL_HEIGHT));
	SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), PANEL_WIDTH));
	SetPaintBackgroundType(2);

	if(!m_pName)
	{
		m_pName = new vgui::Label(this, "TrapName", "");

		m_pName->SetWide(GetWide());
		m_pName->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9));
		m_pName->SetContentAlignment(vgui::Label::a_center);
		m_pName->SetPos(GetWide()/2 - m_pName->GetWide()/2, vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 2));
	}

	if(!m_pCost)
	{
		m_pCost = new vgui::Label(this, "TrapCost", "");

		m_pCost->SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 40));
		m_pCost->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9));

		m_pCost->SetContentAlignment(vgui::Label::a_west);
		m_pCost->SetPos(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 2), 
			vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), PANEL_HEIGHT - 10));

	}

	if(!m_pDuration)
	{
		m_pDuration = new vgui::Label(this, "TrapDuration", "");

		m_pDuration->SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 30));
		m_pDuration->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9));

		m_pDuration->SetContentAlignment(vgui::Label::a_east);
		m_pDuration->SetPos(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), PANEL_WIDTH) - m_pDuration->GetWide(), 
			vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), PANEL_HEIGHT - 10));
	}

	if(!m_ProgressBar)
	{
		m_ProgressBar = new vgui::COverlordProgressBar(this, "", vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 1));

		int height = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 8);
		int xpos = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 1);
		m_ProgressBar->SetPos(xpos , GetTall()/2 - height/2);
		m_ProgressBar->SetTall(height);
		m_ProgressBar->SetWide(GetWide() - 2*xpos);
		m_ProgressBar->SetEmptyColor(Color(0, 0, 0, 160));
		m_ProgressBar->SetOutlineColor(Color(0, 0, 0, 255));
	}
}

void COverlordStaticTrapPanel::ShowPanel(bool state)
{
	if(state == IsVisible())
		return;

	if(state)
	{
		UpdateLabels();

		UpdatePosition();
	}
	else
		m_pModule = NULL;

	SetVisible(state);
}

void COverlordStaticTrapPanel::OnThink()
{
	if(m_pModule != g_ConsoleManager.GetHoverStaticTrap())
	{
		UpdateLabels();
	}

	UpdatePosition();

	if(m_pModule)
	{
		float percent = 1.0f;

		if(m_pModule->IsCoolingDown())
		{
			percent = m_pModule->GetRemainingCooldown()/m_pModule->GetCooldownLength();
		}
		else if(m_pModule->IsActivated())
		{
			percent = (m_pModule->GetDeactivationTime() - gpGlobals->curtime)/m_pModule->GetDuration();
		}
		m_ProgressBar->SetPercentage(percent);
		m_ProgressBar->SetFullColor(m_pModule->GetIndicatorColor());
	}
}

void COverlordStaticTrapPanel::UpdateLabels()
{
	m_pModule = g_ConsoleManager.GetHoverStaticTrap();

	if(!m_pModule)
	{
		Warning("No module in show panel!\n");
		SetVisible(false);
		return;
	}

	if(m_pName)
		m_pName->SetText(m_pModule->GetModuleName());

	char text[32];
	if(m_pCost)
	{
		Q_snprintf(text, ARRAYSIZE(text), "Cost: %i", m_pModule->GetModuleCost());
		m_pCost->SetText(text);
	}

	if(m_pDuration)
	{
		Q_snprintf(text, ARRAYSIZE(text), "Dur: %i", (int)m_pModule->GetDuration());
		m_pDuration->SetText(text);
	}
}
void COverlordStaticTrapPanel::UpdatePosition()
{
	if(!m_pModule)
	{
		Warning("No module in static trap panel!\n");
		ShowPanel(false);
		return;
	}

	Vector screen;
	ScreenTransform(m_pModule->WorldSpaceCenter(), screen);

	const CViewSetup * setup = view->GetViewSetup();

	if(!setup)
	{
		Warning("No view setup in static trap panel!\n");
		ShowPanel(false);
		return;
	}

	int xCenter = setup->width / 2;
	int yCenter = setup->height / 2;

	int x = xCenter + (xCenter * screen.x);
	int y = yCenter - (yCenter * screen.y) - GetTall()/2;

	SetPos(x, y);
}