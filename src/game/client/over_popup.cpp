//==============Overlord mod=====================
//	Popup panel with trap descriptions	
//
//===============================================

#include "cbase.h"
#include "over_cameramenu.h"
#include "over_popup.h"
#include "vgui/IInput.h"

#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
//Popup support first

#include "tier0/vprof.h"

#define LABEL_MARGIN 11
#define DURATION_WIDE 55
#define DURATION_TALL 8
#define COST_WIDE 44
#define COST_TALL 8

void COverlordPopupSupport::OnCursorEntered()
{
	if(m_Description.GetDuration() <= 0 && 
		m_Description.GetCost() <= 0 &&
		!Q_stricmp(m_Description.GetDescription(), "\0"))
		return;

	COverlordPopup * iPanel = static_cast<COverlordPopup*>(g_pClientMode->GetViewport()->FindChildByName(PANEL_POPUP, true));
	
	if(iPanel)
	{
		iPanel->PassTrapDescription(m_Description);
	}

	gViewPortInterface->ShowPanel(PANEL_POPUP, true);
}
void COverlordPopupSupport::OnCursorExited()
{
	gViewPortInterface->ShowPanel(PANEL_POPUP, false);
}


COverlordPopup::COverlordPopup(IViewPort * pViewPort) :
vgui::EditablePanel(NULL, PANEL_POPUP )
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	MakePopup();

	SetZPos(1000);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetProportional(true);

	LoadControlSettings("Resource/UI/PopUpMenu.res");
	InvalidateLayout();

	// Setup the description etc.
	m_RichText = new vgui::RichText(this, "");
	m_DurLabel = new vgui::Label(this, "", "");
	m_CostLabel = new vgui::Label(this, "", "");

	SetDescription("Random description");
	SetCost(0);
	SetDuration(0);

	
}

COverlordPopup::~COverlordPopup()
{
	if(m_RichText)
		delete m_RichText;

	if(m_DurLabel)
		delete m_DurLabel;

	if(m_CostLabel)
		delete m_CostLabel;
}

void COverlordPopup::OnThink()
{
	VPROF_BUDGET("COverlordPopup::OnThink", VPROF_BUDGETGROUP_OTHER_VGUI);

	int x, y;
	vgui::input()->GetCursorPos(x, y);
	
	int xPos = x + 3;
	int yPos = y - GetTall() - 2;
	// Always keep in front
	MoveToFront();

	const CViewSetup * setup = view->GetViewSetup();

	// make sure it's in our view
	if(setup)
	{
		if(setup->width < (xPos + GetWide()))
		{
			xPos = xPos - GetWide();
		}

		if(yPos <= 0)
		{
			yPos = yPos + GetTall() + 4;
		}
	}

	SetPos(xPos, yPos);

}

void COverlordPopup::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	const int margin = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), LABEL_MARGIN);
	BaseClass::ApplySchemeSettings(pScheme);

	//SetBorder(pScheme->GetBorder("BaseBorder"));


	// Align everything
	m_DurLabel->SetProportional(true);
	m_DurLabel->SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), DURATION_WIDE));
	m_DurLabel->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), DURATION_TALL));
	m_DurLabel->SetPos(margin, GetTall() - margin);

	m_CostLabel->SetProportional(true);
	m_CostLabel->SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), COST_WIDE));
	m_CostLabel->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), COST_TALL));
	m_CostLabel->SetPos(GetWide() - margin - margin * 2, GetTall() - margin);

	m_RichText->SetProportional(true);
	m_RichText->SetPos(margin, margin);
	m_RichText->SetWide(GetWide() - margin * 2);
	m_RichText->SetTall(GetTall() - margin - m_CostLabel->GetTall());
	m_RichText->SetVerticalScrollbar(false);

	SetPaintBackgroundType(2);

	m_InitWide = GetWide();
	m_InitTall = GetTall();
}

void COverlordPopup::ShowPanel(bool bShow)
{
	if (IsVisible() == bShow)
		return;

	SetVisible(bShow);	

	// Resolve the coordinates
	bool bTextFound = false;
	if(m_RichText)
	{
		char text[1024];	
		m_RichText->GetText(0, text, sizeof(text));

		if(Q_strlen(text))
			bTextFound = true;
	}

	
	if(bTextFound)
	{
		const int margin = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), LABEL_MARGIN);
		SetTall(m_InitTall);
		// Align everything (again)
		m_DurLabel->SetPos(margin, GetTall() - margin);
		m_CostLabel->SetPos(GetWide() - margin - margin * 2, GetTall() - margin);

		m_RichText->SetVisible(true);
		m_RichText->SetPos(margin, margin);
		m_RichText->SetWide(GetWide() - margin * 2);
		m_RichText->SetTall(GetTall() - margin - m_CostLabel->GetTall());
	}
	else
	{
		const int margin = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), LABEL_MARGIN);
		SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), COST_TALL) + margin);
		m_DurLabel->SetPos(margin, GetTall() - margin);
		m_CostLabel->SetPos(GetWide() - margin - margin * 2, GetTall() - margin);

		m_RichText->SetVisible(false);
	}
}