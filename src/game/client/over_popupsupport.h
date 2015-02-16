//==============Overlord mod=====================
//	Popup support (button, structs etc.)
//	
//===============================================

#ifndef C_H_OV_POPUPSUPPORT
#define C_H_OV_POPUPSUPPORT

#include <vgui_controls/Button.h>
#include "iclientmode.h"

class COverlordPopup;

struct sTrapDescription
{
public:
	sTrapDescription() { Q_strncpy(m_Description, "\0", ARRAYSIZE(m_Description)); m_iCost = 0; m_iDuration = 0; };
	sTrapDescription(const char * description, int cost, int duration)
	{
		Q_strncpy(m_Description, description, ARRAYSIZE(m_Description));
		m_iCost = cost;
		m_iDuration = duration;
	}

	const char * GetDescription() const { return m_Description; };
	int			 GetCost() const { return m_iCost; };
	int			 GetDuration() const { return m_iDuration; };
private:
	char m_Description[512];
	int m_iCost;
	int m_iDuration;
};

// Support class for different kinds of buttons, images etc.
class COverlordPopupSupport
{
public:
	DECLARE_CLASS_NOBASE(COverlordPopupSupport);

	COverlordPopupSupport()
	{
	};

	virtual ~COverlordPopupSupport()
	{
	};

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	void SetDescription(sTrapDescription desc) { m_Description = desc; };

	sTrapDescription GetTrapDescription() const { return m_Description; };
private:
	sTrapDescription m_Description;
};

// General button implementation
class COverlordModuleButton : public vgui::Button, public COverlordPopupSupport
{
public:
	DECLARE_CLASS(COverlordModuleButton, vgui::Button);

	COverlordModuleButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL) 
		: vgui::Button(parent, panelName, text, pActionSignalTarget, pCmd) { };
	COverlordModuleButton(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL)
		: vgui::Button(parent, panelName, text, pActionSignalTarget, pCmd) { };
	virtual ~COverlordModuleButton()
	{
	}

	virtual void OnCursorEntered()
	{
		BaseClass::OnCursorEntered();

		COverlordPopupSupport::OnCursorEntered();
	};

	virtual void OnCursorExited()
	{
		BaseClass::OnCursorExited();

		COverlordPopupSupport::OnCursorExited();
	};


};

#endif