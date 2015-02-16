//==============Overlord mod=====================
// Selection menu displaying selected player's
// statistics
//===============================================

#ifndef C_H_OV_SELECTION_MENU
#define C_H_OV_SELECTION_MENU

#include <vgui_controls/EditablePanel.h>

class CAvatarImagePanel;
class vgui::RichText;

class COverlordSelection : public vgui::EditablePanel, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordSelection, vgui::EditablePanel);

	COverlordSelection(IViewPort * pViewPort);
	virtual ~COverlordSelection();

	const char * GetName() { return PANEL_SELECTION; };

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void OnThink();
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
	virtual void UpdateLabels();
	virtual void BuildLabels(vgui::IScheme *pScheme);

	IViewPort * m_pViewPort;

	vgui::RichText * m_RichText;
	CAvatarImagePanel * m_Avatar;
}; 

#endif