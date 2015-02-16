//==============Overlord mod=====================
//	Manager class for the Overlord's in-console
//	view and abilities
//===============================================

#include "cbase.h"
#include "overlord_consolemanager.h"
#include "baseviewport.h"
#include "hl2mp_gamerules.h"
//#include <game/client/iviewport.h>
#include "vgui/IInput.h"
#include "in_buttons.h"
#include "iclientmode.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "ienginevgui.h"
#include "c_team.h"
#include "iviewrender_beams.h"
#include "c_hl2mp_player.h"
#include "c_physicsprop.h"
#include "c_overlord_basemodule.h"
#include "over_cameramenu.h"
#include "over_trapmanager.h"
#include "overlord_powerups_shared.h"

// Dynamic lights
#include "IEfx.h"
#include "dlight.h"

#include "c_baseplayer.h"
#include "overlord_dynamictraps.h"

#include "tier0/vprof.h"

//static COverlordConsoleManager g_ConsoleManager;

//#define CONSOLE_INDICATOR "ConsolePlayer"

// We can set whether we can see player's radius or not
ConVar eo_cl_playerradius("eo_cl_playerradius", "1", FCVAR_ARCHIVE);
ConVar eo_console_buildfreeze("eo_console_buildfreeze", "0.15");
ConVar eo_console_manager_sensitivity("eo_console_manager_sensitivity", "0.09");

extern void ScreenToWorld( int mousex, int mousey, float fov,
					const Vector& vecRenderOrigin,
					const QAngle& vecRenderAngles,
					Vector& vecPickingRay );

COverlordConsoleManager g_ConsoleManager;

#define GHOST_ALPHA 200
#define MODULE_GLOW_COLOR Color(255, 255, 255, 255)

// List of all panels on which the mouseclick will not register
namespace
{
	const char s_PanelList[][40] = 
	{
		//PANEL_CAMERAMENU,
		PANEL_SELECTION,
		PANEL_TRAPMENU,
		PANEL_POPUP,
		PANEL_TRAPMANAGER,
		PANEL_POWERUPPANEL,
	};
}

COverlordConsoleManager::COverlordConsoleManager()
{
	m_GlowEntity = NULL;
	m_LastIndex = -1;
	m_pDynamicLight = NULL;
	m_bInit = false; 
	m_Selected = NULL;
	m_LastEntity = NULL;
	m_BuildMode = BL_NONE;
	m_RadiusTexture = -1;
	m_flFreezeBuild = 0.0f;
	m_bFreeroam = false;
	m_LastSurfFlags = 0;

	//m_FreeroamX = 0;
	//m_FreeroamY = 0;

	m_GroupControl = -1;

	m_BeamMat = NULL;
	m_flInputDelay = 0.0f;
	m_bOverrideAngle = false;
	m_angOverride = QAngle(0, 0, 0);
	m_NormalOverride = vec3_origin;
	m_AxisLock = Z_AXIS;
	m_bPowerupPanel = false;
}

COverlordConsoleManager::~COverlordConsoleManager()
{

}

void COverlordConsoleManager::FireGameEvent(IGameEvent * event)
{
	C_BasePlayer * pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pLocalPlayer || !pLocalPlayer->IsOverlord())
		return;

	if(event)
	{
		if(!Q_stricmp(event->GetName(), "camera_exited"))
		{
			int entindex = event->GetInt("entindex");

			if(entindex)
			{
				C_OverlordCamera * pEnt = static_cast<C_OverlordCamera*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					pEnt->OnCameraExit();

					if(!Q_stricmp(pEnt->GetClientClass()->GetName(), "COverlordFreeRoam"))
						m_bFreeroam = false;
				}
			}
		}
		else if(!Q_stricmp(event->GetName(), "camera_entered"))
		{
			// Disable freeroam on entering some other camera
			int entindex = event->GetInt("entindex");

			if(entindex)
			{
				C_OverlordCamera * pEnt = static_cast<C_OverlordCamera*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					pEnt->OnCameraEnter();

					if(Q_stricmp(pEnt->GetClientClass()->GetName(), "COverlordFreeRoam"))
						m_bFreeroam = false;
				}
			}
		}
		else if(!Q_stricmp(event->GetName(), "trap_built"))
		{
			if(m_GroupControl != -1)
			{
				int entindex = event->GetInt("entindex");

				if(entindex)
				{
					C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				
					if(pEnt)
					{
						/*ControlGroup_t & controlGroup = GetControlGroup(m_GroupControl);

						ControlGroupInfo_t info;
						info.m_hEntity = pEnt;
						info.m_EntityType = ControlGroupInfo_t::TYPE_TRAP;

						controlGroup.AddToTail(info);*/
						AddToControlGroup(pEnt, m_GroupControl, ControlGroupInfo_t::TYPE_TRAP, false);
					}
				}


				m_GroupControl = -1;
			}
		}
	}
}

void COverlordConsoleManager::Precache()
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(m_RadiusTexture == -1)
	{
		if(pPlayer)
		{
			m_RadiusTexture = pPlayer->PrecacheModel("sprites/physbeam.vmt"); //
		}
	}
	
	if(!m_BeamMat)
		m_BeamMat = materials->FindMaterial("sprites/bluelaser1", TEXTURE_GROUP_CLIENT_EFFECTS);

	CBaseEntity::PrecacheScriptSound("HL2Player.UseDeny");
	CBaseEntity::PrecacheScriptSound("NPC_FloorTurret.Ping");
}

void COverlordConsoleManager::Init()
{
	//ShowConsolePanel(true);

	m_GroupControl = -1;
	m_GlowEntity = NULL;
	m_bInit = true;
	ExitBuildMode();
	m_bFreeroam = false;
	m_StaticTrap = NULL;

	OverrideAngles(false);

	ListenForGameEvent("camera_entered");
	ListenForGameEvent("camera_exited");
	ListenForGameEvent("trap_built");

	Precache();

	// Add glows
	C_OverlordCamera * pCam = GetOverlordData()->GetCamera();

	if(pCam)
		pCam->OnCameraEnter();

	// Delay the freeroam entrance
	m_flInputDelay = gpGlobals->curtime + 0.25f;
}

void COverlordConsoleManager::DeInit()
{
	//ShowConsolePanel(false);
	gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, false);
	gViewPortInterface->ShowPanel(PANEL_POPUP, false);
	gViewPortInterface->ShowPanel(PANEL_STATICTRAPPANEL, false);

	m_bInit = false;
	m_bFreeroam = false;
	m_LastSurfFlags = 0;

	// Set no overlay
	//view->SetScreenOverlayMaterial(NULL);

	StopListeningForAllEvents();

	ExitBuildMode();
	DeselectEntity();

	m_RadiusTexture = -1;

	//ShowConsolePanel(false);

	if(m_GlowEntity)
		DeregisterGlowEntity();

	m_flInputDelay = 0.0f;
	/*C_Team * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(pRebels)
	{
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			// "shadertest/wireframevertexcolor"
			C_HL2MP_Player * pRebel = dynamic_cast<C_HL2MP_Player*>(pRebels->GetPlayer(i));

			if(!pRebel || !pRebel->IsAlive())
				continue;

			pRebel->SetMaterialOverlay(NULL);
		}
	}*/

	//view->DrawAllPlayersOfTeam(TEAM_INVALID);

	for(int i = 0; i < MAX_CONTROL_GROUPS; i++)
	{
		ControlGroup_t & control = m_ControlGroup[i];

		if(control.Count() == 0)
			continue;

		control.Purge();
	}
}

void COverlordConsoleManager::RegisterGlowEntity(C_BaseEntity * pEnt, Color clr)
{
	if(!pEnt)
		return;

	if(m_GlowEntity != pEnt)
	{
		DeregisterGlowEntity();
		pEnt->RegisterGlow(clr);
	}
	else
		pEnt->SetGlowColor(clr);
	
	m_GlowEntity = pEnt;
}

void COverlordConsoleManager::DeregisterGlowEntity()
{
	if(m_GlowEntity)
	{
		m_GlowEntity->DeregisterGlow();
		m_GlowEntity = NULL;
	}
}

void COverlordConsoleManager::Run()
{
	VPROF_BUDGET("COverlordConsoleManager::Run", VPROF_BUDGETGROUP_OTHER_VGUI);

	C_BasePlayer * pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pLocalPlayer || !pLocalPlayer->IsOverlord() || !GET_OVERLORD_DATA)
	{
		if(IsInited())
			DeInit();

		return;
	}

	if(!HL2MPRules())
		return;

	COverlordData & data = *GET_OVERLORD_DATA;
	// Init the settings first

	if(data.IsInVehicle() && !IsInited())
	{
		Init();
	}
	else if(!data.IsInVehicle() && IsInited())
	{
		DeInit();
	}
	if(!data.IsInVehicle())
		return;

	// Don't run it when playing a demo!
	if(engine->IsPlayingDemo())
		return;

	if(enginevgui->IsGameUIVisible())
		return;

	HandleAllNumericKeys();

	DoMouseButtonChecks();

	if(ShouldEnterFreeroam())
	{
		EnterFreeroam();
	}

	if(!IsInBuildMode())
	{
		OutsideBuildModeThink();
	}
	else
	{		
		BuildModeThink();
	}

	// Try trap manager now
	HandleManagerHints();

}


void COverlordConsoleManager::HandleManagerHints()
{
	// We need to control whether we've enabled glow or not
	static bool bGlow = false;

	COverlordTrapManager * pManager = static_cast<COverlordTrapManager*>(gViewPortInterface->FindPanelByName(PANEL_TRAPMANAGER));

	if(!pManager || !pManager->IsVisible() || !pManager->GetTrapHover())
	{
		// Deglow us
		if(bGlow)
		{
			DeregisterGlowEntity();
			bGlow = false;
		}

		return;
	}

	C_OverlordCamera * pCamera = GET_OVERLORD_DATA->GetCamera();

	if(pCamera)
	{
		C_OverlordTrap * pTrap = pManager->GetTrapHover();

		// Glow us
		if(m_GlowEntity != pTrap)
		{
			RegisterGlowEntity(pTrap, MODULE_GLOW_COLOR);
			bGlow = true;
		}

		/*color32 color;
		color.a = 255;
		color.r = 255;
		color.g = 0;
		color.b = 0;

		FXLineData_t data; 
		data.m_flDieTime = 0.018f;

		// Calculate spot below us
		Vector relDown;
		VectorRotate(Vector(0, 0, -1), pCamera->GetCameraAngles(), relDown);

		data.m_vecStart = pCamera->GetCameraVector() + relDown * 16;
		data.m_vecEnd = pTrap->WorldSpaceCenter();
		data.m_pMaterial = m_BeamMat;
		data.m_Color = color;
		data.m_flStartScale = 2;
		data.m_flEndScale = 2;
		data.m_flStartAlpha = 255;
		data.m_flEndAlpha = 255;

		data.m_vecStartVelocity = vec3_origin;
		data.m_vecEndVelocity = vec3_origin;

		FX_AddLine(data);*/
	}
}

void COverlordConsoleManager::EnterFreeroam(const Vector & pos /*= vec3_origin*/, const QAngle & angle /*= vec3_angle*/)
{
	CloseAllAuxiliaryPanels();
	m_bFreeroam = true;

	DeregisterGlowEntity();

	m_GroupControl = -1;

	// In the rare condition of clicking at 0, 0, 0 and having 0, 0, 0 angle, we won't be able to spawn
	// a freeroam (or via command), but that's not a likely case
	if(pos == vec3_origin && angle == vec3_angle)
	{
		engine->ServerCmd("eo_freeroam");
	}
	else
	{
		char command[128];
		Q_snprintf(command, ARRAYSIZE(command), "eo_freeroam %f %f %f %f %f %f", pos.x, pos.y, pos.z,
																				angle.x, angle.y, angle.z);
		engine->ServerCmd(command);
	}
}
bool COverlordConsoleManager::IsNonPanelMouseClick() const
{
	int x, y;
	input()->GetCursorPosition(x, y);

	// Do not allow clicking while the menu is visible
	if(enginevgui->IsGameUIVisible())
		return false;

	// Check all console panels
	for(int i = 0; i < ARRAYSIZE(s_PanelList); i++)
	{	
		// We check children of every panel if the panel we are looking for isn't
		// a direct child of the viewport.
		vgui::Panel * iPanel = g_pClientMode->GetViewport()->FindChildByName(s_PanelList[i], true);
		if(!iPanel)
		{
			Warning("Panel in console manager does not exist!\n");
			continue;
		}

		if(iPanel->IsVisible() && iPanel->IsWithin(x, y))
			return false;

	}

	return true;
}

void COverlordConsoleManager::OutsideBuildModeThink()
{
	// Check whether it's a trap
	CBaseEntity * pUnderCursor = GetEntityUnderCursor();
	if(pUnderCursor && COverlordTrap::EntityToTrap(pUnderCursor))
	{
		// GLow us up
		if(!GetHoverTrap())
			RegisterGlowEntity(pUnderCursor, MODULE_GLOW_COLOR);

		// We were hovering over another trap before,
		// restart the status
		if(GetHoverTrap() && GetHoverTrap() != pUnderCursor)
		{
			RegisterGlowEntity(pUnderCursor, MODULE_GLOW_COLOR);
			gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, false);
		}

		m_HoverTrap = static_cast<C_OverlordTrap*>(pUnderCursor);
		gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, true);

		m_StaticTrap = NULL;
	}
	else
	{
		if(GetHoverTrap())
		{
			if(m_GlowEntity == m_HoverTrap)
			{
				DeregisterGlowEntity();
			}

			m_HoverTrap = NULL;
			gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, false);
		}

		HandleStaticTrapHover();
	}

	if(!GetHoverStaticTrap())
		gViewPortInterface->ShowPanel(PANEL_STATICTRAPPANEL, false);

	// Show player menu should we hover over a player
	if(ToBasePlayer(pUnderCursor))
	{
		SelectPlayer(ToBasePlayer(GetEntityUnderCursor()));
	}
	else
	{
		if(IsPlayerSelected())
			DeselectPlayer();
	}
}

void COverlordConsoleManager::BuildModeThink()
{
	static bool bEKeyPressed = false;
	static bool bQKeyPressed = false;

	HandleColours();

	if(m_Building)
	{
		// Set the light's position 
		if(m_pDynamicLight)
			m_pDynamicLight->origin = m_Building->WorldSpaceCenter() + (GetPlaneNormal() * 32);

		if(GetBuildMode() == BL_MOVE)
		{
			// Handle 90 degrees right/left rotations
			if(!bEKeyPressed && input()->IsKeyDown(KEY_E))
			{
				bEKeyPressed = true;
				QAngle angle = m_Building->GetAbsAngles();
				angle[YAW] += 90;
				m_Building->SetAbsAngles(angle);
				OverrideAngles(true, m_LastNormal, angle);
			}
			else if(bEKeyPressed && !input()->IsKeyDown(KEY_E))
			{
				bEKeyPressed = false;
			}

			if(!bQKeyPressed && input()->IsKeyDown(KEY_Q))
			{
				bQKeyPressed = true;
				QAngle angle = m_Building->GetAbsAngles();
				angle[YAW] -= 90;
				m_Building->SetAbsAngles(angle);
				OverrideAngles(true, m_LastNormal, angle);
			}
			else if(bQKeyPressed && !input()->IsKeyDown(KEY_Q))
			{
				bQKeyPressed = false;
			}
		}

	}

	// Draw radius around player
	if(eo_cl_playerradius.GetBool() && m_RadiusTexture != 0)
	{
		CSingleUserRecipientFilter player(C_BasePlayer::GetLocalPlayer());

		
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);
			if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive() || pPlayer->IsInvisible())
				continue;

			// Multiply it by two to get diameter which it needs
			beams->CreateBeamRingPoint(pPlayer->WorldSpaceCenter(),
				(2*eo_trapsdistance.GetFloat()) - 10, // start radius
				2*eo_trapsdistance.GetFloat(), // end radius
				m_RadiusTexture, 
				0, // halo index
				0, // halo scale
				0.05f, // life 
				10, // width
				10, // end width
				10,	// fade length
				2, // Amplitude
				80, // Brightness
				0, // Speed
				0, //Start Frame
				1, // framerate
				255, // r
				0, // g
				0,  // b
				0); // flags
		}
	}
}

void COverlordConsoleManager::DoMouseButtonChecks()
{
	static bool bMouse1Pressed = false;
	static bool bMouse2Pressed = false;
	static bool bMouse3Pressed = false;
	static float flMouse1Delay = 0.0f;
	static float flMouse2Delay = 0.0f;
	static float flMouse3Delay = 0.0f;

	if(input()->IsMouseDown(MOUSE_LEFT) && !bMouse1Pressed)
	{
		if(IsNonPanelMouseClick())
		{
			flMouse1Delay = gpGlobals->curtime + 0.009f;
			HandleLeftPressed();
		}

		bMouse1Pressed = true;
	}
	else if(!input()->IsMouseDown(MOUSE_LEFT)&& bMouse1Pressed && flMouse1Delay <= gpGlobals->curtime)
	{
		flMouse1Delay = 0.0f;
		bMouse1Pressed = false;
	}

	if(input()->IsMouseDown(MOUSE_RIGHT) && !bMouse2Pressed)
	{
		if(IsNonPanelMouseClick())
		{
			flMouse2Delay = gpGlobals->curtime + 0.005f;
			HandleRightPressed();
		}

		bMouse2Pressed = true;
	}
	else if(!input()->IsMouseDown(MOUSE_RIGHT)&& bMouse2Pressed && flMouse2Delay <= gpGlobals->curtime)
	{
		bMouse2Pressed = false;
		flMouse2Delay = 0.0f;
	}

	if(input()->IsMouseDown(MOUSE_MIDDLE) && !bMouse3Pressed)
	{
		if(IsNonPanelMouseClick())
		{
			flMouse3Delay = gpGlobals->curtime + 0.115f;
			HandleMiddlePressed();
		}

		bMouse3Pressed = true;
	}
	else if(!input()->IsMouseDown(MOUSE_MIDDLE)&& bMouse3Pressed && flMouse3Delay <= gpGlobals->curtime)
	{
		bMouse3Pressed = false;
		flMouse3Delay = 0.0f;
	}
}

void COverlordConsoleManager::HandleLeftPressed()
{
	if(GetOverlordData()->GetCamera()->ShouldBlockControls())
		return;

	// Clicking outside of the selection panel throws us out of the build mode
	if(GetBuildMode() == BL_SELECT)
	{
		ExitBuildMode();
		return;
	}

	if(IsPowerupPanelOpen())
	{
		SetPowerupPanelOpen(false);
		return;
	}

	if(IsSelected())
	{
		DeselectEntity();
		return;
	}	

	trace_t tr;
	C_BaseEntity * pEnt = SelectEntity(&m_vSelection, &tr);

	// First handle it if it isn't in buildmode
	if(!IsInBuildMode())
	{
		// First check our entity-specific clicks
		if(pEnt && COverlordTrap::EntityToTrap(pEnt))
		{
			ClickOnTrap(static_cast<C_OverlordTrap*>(pEnt));
		}
		else
		{
			// Now build
			if(!tr.startsolid && !tr.allsolid)
			{
				// Handle build press
				m_LastEntity = pEnt;
				m_LastSurfFlags = tr.surface.flags;

				if(m_LastEntity)
				{
					Vector temp = m_vSelection;
					m_LastEntity->WorldToEntitySpace(temp, &m_vSelection);
					m_LastEntity->WorldToEntitySpace(tr.plane.normal, &m_LastNormal);
				}
				else
				{
					m_LastNormal = tr.plane.normal;
				}

				EnterBuildMode();
			}
		}
	}
	else
	{
		// We are in build mode, so handle any build-presses now
		bool bShiftPressed = input()->IsKeyDown(KEY_LSHIFT) || input()->IsKeyDown(KEY_RSHIFT);

		// Cancel if we press it with right mouse button
		if((!bShiftPressed && GetBuildMode() == BL_MOVE))
		{
			if(m_flFreezeBuild <= gpGlobals->curtime)
			{
				/*if(CanBuildTrap())
				{*/
					m_BuildMode = BL_ROTATE;
					
					engine->ServerCmd("EnterRotateTrap");
				/*}
				else
				{
					error_t error = GetGhost() ? GetGhost()->CanBuild() : error_t();

					if(error.canBuild)
						FireHint("Cannot build in this place");
					else
						FireHint(error.desc);
				}*/
			}
			
		}
		else if(bShiftPressed || (GetBuildMode() == BL_ROTATE))
		{
			if(CanBuildTrap())
			{	
				bool bOverride = true;
				QAngle angOverride = m_Building->GetAbsAngles();
				Vector normal = m_LastNormal;
				Build();

				if(bShiftPressed)
				{
					if((m_LastIndex >= 0) &&
						(m_LastIndex < (NUM_OF_TRAPS)))
					{
						EnterBuildMode(false);

						const C_OverlordTrap::STrapRecord & record = C_OverlordTrap::GetRecord(m_LastIndex);

						OverrideAngles(bOverride, normal, angOverride);

						BuildGhost(record.entName);

						if(m_bOverrideAngle)
							m_Building->SetAbsAngles(m_angOverride);
					}
				}
			}
			else
			{
				EmitSound("HL2Player.UseDeny");
			}
		}
	}



}

void COverlordConsoleManager::HandleMiddlePressed(void)
{
	if(GetOverlordData()->GetCamera()->ShouldBlockControls())
		return;

	if(!IsInBuildMode())
	{
		trace_t tr;
		SelectEntity(NULL, &tr);

		Vector pos = tr.endpos + tr.plane.normal * 16;
		Vector dir = tr.startpos - pos;
		VectorNormalize(dir);

		QAngle ang;
		VectorAngles(dir, ang);

		EnterFreeroam(pos, ang);
	}
	else
	{
		/*if(GetBuildMode() == BL_ROTATE)
		{
			ExitBuildMode();
			// Also works for canceling to freeze the camera jump
			m_flFreezeBuild = gpGlobals->curtime + eo_console_buildfreeze.GetFloat();

			FireHint("Trap build canceled.");
		}*/
	}
}

void COverlordConsoleManager::HandleRightPressed(void)
{
	if(GetOverlordData()->GetCamera()->ShouldBlockControls())
		return;

	int x, y;
	input()->GetCursorPosition(x, y);

	if(IsInBuildMode())
	{
		if(GetBuildMode() == BL_ROTATE)
		{
			// Double click cancels!
			if((m_flRightClickHoldDelay != 0.0f) && (m_flRightClickHoldDelay <= gpGlobals->curtime))
			{
				CancelBuildMode();
				m_flRightClickHoldDelay = 0.0f;
				return;
			}
			//GetGhost()->SetAbsAngles(GetGhost()->NormalToAngle(GetPlaneNormal()));
			if(m_AxisLock == Y_AXIS)
				m_AxisLock = Z_AXIS;
			else
				m_AxisLock = Y_AXIS;

			m_flRightClickHoldDelay = gpGlobals->curtime + 0.125f;
		}
		else if(GetBuildMode() == BL_MOVE)
		{
			CancelBuildMode();
		}
	}
	else
	{
		if(m_flFreezeBuild <= gpGlobals->curtime)
		{
			trace_t tr;
			GetEntityUnderCursor(&tr);


			if(!COverlordTrap::EntityToTrap(tr.m_pEnt))
			{
				gViewPortInterface->ShowPanel(PANEL_TRAPMANAGER, true);
			}
			else
			{
				C_OverlordTrap * pTrap = static_cast<C_OverlordTrap*>(tr.m_pEnt);
				ClickOnTrap(pTrap, true);
			}

		}
	}
}

bool COverlordConsoleManager::CanBuildTrap() const
{
	if(!CanBuildOnEntity(m_LastEntity))
		return false;
	
	if(!CanBuildOnSurface(m_LastSurfFlags))
		return false;

	if(GetGhost() && !GetGhost()->CanBuild().canBuild)
		return false;

	return true;
}

bool COverlordConsoleManager::CanBuildOnEntity(CBaseEntity * pEnt) const
{
	if(!pEnt)
		return true;

	//if(!pEnt->IsStandable())
	//	return false;

	C_PhysicsProp * pPhys = dynamic_cast<C_PhysicsProp*>(pEnt);
	if(pPhys && !pPhys->CanHaveTrapsBuilt())
		return false;

	if(COverlordTrap::EntityToTrap(pEnt))
		return false;

	if(!Q_stricmp("CBreakableSurface", pEnt->GetClientClass()->GetName()))
		return false;

	if(!Q_stricmp("CPhysBox", pEnt->GetClientClass()->GetName()))
		return false;

	return true;
}

bool COverlordConsoleManager::CanBuildOnSurface(int flags) const
{
	if(flags & SURF_SKY)
		return false;

	if(flags & SURF_NODRAW)
		return false;

	return true;
}

void COverlordConsoleManager::HandleColours()
{
	// Apply colour depending on our power level
	if(m_Building)
	{
		color32 color = m_Building->GetRenderColor();
		color.g = 0;
		color.b = 0;
		color.r = 0;

		// Apply color depending whether you can build it
		if(!CanBuildTrap())
		{
			color.r = 255;
		}
		else
		{
			color.g = 255;
		}
		m_Building->SetRenderColor(color.r, color.g, color.b);

		if(m_pDynamicLight)
		{
			m_pDynamicLight->color.r = color.r;
			m_pDynamicLight->color.g = color.g;
			m_pDynamicLight->color.b = color.b;
		}

		// Update the glow every frame or so
		if(m_Building->GetRenderColor().a > 0)
			RegisterGlowEntity(m_Building, Color(color.r, color.g, color.b, color.a));
	}

	
}

void COverlordConsoleManager::RotateTrap(int x, int y)
{
	//DevMsg("Rot: X: %i Y: %i\n", x, y);

	if(x == 0 && y == 0)
		return;
	
	C_OverlordTrap * pTrap = GetGhost();

	if(!pTrap)
		return;

	bool bZlocked = IsZLocked();
	bool bYlocked = IsYLocked();
	float sensitivity = eo_console_manager_sensitivity.GetFloat();
	QAngle final = pTrap->GetAbsAngles();

	if(!input()->IsMouseDown(MOUSE_RIGHT) && m_flRightClickHoldDelay != 0.0f)
		m_flRightClickHoldDelay = 0.0f;

	if((m_flRightClickHoldDelay != 0.0f) && (m_flRightClickHoldDelay <= gpGlobals->curtime))
	{
		bYlocked = false;
		bZlocked = false;

		// Relock the Z axis
		m_AxisLock = Z_AXIS;
	}

	if(!bYlocked)
	{
		final += QAngle(0, -x * sensitivity, 0);
	}

	if(!bZlocked)
	{
		final += QAngle(y * sensitivity, 0, 0);
	}

	pTrap->SetAbsAngles(final);
#if 0
	//CBasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();
	
	static float flDelay =  gpGlobals->curtime;

	if(m_Building == NULL)
		return;

	const float multiplier = eo_console_manager_sensitivity.GetFloat();

	int x, y;
	input()->GetCursorPosition(x, y);

	// Check for locked axis
	bool bZlocked = IsZLocked();
	bool bYlocked = IsYLocked();;


	// Rotate
	if(!input()->IsMouseDown(MOUSE_RIGHT))
	{
		if(pX != x && !bYlocked)
		{
			// Rotate
			float diff = pX - x;
			diff *= multiplier;


			QAngle rotation(0, diff, 0);
			QAngle final = rotation + m_Building->GetAbsAngles();

			m_Building->SetAbsAngles(final);
		
		}

		if(pY != y && !bZlocked)
		{
			// Rotate
		//	int diff = pY - y;
			float diff = y - pY;
			diff *= multiplier;

	
			QAngle rotation(diff, 0, 0);
			QAngle final = rotation + m_Building->GetAbsAngles();

			m_Building->SetAbsAngles(final);
			
		}
	}
#endif
#if 0
	else
	{
		// We can easily control the speed of rotations with this
		if(flDelay > gpGlobals->curtime)
			return;

		// Use right angles if right mouse button is pressed

		int diffY = pY - y;
		int diffX = pX - x;

		if(diffY == 0 && diffX == 0)
			return;

		// Select the biggest difference
		const int & diff = (static_cast<unsigned int>(diffY) >= static_cast<unsigned int>(diffX)) ? diffY : diffX;

		flDelay = gpGlobals->curtime + 0.625f;

		if(diff == diffX && !bYlocked)
		{
			QAngle entAngle(0, int(m_Building->GetAbsAngles().y), 0);
			
			bool rightAngle = false;
			/*bool rightAngle = (int(entAngle.y) % 90 == 0 || int(entAngle.y) == 0) ? true : false;
			
			if(rightAngle)
				return;*/

			int temp = (diff >= 0) ? 1 : -1;

			for(entAngle.y = entAngle.y + temp; ; entAngle.y = entAngle.y + temp)
			{
				rightAngle = (((int(entAngle.y)) % 90 == 0) || (int(entAngle.y) == 0));
				if(rightAngle)
					break;
			}

			QAngle final = m_Building->GetAbsAngles();
			final.y = entAngle.y;

			m_Building->SetAbsAngles(final);
		}
		else if(diff == diffY && !bXlocked)
		{
			QAngle entAngle(int(m_Building->GetAbsAngles().x), 0, 0);
			
			bool rightAngle = false;
			/*bool rightAngle = (int(entAngle.x) % 90 == 0) ? true : false;
			
			if(rightAngle)
				return;*/

			int temp = (diff >= 0) ? 1 : -1;

			for(entAngle.x = entAngle.x + temp; ; entAngle.x = entAngle.x + temp)
			{
				rightAngle = (int(entAngle.x)) % 90 == 0;
				if(rightAngle)
					break;
			}

			QAngle final = m_Building->GetAbsAngles();
			final.x = entAngle.x;

			m_Building->SetAbsAngles(final);
		}
	}
#endif
}

void COverlordConsoleManager::MoveTrap()
{
	C_OverlordCamera * pCamera = (GET_OVERLORD_DATA)->GetCamera();

	if(!pCamera || !m_Building)
		return;

	trace_t tr;

	Vector start = pCamera->GetCameraVector();
	Vector forw;
	AngleVectors(pCamera->GetAbsAngles(), &forw);
	Vector end = start + forw * MAX_TRACE_LENGTH;

	UTIL_TraceLine(start, end, MASK_SHOT_HULL, pCamera, COLLISION_GROUP_NONE, &tr);

	m_LastEntity = tr.m_pEnt;
	m_LastSurfFlags	= tr.surface.flags;	

	if(m_bOverrideAngle && (m_LastNormal != tr.plane.normal))
	{
		OverrideAngles(false, m_LastNormal, m_angOverride);
	}
	else if(!m_bOverrideAngle && ShouldOverrideForNormal(tr.plane.normal))
	{
		OverrideAngles(true, tr.plane.normal, m_angOverride);
	}

	m_LastNormal = tr.plane.normal;

	m_Building->SetAbsOrigin(tr.endpos + m_Building->GetNormalMultiplier() * tr.plane.normal);
	m_Building->SetAbsAngles(GetBuildingAngle(tr.plane.normal));
	m_Building->SetParent(m_LastEntity);

	if((!CanBuildOnSurface(tr.surface.flags) || ToBasePlayer(tr.m_pEnt)) && m_Building->GetRenderColor().a != 0)
	{
		m_Building->SetRenderColorA(0);
		DeregisterGlowEntity();
	}
	else if((CanBuildOnSurface(tr.surface.flags) && !ToBasePlayer(tr.m_pEnt)) && m_Building->GetRenderColor().a != GHOST_ALPHA)
	{
		m_Building->SetRenderColorA(GHOST_ALPHA);
	}

}


bool COverlordConsoleManager::ShouldEnterFreeroam() const
{
	COverlordData & data = *GET_OVERLORD_DATA;
	CBasePlayer * pLocalPlayer = CBasePlayer::GetLocalPlayer();

	if(!pLocalPlayer)
		return false;
	
	if(m_flInputDelay > gpGlobals->curtime)
		return false;

	return !m_bFreeroam 
		&& IsInMouseRotation()
		&& (pLocalPlayer->m_nButtons & IN_FORWARD)
		&& Q_strcmp("COverlordFreeRoam", data.GetCamera()->GetClientClass()->GetName())
		&& ((GetBuildMode() != BL_SELECT) && (GetBuildMode() != BL_ROTATE));
}

QAngle COverlordConsoleManager::GetBuildingAngle(Vector normal) const
{
	if(m_bOverrideAngle)
		return m_angOverride;

	return m_Building->NormalToAngle(normal);
}

void COverlordConsoleManager::HandleAllNumericKeys()
{
	static bool bIsPressed[10];
	static bool bInited = false;

	if(!bInited)
	{
		for(int i = 0; i < ARRAYSIZE(bIsPressed); i++)
			bIsPressed[i] = false;

		bInited = true;
	}

	bool bControlPressed = input()->IsKeyDown(KEY_LCONTROL) || input()->IsKeyDown(KEY_RCONTROL);

	for(int i = 0; i < 10; i++)
	{
		ButtonCode_t button = static_cast<ButtonCode_t>(i+1);

		if(input()->IsKeyDown(button) && !bIsPressed[i])
		{
			HandleNumericKey(i, bControlPressed);
			bIsPressed[i] = true;
		}
		else if(!input()->IsKeyDown(button) && bIsPressed[i])
		{
			bIsPressed[i] = false;
		}
	}
	/*if(input()->WasKeyPressed(KEY_1))
		HandleNumericKey(1, bControlPressed);
	if(input()->WasKeyPressed(KEY_2))
		HandleNumericKey(2, bControlPressed);
	if(input()->WasKeyPressed(KEY_3))
		HandleNumericKey(3, bControlPressed);
	if(input()->WasKeyPressed(KEY_4))
		HandleNumericKey(4, bControlPressed);
	if(input()->WasKeyPressed(KEY_5))
		HandleNumericKey(5, bControlPressed);
	if(input()->WasKeyPressed(KEY_6))
		HandleNumericKey(6, bControlPressed);
	if(input()->WasKeyPressed(KEY_7))
		HandleNumericKey(7, bControlPressed);
	if(input()->WasKeyPressed(KEY_8))
		HandleNumericKey(8, bControlPressed);
	if(input()->WasKeyPressed(KEY_9))
		HandleNumericKey(9, bControlPressed);
	if(input()->WasKeyPressed(KEY_0))
		HandleNumericKey(0, bControlPressed);*/
}

void COverlordConsoleManager::HandleNumericKey(short key, bool bControlPressed)
{
	if(bControlPressed)
		WipeControlGroup(key);

	if(!IsInBuildMode())
	{
		C_BaseEntity * pAdd = (GetHoverTrap() != NULL) ? static_cast<C_BaseEntity*>(GetHoverTrap()) : 
			static_cast<C_BaseEntity*>(GetHoverStaticTrap());

		if(pAdd && !IsInControlGroup(key, pAdd))
		{
			AddToControlGroup(pAdd, key, GetHoverTrap() ? ControlGroupInfo_t::TYPE_TRAP : ControlGroupInfo_t::TYPE_MODULE);
		}
		else
		{	
			// Switch state/activate
			ControlGroup_t & controlGroup = GetControlGroup(key);

			if(controlGroup.Count() > 0)
				EmitSound("NPC_FloorTurret.Ping");

			for(int i = 0; i < controlGroup.Count(); i++)
			{
				const ControlGroupInfo_t & info = controlGroup[i];
				C_BaseEntity * pEnt = info.m_hEntity;

				if(!pEnt || (info.m_EntityType == ControlGroupInfo_t::TYPE_NONE))
				{
					controlGroup.Remove(i);
					i--;
					continue;
				}

				if(info.m_EntityType == ControlGroupInfo_t::TYPE_MODULE)
				{
					C_OverlordBaseModule * pModule = static_cast<C_OverlordBaseModule*>(pEnt);

					engine->ServerCmd(VarArgs("eo_activate_module %i", pModule->entindex()));
				}
				else
				{
					C_OverlordTrap * pTrap = static_cast<C_OverlordTrap*>(pEnt);

					int dormant = (pTrap->IsTrapDormant()) ? 0 : 1;

					Color clr(255, 255, 255, 255);
					if(dormant == 1)
						clr = Color(255, 0, 0, 255);
					else
						clr = Color(0, 255, 0, 255);
					pTrap->DoGlowBlink(clr);

					engine->ServerCmd(VarArgs("SetTrapDormant %i %i", dormant, info.m_hEntity.GetEntryIndex()));
				}
			}
		}
	}
	else
	{
		m_GroupControl = key;
		EmitSound("NPC_FloorTurret.Ping");
	}
}

void COverlordConsoleManager::AddToControlGroup(C_BaseEntity * pEnt, int group, unsigned short type, bool bSound /* = true */)
{
	ControlGroup_t & controlGroup = GetControlGroup(group);

	ControlGroupInfo_t info;
	info.m_EntityType = type;
	info.m_hEntity = pEnt;

	if(controlGroup.Find(info) != -1)
		return;

	controlGroup.AddToTail(info);

	if(bSound)
		EmitSound("NPC_FloorTurret.Ping");
}

void COverlordConsoleManager::RemoveFromControlGroup(C_BaseEntity * pEnt, int group)
{
	ControlGroup_t & controlGroup = m_ControlGroup[group];
	for(int i = 0; i < controlGroup.Count(); i++)
	{
		ControlGroupInfo_t info = controlGroup[i];

		if(info.m_hEntity == pEnt)
		{
			controlGroup.Remove(i);
			return;
		}
	}
}

bool COverlordConsoleManager::IsInControlGroup(int group, C_BaseEntity * pEnt) const
{
	const ControlGroup_t & controlGroup = GetControlGroup(group);

	for(int i = 0; i < controlGroup.Count(); i++)
	{
		ControlGroupInfo_t info = controlGroup[i];

		if(info.m_hEntity == pEnt)
		{
			return true;
		}
	}

	return false;
}

bool COverlordConsoleManager::ShouldOverrideForNormal(const Vector & normal) const
{
	return DotProduct(normal, m_NormalOverride) >= 0.9f;
}

C_BaseEntity * COverlordConsoleManager::SelectEntity(Vector * position, trace_t * tra)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;

	trace_t tr;

	GetEntityUnderCursor(&tr);

	if(position)
		*position = tr.endpos;

	if(tra)
		*tra = tr;

	if(tr.m_pEnt)
		return tr.m_pEnt;

	return NULL;
}

Vector COverlordConsoleManager::GetSelectedLocation() const
{
	if(m_LastEntity)
	{
		Vector tmp = m_vSelection;

		m_LastEntity->EntityToWorldSpace(m_vSelection, &tmp);

		return tmp;
	}

	return m_vSelection;
}

Vector COverlordConsoleManager::GetPlaneNormal() const
{
	if(m_LastEntity)
	{
		Vector tmp = m_LastNormal;

		m_LastEntity->EntityToWorldSpace(m_LastNormal, &tmp);

		return tmp;
	}


	return m_LastNormal;
}

void COverlordConsoleManager::SetPowerupPanelOpen(bool bOpen)
{
	m_bPowerupPanel = bOpen;
	gViewPortInterface->ShowPanel(PANEL_POWERUPPANEL, bOpen);
}

void COverlordConsoleManager::BuildGhost(const char * type, const Vector & location)
{
	C_OverlordTrap * pEnt = static_cast<C_OverlordTrap*>(CreateEntityByName(type));

	if(!pEnt)
	{
		Warning("No such trap.\n");
		ExitBuildMode();
		return;
	}

	if(pEnt->InitializeAsClientEntity( NULL, RENDER_GROUP_OPAQUE_ENTITY ) == false)
	{
		pEnt->Remove();
		Warning("Could not initialize the trap as a client entity.\n");
		ExitBuildMode();
		return;
	}



	int index = -1;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, pEnt->GetClientClass()->GetName()))
			continue;

		index = i;
		break;
	}
	m_LastIndex = index;



	Vector PlaneNormal = GetPlaneNormal();

	m_flFreezeBuild = gpGlobals->curtime + eo_console_buildfreeze.GetFloat();

	m_Building = pEnt;

	pEnt->SetAbsOrigin(location + (PlaneNormal * pEnt->GetNormalMultiplier()));

	// Calculate perpendicular angles
	pEnt->SetAbsAngles(GetBuildingAngle(PlaneNormal));

	pEnt->SetRenderMode(kRenderTransColor);
	pEnt->SetRenderColorA(GHOST_ALPHA);

	pEnt->Spawn();
	pEnt->SetParent(m_LastEntity.Get());

	if(m_Building->IsServerEntity())
	{
		Warning("Trap is not client-side\n");
	}

	// Delete dynamic light
	if(m_pDynamicLight)
	{
		m_pDynamicLight->die = gpGlobals->curtime;
		m_pDynamicLight = NULL;
	}

	dlight_t *dl = effects->CL_AllocDlight( 0 );

	dl->decay	= 0;
	dl->radius	= 150;

	dl->color.r = 255;
	dl->color.g = 255;
	dl->color.b = 255;
	dl->color.exponent = 5;

	dl->m_InnerAngle = 5.0f;
	dl->m_OuterAngle = 10.0f;
	dl->m_Direction = -PlaneNormal;

	dl->origin = pEnt->WorldSpaceCenter() + (PlaneNormal * 32);

	dl->die		=  gpGlobals->curtime + 1e6;

	m_pDynamicLight = dl;

	//input()->SetCursorPos(m_LastX, m_LastY);
	//input()->SetCursorPos(0, 0);

	m_BuildMode = BL_MOVE;
	engine->ServerCmd("EnterMoveTrap");
}

void COverlordConsoleManager::Build(const Vector & location)
{
	if(!m_Building)
	{
		ExitBuildMode();
		return;
	}

	// The ghost gets deleted here

	int index = -1;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		if(Q_strcmp(C_OverlordTrap::GetRecord(i).className, m_Building->GetClientClass()->GetName()))
			continue;

		index = i;
		break;
	}

	if(index == -1)
	{
		Warning("No entity found!\n");
		ExitBuildMode();
		return;
	}

	// Get position vector - relative or just world
	Vector pos;
	const QAngle & ang = m_Building->GetAbsAngles();

	int entindex = -1;
	if(m_Building->GetMoveParent())
	{
		EHANDLE handle = m_Building->GetMoveParent(); 
		entindex = handle.GetEntryIndex();
		Vector tmp = m_Building->GetAbsOrigin();
		m_Building->GetMoveParent()->WorldToEntitySpace(tmp, &pos);
	}
	else
	{
		pos = m_Building->GetAbsOrigin();
	}

	//==========================
	// Make sure we can build it
	//==========================

	error_t error = m_Building->CanBuild();
	if(!error.canBuild)
	{
		EmitSound("HL2Player.UseDeny");
		FireHint(error.desc);
		ExitBuildMode();
		return;
	}
	//=======================
	//		Now build
	//=======================


	// servercmd: BuildTrap <index> <Pos X> <Pos Y> <Pos Z> <Ang X> <Ang Y> <Ang Z> <Parent index>
	engine->ServerCmd(VarArgs("BuildTrap %i %f %f %f %f %f %f %i", int(index), float(pos.x), float(pos.y), float(pos.z),
		float(ang.x), float(ang.y), float(ang.z), int(entindex)));

	ExitBuildMode();
}


// Handles player selection, may receive null pointers
void COverlordConsoleManager::SelectPlayer(C_BasePlayer * pPlayer)
{
	// No player found, deselect current one
	if(!CanSelectPlayer(pPlayer))
		return;

	m_Selected = pPlayer;
	
	gViewPortInterface->ShowPanel(PANEL_SELECTION, true);
}

void COverlordConsoleManager::DeselectEntity()
{
	// Show a custom hint if we had selected a player
	if(IsPlayerSelected())
	{
		//FireHint("A player has been deselected.");
	}
	else if(IsTrapSelected())
	{
		//FireHint("Trap deselected.");
	}
	//else
	//{
	//	FireHint("Entity deselected.");
	//}
	m_Selected = NULL;

	gViewPortInterface->ShowPanel(PANEL_SELECTION, false);

}

void COverlordConsoleManager::SelectDynamicTrap(C_OverlordTrap * pTrap)
{
	
}

void COverlordConsoleManager::EnterBuildMode(bool bShowTrapMenu /*= true*/)
{
	if(bShowTrapMenu)
		gViewPortInterface->ShowPanel(PANEL_TRAPMENU, true);

	gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, false);
	gViewPortInterface->ShowPanel(PANEL_STATICTRAPPANEL, false);
	m_StaticTrap = NULL;
	//ShowConsolePanel(false);

	m_BuildMode = BL_SELECT;
	
	OverrideAngles(false);
	m_AxisLock = Z_AXIS;

	engine->ServerCmd("EnterBuildMode");
}

void COverlordConsoleManager::CancelBuildMode()
{
	m_GroupControl = -1;
	ExitBuildMode();
}
void COverlordConsoleManager::ExitBuildMode()
{
	if(m_Building)
	{
		m_Building->Remove();
		m_Building = NULL;
	}

	m_BuildMode = BL_NONE;

	//ShowConsolePanel(true);
	gViewPortInterface->ShowPanel(PANEL_TRAPMENU, false);

	if(m_pDynamicLight)
	{
		m_pDynamicLight->die = 0.0f;
		m_pDynamicLight = NULL;
	}

	m_flFreezeBuild = 0.0f;

	engine->ServerCmd("ExitBuildMode");
	OverrideAngles(false);
}


void COverlordConsoleManager::FireHint(const char * hint) const
{
	IGameEvent * event = gameeventmanager->CreateEvent("eo_customhint");
	if(event)
	{
		event->SetString("hint", hint);
		event->SetInt("userid", -1);

		gameeventmanager->FireEventClientSide(event);
	}
}
	
void COverlordConsoleManager::CloseAllAuxiliaryPanels()
{
	m_HoverTrap = NULL;
	gViewPortInterface->ShowPanel(PANEL_TRAPSTATUS, false);

	DeselectEntity();
	gViewPortInterface->ShowPanel(PANEL_POPUP, false);

	gViewPortInterface->ShowPanel(PANEL_POWERUPPANEL, false);
}

void COverlordConsoleManager::EmitSound(const char * sound) const
{
	COverlordCamera * pCamera = GetOverlordData()->GetCamera();

	if(!pCamera)
		return;

	Vector vec = pCamera->GetCameraVector();
	CLocalPlayerFilter filter;
	C_BaseEntity::EmitSound( filter, pCamera->entindex(), sound, &vec );
}

bool COverlordConsoleManager::IsRoaming() const
{ 
	return m_bFreeroam && IsInMouseRotation(); 
}

bool COverlordConsoleManager::IsInMouseRotation() const
{
	return ( GetOverlordData()->GetCamera() ? GetOverlordData()->GetCamera()->IsInMouseRotation() : false);
}

bool COverlordConsoleManager::IsZLocked() const
{
	return m_AxisLock == Z_AXIS;
	//return C_BasePlayer::GetLocalPlayer()->m_nButtons & IN_LOCKXAXIS;
}

bool COverlordConsoleManager::IsYLocked() const
{ 
	return m_AxisLock == Y_AXIS;
	//return C_BasePlayer::GetLocalPlayer()->m_nButtons & IN_LOCKYAXIS;
}

C_BaseEntity * COverlordConsoleManager::GetEntityUnderCursor(trace_t * tra)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;
	
	COverlordCamera * pCam = GetOverlordData()->GetCamera();

	if(!pCam)
		return NULL;

	Vector origin = pCam->GetCameraVector();
	QAngle angle = pCam->GetCameraAngles();
	
	Vector forw;
	AngleVectors(angle, &forw);
	
	trace_t tr;
	UTIL_TraceLine(origin, origin + forw * MAX_TRACE_LENGTH, MASK_SHOT_HULL, pCam, COLLISION_GROUP_NONE, &tr);

	if(tra)
		*tra = tr;

	return tr.m_pEnt;
}

void COverlordConsoleManager::HandleStaticTrapHover()
{
	// Nullify static trap
	bool bWasVisible = (m_StaticTrap != NULL);
	m_StaticTrap = NULL;

	if(IsInBuildMode())
		return;

	const C_OverlordCamera * pCam = (GET_OVERLORD_DATA)->GetCamera();

	if(!pCam)
		return;

	Vector start = pCam->GetCameraVector();
	Vector forw;
	AngleVectors(pCam->GetCameraAngles(), &forw);

	Ray_t ray;
	ray.Init(start, start + forw * MAX_TRACE_LENGTH, -Vector(16, 16, 16), Vector(16, 16, 16));

	C_BaseEntity * pList[1024];
	int count = UTIL_EntitiesAlongRay(pList, ARRAYSIZE(pList), ray, 0);

	float dist = FLT_MAX;
	for(int i = 0; i < count; i++)
	{
		if(!pList[i])
			continue;

		// TODO: Fix this! Remove the dynamic cast somehow
		C_OverlordBaseModule * pModule = dynamic_cast<C_OverlordBaseModule*>(pList[i]);

		if(!pModule || !pModule->CanBeSelected())
			continue;

		float newDist = (pModule->GetAbsOrigin() - start).Length();
		if(dist > newDist)
		{
			dist = newDist;
			m_StaticTrap = pModule;
		}
		
	}

	if(!bWasVisible && m_StaticTrap)
		gViewPortInterface->ShowPanel(PANEL_STATICTRAPPANEL, true);
}

void COverlordConsoleManager::DoCameraJump()
{
	// Camera jumping
	trace_t tra;

	GetEntityUnderCursor(&tra);

	DoCameraJump(tra);
}

void COverlordConsoleManager::DoCameraJump(const trace_t & tra)
{
	// Cycle through entity list in search of cameras

	float lowest = -1.0f;
	C_OverlordCamera * pNext = NULL;

	if(!(tra.surface.flags & SURF_SKY))
	{
		for(int i = 0; i <= ClientEntityList().GetHighestEntityIndex(); i++)
		{
			C_BaseEntity * pEnt = ClientEntityList().GetBaseEntity(i);
			if(!pEnt)
				continue;

			C_OverlordCamera * pNew = dynamic_cast<C_OverlordCamera*>(pEnt);

			if(pNew && pEnt != GET_OVERLORD_DATA->GetCamera())
			{
				float distance = (tra.endpos - pNew->GetAbsOrigin()).Length();
				if(lowest == -1.0f || (distance < lowest))
				{
					trace_t tr;
					UTIL_TraceLine(tra.endpos + tra.plane.normal * 4, pNew->GetCameraVector(), MASK_SHOT_HULL, NULL, COLLISION_GROUP_NONE, &tr);

					if(tr.fraction >= 0.99f && !tr.allsolid)
					{
						pNext = pNew;
						lowest = distance;
					}
				}
			}
		}
	}

	if(pNext)
	{
		EHANDLE handle = pNext;
		
		// We need this to fix a lag issue!
		m_bFreeroam = false;

		engine->ServerCmd(VarArgs("SwitchToCamera %i", handle.GetEntryIndex()));
	}
}

void COverlordConsoleManager::ClickOnTrap(C_OverlordTrap * pTrap, bool bRightMouseButton)
{
	if(!bRightMouseButton)
	{
		if(pTrap->CanBeDormant())
		{
			int dormant = pTrap->IsTrapDormant() ? 0 : 1;
			EHANDLE handle = pTrap;

			// Initial dormant traps should be switched into manual dormant
			if(pTrap->IsInInitialDormant())
			{	
				// HACK! Disable dormant first, then enable it again!
				engine->ServerCmd(VarArgs("SetTrapDormant 0 %i", handle.GetEntryIndex()));
				dormant = 1;
			}

			// Client side dormant
			//pTrap->SetTrapDormant(!pTrap->IsTrapDormant());
			C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
			pPlayer->EmitAmbientSound(pPlayer->entindex(), pPlayer->EyePosition(), "HL2Player.Use");

			engine->ServerCmd(VarArgs("SetTrapDormant %i %i", dormant, handle.GetEntryIndex()));
		}
	}
	else
	{
		int index = pTrap->GetTrapIndex();
		engine->ServerCmd(VarArgs("DecayTrap %i", index));
	}
}


CON_COMMAND(eo_poweruppanel_show, "")
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer || !pPlayer->IsOverlord())
		return;

	COverlordData * pData = GetOverlordData();

	if(!pData || !pData->IsInVehicle())
		return;

	g_ConsoleManager.SetPowerupPanelOpen(true);

}