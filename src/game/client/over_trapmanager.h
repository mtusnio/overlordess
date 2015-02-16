//==============Overlord mod=====================
//	Trap manager menu
//===============================================

#ifndef C_H_OV_TRAPMANAGER
#define C_H_OV_TRAPMANAGER

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include "GameEventListener.h"

class TrapRecord;
class C_OverlordTrap;

class COverlordTrapManager : public vgui::Frame, public IViewPortPanel, public CGameEventListener
{
public:
	DECLARE_CLASS(COverlordTrapManager, vgui::Frame);

	COverlordTrapManager(IViewPort * pViewPort);
	virtual ~COverlordTrapManager();

	virtual const char * GetName() { return PANEL_TRAPMANAGER; };

	virtual void OnThink();

	virtual void FireGameEvent(IGameEvent * event);

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

	C_OverlordTrap * GetTrapHover();
	bool		 IsShiftPressed() const;

	virtual void OnCommand(const char * command);

private:
	void AddTrapRecord(C_OverlordTrap * pTrap);
	void RemoveTrapRecord(C_BaseEntity * pTrap);
	void HandleDimensions();
	void UpdateDisplay();

	CUtlVector<TrapRecord*> m_TrapRecord;

	float m_flLockTime;
	
	// Amount to which the menu is scaled
	int m_iScaled;

	// Last index in the first row
	int m_LastIndex;
};

#endif
