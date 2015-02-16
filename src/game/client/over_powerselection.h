//==============Overlord mod=====================
//	Ov power selection menu
//===============================================

#ifndef C_H_OV_POWERSELECTION
#define C_H_OV_POWERSELECTION

#include "hudelement.h"
#include "weapon_selection.h"
#include "hl2mp_gamerules.h"
#include <vgui_controls/Panel.h>
#include <game/client/iviewport.h>


#define MAX_POWERS 10

class vgui::ImagePanel;
class vgui::Label;
class vgui::Frame;

class C_BaseCombatWeapon;

class COverlordPowerSelection : public CBaseHudWeaponSelection, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(COverlordPowerSelection, vgui::Panel);

	COverlordPowerSelection( const char *pElementName );
	virtual ~COverlordPowerSelection();

	virtual bool ShouldDraw() { return false; };

	virtual void Init();
	virtual void OnThink();

	virtual void CycleToNextWeapon( void ) {  };
	virtual void CycleToPrevWeapon( void ) {  };
	virtual C_BaseCombatWeapon *GetWeaponInSlot( int iSlot, int iSlotPos ) { return GetWeaponInSlot(iSlot); };
	virtual void SelectWeaponSlot( int iSlot );
	virtual C_BaseCombatWeapon	*GetSelectedWeapon( void ) { return m_hSelectedWeapon; };

	C_BaseCombatWeapon * GetWeaponInSlot(int iSlot) const;
	C_BaseCombatWeapon * GetCurrentWeapon() const;
	C_BaseCombatWeapon * GetNextWeapon() const;
	C_BaseCombatWeapon * GetPreviousWeapon() const;

	virtual void UpdateWeaponDisplay();
	int  IndexToSlot(int index) const;

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	virtual void OnWeaponPickup( C_BaseCombatWeapon *pWeapon );

	virtual int GetTeam() const { return TEAM_OVERLORD; };
protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void RevertAllImages();
	void RevertImage(int iSlot);
	void SetImageAsSelected(int index);
	void CreatePath(int size, char * path, const char * weapon, bool bSelected = false);

	CHandle<C_BaseCombatWeapon> m_hSelectedWeapon;
	vgui::ImagePanel * m_Image[MAX_POWERS];
	vgui::Label * m_Label[MAX_POWERS];


	vgui::HFont m_hFont;
	vgui::Frame * m_InfoPanel;
	vgui::Label * m_InfoWeaponName;
	vgui::Label * m_InfoPrimary;
	vgui::Label * m_InfoSecondary;
	vgui::Label * m_InfoCost;
};	



#endif