//==============Overlord mod=====================
// Overlordess class menu
//	
//===============================================

#ifndef C_H_OV_OVCLASSMENU
#define C_H_OV_OVCLASSMENU

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

#include <game/client/iviewport.h>

class COverlordOvClassMenu : public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordOvClassMenu, vgui::Frame);

	COverlordOvClassMenu(IViewPort * pViewPort);
	virtual ~COverlordOvClassMenu();

	virtual void OnThink();

	const char * GetName() { return PANEL_OVCLASS; };

	virtual void SetData(KeyValues *data) {};
	virtual void Reset();
	virtual void Update() { };

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }

	virtual void ShowPanel( bool state ); 

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	virtual void OnCommand(const char*command);
private:
	virtual void InitRichText(const char * chrichtext);

	bool m_bInited;
};

#endif
