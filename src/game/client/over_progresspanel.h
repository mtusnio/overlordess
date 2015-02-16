//==============Overlord mod=====================
//	Progress bar panel for door consoles etc.
//===============================================

#ifndef C_H_OV_PROGRESSBAR
#define C_H_OV_PROGRESSBAR

#include <vgui_controls/Panel.h>
#include <game/client/iviewport.h>
#include "over_progressbar.h"

class COverlordProgressPanel : public vgui::Panel, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordProgressPanel, vgui::Panel);

	COverlordProgressPanel(IViewPort * pViewPort);
	virtual ~COverlordProgressPanel();

	const char * GetName() { return PANEL_PROGRESSBAR; };

	virtual void OnThink();

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

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);

	virtual void SetProgress(float percent);

private:
	vgui::COverlordProgressBar * m_ProgressBar;

	float m_flLifeTime;
};


#endif