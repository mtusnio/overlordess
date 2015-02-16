//==============Overlord mod=====================
//	Progress bar panel for door consoles etc.
//===============================================

#include "cbase.h"
#include "over_progresspanel.h"
#include <game/client/iviewport.h>
#include "iviewrender.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include "hud.h"

#define WIDTH 420
#define HEIGHT 10

#define MARGIN_X 1
#define MARGIN_Y 1

#define OUTLINE_DIST 2

#define SHIFT_Y 20

using namespace vgui;

COverlordProgressPanel::COverlordProgressPanel(IViewPort * pViewPort) :
vgui::Panel(NULL, PANEL_PROGRESSBAR )
{
	m_flLifeTime = 0.0f;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme"));

	SetZPos(1000);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	m_ProgressBar = NULL;
}

COverlordProgressPanel::~COverlordProgressPanel()
{
	if(m_ProgressBar)
		delete m_ProgressBar;
}

void COverlordProgressPanel::OnThink()
{
	if(m_flLifeTime <= gpGlobals->curtime)
	{
		SetVisible(false);
		m_flLifeTime = 0.0f;
	}
}

void COverlordProgressPanel::ShowPanel(bool state)
{
	if(state)
	{
		m_flLifeTime = gpGlobals->curtime + 0.55f;

		if(!IsVisible())
		{
			SetVisible(true);
		}
	}
	else
	{
		if(IsVisible())
		{
			SetVisible(false);

			if(m_ProgressBar)
				m_ProgressBar->SetPercentage(0.0f);
		}
	}

}

void COverlordProgressPanel::ApplySchemeSettings(vgui::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), WIDTH));
	SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), HEIGHT));

	SetPos(ScreenWidth()/2 - GetWide()/2, 
		ScreenHeight()/2 + GetTall() + 
		vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), SHIFT_Y));

	if(!m_ProgressBar)
	{
		int marginX = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), MARGIN_X);
		int marginY = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), MARGIN_Y);

		m_ProgressBar = new vgui::COverlordProgressBar(this, "", vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), OUTLINE_DIST));
		m_ProgressBar->SetEmptyColor(Color(0, 0, 0, 120));
		m_ProgressBar->SetPos(marginX,
			marginY);
		m_ProgressBar->SetWide(GetWide() - 2 * marginX);
		m_ProgressBar->SetTall(GetTall() - 2 * marginY);
	}						  
}

void COverlordProgressPanel::SetProgress(float percent)
{ 
	if(!m_ProgressBar)
		return;

	if(percent > 1.0f)
		percent = 1.0f;
	if(percent < 0.0f)
		percent = 0.0f;

	Color full;
	if(percent < 1.0f)
	{
		full = Color(255, 0, 0, 120);
	}
	else
	{
		full = Color(0, 255, 0, 120);
	}

	m_ProgressBar->SetFullColor(full);
	m_ProgressBar->SetPercentage(percent);
}

void CC_ProgressBar( const CCommand& args )
{
	if(args.ArgC() < 2)
		return;


	float perc = (float)atof(args.Arg(1));

	COverlordProgressPanel * pPanel = static_cast<COverlordProgressPanel*>(gViewPortInterface->FindPanelByName(PANEL_PROGRESSBAR));

	if(pPanel)
		pPanel->SetProgress(perc);

	gViewPortInterface->ShowPanel(PANEL_PROGRESSBAR, true);
}
static ConCommand progress_bar("progress_bar", CC_ProgressBar);

void CC_HideProgressBar( const CCommand& args )
{
	gViewPortInterface->ShowPanel(PANEL_PROGRESSBAR, false);
}
static ConCommand hide_progress_bar("hide_progress_bar", CC_HideProgressBar);