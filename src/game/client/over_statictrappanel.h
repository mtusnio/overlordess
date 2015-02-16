//==============Overlord mod=====================
//	Static trap description popup
//===============================================

#ifndef C_H_OV_STATICTRAPPANEL
#define C_H_OV_STATICTRAPPANEL

#include <vgui_controls/Panel.h>
#include <game/client/iviewport.h>
#include "over_progressbar.h"

class C_OverlordBaseModule;

class COverlordStaticTrapPanel : public vgui::Panel, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordStaticTrapPanel, vgui::Panel);

	COverlordStaticTrapPanel(IViewPort * pViewPort);
	virtual ~COverlordStaticTrapPanel();

	const char * GetName() { return PANEL_STATICTRAPPANEL; };

	virtual void ShowPanel(bool state);

	virtual void OnThink();

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }


private:
	void UpdateLabels();
	void UpdatePosition();


	vgui::Label * m_pName;
	vgui::Label * m_pCost;
	vgui::Label * m_pDuration;
	vgui::COverlordProgressBar * m_ProgressBar;

	C_OverlordBaseModule * m_pModule;
};




#endif