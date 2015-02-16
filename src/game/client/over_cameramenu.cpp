//==============Overlord mod=====================
//	Overlord's camera menu
//	
//===============================================

#include "cbase.h"
#include "over_cameramenu.h"
#include "hl2mp_gamerules.h"
#include <vgui_controls/RichText.h>
#include <vgui_controls/Button.h>
#include "over_popupsupport.h"
#include "overlord_camera.h"
#include "c_overlord_area.h"
#include "c_overlord_basemodule.h"
#include "overlord_consolemanager.h"
#include "vgui/IInput.h"

#include "tier0/vprof.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if 0

//#define CAMERA_THINK 0.2f
#define MODULE_STRING_LENGTH 6
#define MODULE_STRING_INT_LENGTH 2
#define MODULE_STRING_INT_POSITION (MODULE_STRING_LENGTH)
#define MODULE_STRING_LENGTH_FULL (MODULE_STRING_LENGTH + MODULE_STRING_INT_LENGTH)

COverlordCameraMenu::COverlordCameraMenu(IViewPort *pViewPort) : vgui::Frame(NULL, PANEL_CAMERAMENU )
{
	m_flNextUpdate = 0.0f;

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		m_pModules[i] = NULL;
	}

	m_CameraNext = NULL;
	m_CameraPrev = NULL;
	m_CameraDown = NULL;
	m_CameraUp = NULL;
	//m_pDescription = NULL;
	m_AreaLabel = NULL;
	m_SpawnsLabel = NULL;
	m_TrapManager = NULL;

	m_StasisButton = NULL;

	m_pViewPort = pViewPort;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	SetTitle("", true);

	SetMoveable(false);
	SetProportional(true);
	SetSizeable(false);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);

	CreateControls();

	LoadControlSettings("Resource/UI/CameraMenu.res");
	InvalidateLayout();

}

COverlordCameraMenu::~COverlordCameraMenu()
{
	DeleteControls();
}

void COverlordCameraMenu::DeleteControls()
{
	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		if(m_pModules[i])
			delete m_pModules[i];

		m_pModules[i] = NULL;
	}

	if(m_TrapManager)
	{
		delete m_TrapManager;
		m_TrapManager = NULL;
	}

	//if(m_pDescription)
	//	delete m_pDescription;

	if(m_CameraNext)
	{
		delete m_CameraNext;
		m_CameraNext = NULL;
	}

	if(m_CameraPrev)
	{
		delete m_CameraPrev;
		m_CameraPrev = NULL;
	}

	if(m_CameraUp)
	{
		delete m_CameraUp;
		m_CameraUp = NULL;
	}

	if(m_CameraDown)
	{
		delete m_CameraDown;
		m_CameraDown = NULL;
	}

	if(m_AreaLabel)
	{
		delete m_AreaLabel;
		m_AreaLabel = NULL;
	}

	if(m_SpawnsLabel)
	{
		delete m_SpawnsLabel;
		m_SpawnsLabel = NULL;
	}

	if(m_HealthLabel)
	{
		delete m_HealthLabel;
		m_HealthLabel = NULL;
	}

	if(m_StasisButton)
	{
		delete m_StasisButton;
		m_StasisButton = NULL;
	}
}

void COverlordCameraMenu::OnThink()
{
	VPROF_BUDGET("COverlordCameraMenu::OnThink", VPROF_BUDGETGROUP_OTHER_VGUI);

	if(m_flNextUpdate <= gpGlobals->curtime)
	{
		ResolveLabels();
		m_flNextUpdate = gpGlobals->curtime + 0.05f;
	}

	// Check whether we want to open trap manager
	if(m_TrapManager)
	{
		int x, y;
		vgui::input()->GetCursorPosition(x, y);

		if(m_TrapManager->IsCursorOver())
		{
			if(!g_ConsoleManager.IsInBuildMode())
			{
				gViewPortInterface->ShowPanel(PANEL_TRAPMANAGER, true);
			}
		}
	}

}

void COverlordCameraMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("BaseBorder"));

	// Similiar to classmenu, no pScheme pointer before this call
	//if(m_pDescription)
	//	m_pDescription->SetBorder(pScheme->GetBorder("BaseBorder"));

	m_defaultFgColor = GetSchemeColor( "Button.TextColor", pScheme );

	SetPaintBackgroundType(2);
}


void COverlordCameraMenu::CreateControls()
{
	DeleteControls();

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		char buttonname[16];

		// Create full name
		Q_snprintf(buttonname, ARRAYSIZE(buttonname), "Button%d", i);

		if(!m_pModules[i])
		{
			m_pModules[i] = new COverlordModuleButton(this, buttonname, " ");
			m_pModules[i]->SetProportional(true);
		}
		else
			Warning("Module button already exists!\n");
	}

	// Initing description settings
	//m_pDescription = new vgui::RichText(this, "Description");
	//m_pDescription->SetUnusedScrollbarInvisible(true);
	//m_pDescription->SetPaintBackgroundType(2);	
	//m_pDescription->SetProportional(true);

	m_CameraNext = new vgui::Button(this, "CameraForw", "");
	m_CameraPrev = new vgui::Button(this, "CameraBack", "");

	m_CameraUp = new vgui::Button(this, "CameraUp", "");
	m_CameraDown = new vgui::Button(this, "CameraDown", "");

	m_TrapManager = new vgui::Button(this, "TrapManagerButton", "");

	m_AreaLabel = new vgui::Label(this, "AreaName", "");
	m_SpawnsLabel = new vgui::Label(this, "SpawnsLabel", "");
	m_HealthLabel = new vgui::Label(this, "HealthLabel", "");

	m_StasisButton = new vgui::Button(this, "GlobalStasis", "");
}

void COverlordCameraMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

//	SetKeyBoardInputEnabled(false);

	if (bShow)
	{
		//Activate();
		
		ResolveLabels();

		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(true);
		SetVisible(true);
		m_flNextUpdate = gpGlobals->curtime;
	}
	else
	{
		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(false);
		SetVisible(false);
		gViewPortInterface->ShowPanel(PANEL_TRAPMANAGER, false);
		m_flNextUpdate = 0.0f;
	}

}

void COverlordCameraMenu::ResolveLabels()
{
	GET_OVERLORD_DATA_ASSERT(pData);

	C_OverlordCamera * pCamera = pData->GetCamera();

	if(!pCamera)
		return;
	const Color blank(0, 0, 0, 0);

	for(int i = 0; i < MAX_BUTTONS; i++)
	{
		if(!m_pModules[i])
			continue;

		m_pModules[i]->SetDefaultColor(m_defaultFgColor, blank);


		//if(cam->GetButtonName(i) == 0)
			//continue;

		// TODO: Fix a crash that occurs here
		// FIXED?
		m_pModules[i]->SetText(pCamera->GetButtonCaption(i));

		static_cast<COverlordModuleButton*>(m_pModules[i])->SetDescription(pCamera->GetButtonRecord(i));

		// Apply visibility according to state
		if(!pCamera->IsButtonVisible(i))
		{
			// Hide button!
			m_pModules[i]->SetVisible(false);
			continue;
		}
		else
		{
			m_pModules[i]->SetVisible(true);
		}

		Color bgColor = pCamera->GetButtonColor(i);

		m_pModules[i]->SetDefaultColor(m_defaultFgColor, bgColor);
	}

	//m_pDescription->SetText(pCamera->GetDescription());

	if(m_AreaLabel)
	{
		C_OverlordArea * pArea = pCamera->GetArea();
		char label[48];
		if(!pArea)
		{
			Q_snprintf(label, ARRAYSIZE(label), "Area: None");
		}
		else
		{
			Q_snprintf(label, ARRAYSIZE(label), "Area: %s", pArea->GetAreaName());
		}

		m_AreaLabel->SetText(label);
	}

	if(m_SpawnsLabel)
	{
		char label[24];
		Q_snprintf(label, ARRAYSIZE(label), "Spawns: %i/%i", int(pData->GetSpawnsAmount()), int(pData->GetInitialSpawns()));

		m_SpawnsLabel->SetText(label);
	}

	if(m_HealthLabel)
	{
		char label[32];
		Q_snprintf(label, ARRAYSIZE(label), "Health: %i/%i", pData->GetOverlord()->GetHealth(), 
			pData->GetHealthCap());

		m_HealthLabel->SetText(label);
	}

	if(m_StasisButton)
	{
		if(pData->IsInStasis())
		{
			if(m_StasisButton->IsEnabled())
				m_StasisButton->SetEnabled(false);

			float time = pData->GetStasisEnd() - gpGlobals->curtime;
		
			if(time <= 0)
				time = 1.0f;

			char label[32];
			Q_snprintf(label, ARRAYSIZE(label), "Stasis (%i)", int(ceil(time)));

			m_StasisButton->SetText(label);
		}
		else
		{
			if(pData->GetStasisCooldown() > gpGlobals->curtime)
			{
				float time = pData->GetStasisCooldown() - gpGlobals->curtime;
		
				if(time <= 0)
					time = 1.0f;

				char label[32];
				Q_snprintf(label, ARRAYSIZE(label), "Stasis (%i)", int(time));

				m_StasisButton->SetText(label);		
			}
			else
			{
				if(!m_StasisButton->IsEnabled())
				{
					m_StasisButton->SetEnabled(true);
					m_StasisButton->SetText("Stasis");
				}
			}
			
		}
	}
	//char chdesc[DESC_LENGTH] = "\0";

	
}

int COverlordCameraMenu::HoversOverModule()
{
	for(int i = 0; i < ARRAYSIZE(m_pModules); i++)
	{
		if(!m_pModules[i])
			continue;

		if(m_pModules[i]->IsCursorOver())
			return i;
	}

	if(m_CameraNext && m_CameraNext->IsCursorOver())
		return NEXTCAMERA;

	if(m_CameraPrev && m_CameraPrev->IsCursorOver())
		return PREVIOUSCAMERA;

	if(m_CameraUp && m_CameraUp->IsCursorOver())
		return UPPERCAMERA;

	if(m_CameraDown && m_CameraDown->IsCursorOver())
		return LOWERCAMERA;
	
	// Hovers over none
	return -1;
}

void COverlordCameraMenu::OnCommand(const char *command)
{
	if(!Q_stricmp(command, "closeconsole"))
	{
		engine->ServerCmd("eo_exitconsole\n");
		return;
	}
	else if(!Q_stricmp(command, "cameraforw"))
	{
		engine->ServerCmd("eo_cameraforward");
		return;
	}
	else if(!Q_stricmp(command, "cameraback"))
	{
		engine->ServerCmd("eo_cameraback");
		return;
	}
	else if(!Q_stricmp(command, "cameraup"))
	{
		engine->ServerCmd("eo_cameraup");
		return;
	}
	else if(!Q_stricmp(command, "cameradown"))
	{
		engine->ServerCmd("eo_cameradown");
		return;
	}
	else if(!Q_strnicmp(command, "module", 6))
	{

		CCommand cmd;
		cmd.Tokenize(command);

		//DevMsg("Module: %i\n", atoi(cmd.Arg(1)));

		engine->ServerCmd(VarArgs("eo_activate_module %i", atoi(cmd.Arg(1))));
		//VarArgs(final, MODULE_STRING_LENGTH_FULL)
	}
	else if(!Q_stricmp(command, "lasthacked"))
	{
		engine->ServerCmd("eo_lasthacked");
	}
	else if(!Q_stricmp(command, "stasis"))
	{
		engine->ServerCmd("eo_stasis");
	}
}





#endif