//==============Overlord mod=====================
//	Class menu
//	
//===============================================

#ifndef C_H_OV_CLASSMENU
#define C_H_OV_CLASSMENU

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>

#include <game/client/iviewport.h>

class vgui::RichText;

class COverlordClassButton : public vgui::Button
{
public:
	DECLARE_CLASS_SIMPLE(COverlordClassButton, vgui::Button);

	COverlordClassButton(vgui::RichText * RichText, Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL);
	COverlordClassButton(vgui::RichText * RichText, Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL);
	virtual ~COverlordClassButton();

	virtual void SetRichPanel(vgui::RichText * richtext) { m_pRichText = richtext; };
	virtual void InitClassFile(const int classnum);

	virtual void SetRichText(const char * text) { Q_strncpy(m_pText, text, sizeof(m_pText)); };

	virtual void OnCursorEntered();
protected:
	char * m_pText;
	vgui::RichText * m_pRichText;
};

class COverlordClassMenu : public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordClassMenu, vgui::Frame);

	COverlordClassMenu(IViewPort * pViewPort);
	virtual ~COverlordClassMenu();

	const char * GetName() { return PANEL_CLASS; };

	virtual void OnThink();

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual void ShowPanel(bool bShow);

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void OnCommand(const char *command);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
private:
	void CreateControls();

	// Count labels
	vgui::Label * m_AssaultLabel;
	vgui::Label * m_PsionLabel;
	vgui::Label * m_HackerLabel;
	vgui::Label * m_StealtherLabel;


	IViewPort * m_pViewPort;
	vgui::RichText * m_pDesc;
	COverlordClassButton * m_pButton[NUM_OF_CLASSES];
};

#endif
