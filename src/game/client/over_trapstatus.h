//==============Overlord mod=====================
//	Trap status menu
//===============================================

#ifndef C_H_OV_TRAPSTATUS

#include <vgui_controls/EditablePanel.h>
#include <game/client/iviewport.h>
#include "overlord_dynamictraps.h"

namespace vgui
{
class COverlordProgressBar;
}

class COverlordTrapStatus : public vgui::EditablePanel, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordTrapStatus, vgui::EditablePanel);

	COverlordTrapStatus(IViewPort * pViewPort);
	virtual ~COverlordTrapStatus();

	virtual const char * GetName() { return PANEL_TRAPSTATUS; };

	virtual void OnThink();

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual void ShowPanel(bool bShow);

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
private:
	Color GetHealthColor(float fraction);
	Color GetDurationColor(float fraction);
	void  GetDurationBarText(char * text, int size);

	vgui::COverlordProgressBar * m_HealthBar;
	vgui::COverlordProgressBar * m_DurationBar;
	vgui::Label * m_StatusLabel;

	CHandle<C_OverlordTrap> m_pTrap;
};


#endif