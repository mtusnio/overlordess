//==============Overlord mod=====================
//	Trap menu
//===============================================

#ifndef C_H_OV_TRAPMENU
#define C_H_OV_TRAPMENU

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include "overlord_dynamictraps.h"

class CModelPanel;

class COverlordTrapMenu : public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS(COverlordTrapMenu, vgui::Frame);

	COverlordTrapMenu(IViewPort * pViewPort);
	virtual ~COverlordTrapMenu();

	const char * GetName() { return PANEL_TRAPMENU; };

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

	virtual void OnCommand(const char *command);
	virtual void OnSizeChanged(int wide, int tall)
	{
		BaseClass::OnSizeChanged(wide, tall);

		MoveButtons();
	}
private:
	virtual void ClearCategories();
	virtual void CreateCategories();

	// Returns next y position to start at
	virtual int  BuildCategory(const char * name, int x, int y);

	virtual void BuildTrapButtons(vgui::IScheme *pScheme);
	virtual void MoveButtons();
	int			 GetButtonIndexByOrder(const char * category, int order);
	const char * GetButtonCategory(int i) const;
	void		 MovePanelToCursor();

	IViewPort * m_pViewPort;

	CUtlMap<string_t, CUtlVector<int>*> m_Categories;
	CUtlMap<const char *, vgui::Label*> m_Labels;
	CUtlVector<vgui::Divider *> m_Dividers;

	CModelPanel * m_Buttons[NUM_OF_TRAPS];
};

#endif