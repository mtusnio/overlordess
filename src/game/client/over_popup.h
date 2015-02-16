//==============Overlord mod=====================
//	Popup panel with trap descriptions	
//
//===============================================

#ifndef C_H_OV_POPUP
#define C_H_OV_POPUP

#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>
#include "over_popupsupport.h"

class COverlordPopup : public vgui::EditablePanel, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordPopup, vgui::Panel);

	COverlordPopup(IViewPort * pViewPort);
	virtual ~COverlordPopup();

	const char * GetName() { return PANEL_POPUP; };

	virtual void OnThink();

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void ShowPanel(bool state);

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	inline void PassTrapDescription(sTrapDescription desc) { SetDescription(desc.GetDescription()); SetCost(desc.GetCost()); SetDuration(desc.GetDuration()); };
private:
	// Interface for description, cost and duration
	void SetDescription(const char * description) { m_RichText->SetText(description); };
	void SetCost(int cost) 
	{ 
		char costch[64]; 
		Q_snprintf(costch, ARRAYSIZE(costch), "Cost: %i", cost); 
		m_CostLabel->SetText(costch);
		if(cost > 0)
		{
			m_CostLabel->SetVisible(true);
		}
		else
		{
			m_CostLabel->SetVisible(false);
		}
	};
	void SetDuration(int duration) 
	{ 
		char dur[64]; 
		Q_snprintf(dur, ARRAYSIZE(dur), "Duration: %i", duration); 
		m_DurLabel->SetText(dur); 

		if(duration > 0)
		{
			m_DurLabel->SetVisible(true);
		}
		else
		{
			m_DurLabel->SetVisible(false);
		}
	};

	// labels etc. here
	vgui::Label * m_CostLabel;
	vgui::Label * m_DurLabel;
	vgui::RichText * m_RichText;

	int m_InitWide;
	int m_InitTall;
};


#endif