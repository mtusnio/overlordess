//==============Overlord mod=====================
//	Powerup panel base class
//===============================================

#ifndef C_H_OV_POWERUPPANEL
#define C_H_OV_POWERUPPANEL

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

namespace vgui
{
class COverlordProgressBar;
};

class COverlordPowerupPanel : public vgui::Frame, public IViewPortPanel
{
public:	
	DECLARE_CLASS(COverlordPowerupPanel, vgui::Frame);

	COverlordPowerupPanel(IViewPort * pViewPort);
	virtual ~COverlordPowerupPanel();

	virtual const char * GetName() { return PANEL_POWERUPPANEL; };

	virtual void InitializePowerups();
	virtual void ShowPanel(bool bShow);

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual void OnThink();

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	virtual void OnCommand(const char * command);
private:
	virtual void ApplyColors();

	bool m_bInited;
	void FindButtonPosition(int index, int & x, int & y);
	int GetScaledValue(int value) { return vgui::scheme()->GetProportionalScaledValueEx( GetScheme(), value); };
	CUtlVector<vgui::Button*> m_PowerupButtons;
	vgui::COverlordProgressBar * m_ProgressBar;
};


#endif