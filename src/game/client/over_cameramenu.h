//==============Overlord mod=====================
//	Overlord's camera menu
//	
//===============================================

#ifndef C_H_OV_CAMERA_MENU
#define C_H_OV_CAMERA_MENU

#if 0


#include "overlord_data.h"
#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>

class vgui::RichText;

enum
{
	// NEEDS TO BE SIX!
	NEXTCAMERA = 6,
	PREVIOUSCAMERA = 7,
	UPPERCAMERA,
	LOWERCAMERA,
};

class COverlordCameraMenu :  public vgui::Frame, public IViewPortPanel
{
public:
	DECLARE_CLASS_SIMPLE(COverlordCameraMenu, vgui::Frame);

	COverlordCameraMenu(IViewPort *pViewPort);
	virtual ~COverlordCameraMenu();

	const char * GetName() { return PANEL_CAMERAMENU; };

	virtual void OnThink();

	virtual void SetData(KeyValues *data) {};
	virtual void Reset() {};
	virtual void Update() {};
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	virtual void OnCommand(const char *command);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	//void __MsgFunc_UpdateCamera(bf_read &msg);
	int	HoversOverModule();

protected:
	virtual void DeleteControls();
	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);
	virtual void CreateControls();
	virtual void ResolveLabels();

	//vgui::RichText * m_pDescription;

	vgui::Button * m_pModules[MAX_BUTTONS];

	vgui::Button * m_CameraNext;
	vgui::Button * m_CameraPrev;
	vgui::Button * m_CameraUp;
	vgui::Button * m_CameraDown;

	vgui::Button * m_TrapManager;

	vgui::Label * m_SpawnsLabel;
	vgui::Label * m_HealthLabel;
	vgui::Label * m_AreaLabel;

	vgui::Button * m_StasisButton;
private:
	float m_flNextUpdate;
	Color m_defaultFgColor;
	IViewPort * m_pViewPort;
};

#endif
#endif
