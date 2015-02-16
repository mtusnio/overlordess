//==============Overlord mod=====================
//	Overlord's camera
//	
//===============================================

#include "cbase.h"
#include "overlord_camera.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"

#ifndef CLIENT_DLL
#include "overlord_area.h"
#include "overlord_basemodule.h"
#else
#include "c_overlord_basemodule.h"
#include "c_overlord_area.h"
#include "over_popupsupport.h"
#include "overlord_consolemanager.h"
#include "vgui/IInput.h"
#include "baseviewport.h"
#include "ienginevgui.h"
#endif

#include "tier0/vprof.h"

#define RADIUS 0

#ifdef CLIENT_DLL
ConVar eo_predict_cameras("eo_predict_cameras", "0");
#endif

BEGIN_DATADESC(COverlordCamera)
#ifndef CLIENT_DLL
	DEFINE_KEYFIELD(m_iszAreaName,	 FIELD_STRING, "AreaOverride"), 
	DEFINE_KEYFIELD(m_iszDesc,		 FIELD_STRING, "Description"), 

	DEFINE_KEYFIELD(m_iszPrevCamera, FIELD_STRING, "PreviousCamera"),
	DEFINE_KEYFIELD(m_iszNextCam, FIELD_STRING, "NextCamera"),
	DEFINE_KEYFIELD(m_iszUpperCam, FIELD_STRING, "UpperCamera"),
	DEFINE_KEYFIELD(m_iszLowerCam, FIELD_STRING, "LowerCamera"),

	DEFINE_KEYFIELD(m_iszList, FIELD_STRING, "List01"),

	DEFINE_TURRET(),

	DEFINE_INPUTFUNC(FIELD_VOID, "ExitConsole", InputExit),         
	DEFINE_INPUTFUNC(FIELD_VOID, "SwitchToThis", InputSwitch),  
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),  
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

	/*DEFINE_OUTPUT (m_OnButtonPressed[0], "OnButton01Pressed"),      
	DEFINE_OUTPUT (m_OnButtonPressed[1], "OnButton02Pressed"),      
	DEFINE_OUTPUT (m_OnButtonPressed[2], "OnButton03Pressed"),      
	DEFINE_OUTPUT (m_OnButtonPressed[3], "OnButton04Pressed"),      
	DEFINE_OUTPUT (m_OnButtonPressed[4], "OnButton05Pressed"),      
	DEFINE_OUTPUT (m_OnButtonPressed[5], "OnButton06Pressed"),*/

	DEFINE_OUTPUT(m_OnUsePressed, "OnUsePressed"),
	DEFINE_OUTPUT(m_OnForwardPressed, "OnForwardPressed"),
	DEFINE_OUTPUT(m_OnBackPressed, "OnBackPressed"),
	DEFINE_OUTPUT(m_OnRightPressed, "OnRightPressed"),
	DEFINE_OUTPUT(m_OnLeftPressed, "OnLeftPressed"),
	DEFINE_OUTPUT(m_OnJumpPressed, "OnJumpPressed"),
	DEFINE_OUTPUT(m_OnReloadPressed, "OnReloadPressed"),
	DEFINE_OUTPUT(m_OnDuckPressed, "OnDuckPressed"),
	DEFINE_OUTPUT(m_OnEnter, "OnEnter"),
	DEFINE_OUTPUT(m_OnExit, "OnExit"),
#endif
END_DATADESC()


LINK_ENTITY_TO_CLASS(point_overcamera, COverlordCamera);

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordCamera, DT_OverlordCamera);

BEGIN_NETWORK_TABLE(COverlordCamera, DT_OverlordCamera)

#ifndef CLIENT_DLL
	SendPropInt(SENDINFO(m_CurrentList)),
	SendPropStringT(SENDINFO(m_iszDesc) ),

	SendPropInt(SENDINFO(m_spawnflags)),

	SendPropFloat(SENDINFO(m_flFirstMove)),
	SendPropFloat(SENDINFO(m_flLastMove)),
#else
	RecvPropInt(RECVINFO(m_CurrentList) ),
	RecvPropString(RECVINFO(m_iszDesc) ),

	RecvPropInt(RECVINFO(m_spawnflags)),

	RecvPropFloat(RECVINFO(m_flFirstMove)),
	RecvPropFloat(RECVINFO(m_flLastMove)),
#endif

END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(C_OverlordCamera)

	DEFINE_PRED_FIELD_TOL( m_angRotation, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.002f),
	DEFINE_PRED_FIELD_TOL( m_angAbsRotation, FIELD_VECTOR,  FTYPEDESC_INSENDTABLE, 0.002f),
	DEFINE_PRED_FIELD_TOL( m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.002f),

	DEFINE_PRED_FIELD_TOL( m_flFirstMove, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	DEFINE_PRED_FIELD_TOL( m_flLastMove, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),

END_PREDICTION_DATA()
#endif

bool COverlordCamera::m_MouseRotation = true;

/*		Static stuff		*/
void COverlordCamera::SetMouseRotationEnabled(bool bEnabled)
{
	m_MouseRotation = bEnabled;
#ifdef CLIENT_DLL
	if(bEnabled)
		engine->ServerCmd("EnableMouseRotation");
	else
		engine->ServerCmd("DisableMouseRotation");
#endif
}




COverlordCamera::COverlordCamera()
{ 
#ifndef CLIENT_DLL
	m_bEnabled = true; 
	m_CurrentList.Set(FIRST_LIST); 
#endif
	m_flLastMove = 0.0f;
	m_flFirstMove = 0.0f;
	m_flSpeedOverrideX = 0.0f;
	m_flSpeedOverrideY = 0.0f;

	SetPredictionEligible(true);
}


COverlordCamera::~COverlordCamera()
{
}

#ifdef CLIENT_DLL
bool COverlordCamera::ShouldPredict()
{
	if(C_BasePlayer::GetLocalPlayer()->IsOverlord())
		return true;

	return BaseClass::ShouldPredict();
}
#endif

void COverlordCamera::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/editor/camera.mdl");
}

#ifdef CLIENT_DLL
int COverlordCamera::DrawModel(int flags)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer || !pPlayer->IsOverlord())
		return 0;

	C_OverlordData * pData = GET_OVERLORD_DATA;

	if(!pData || !pData->IsInVehicle() || pData->GetCamera() == this)
		return 0;

	return BaseClass::DrawModel(flags);
}
#endif


void COverlordCamera::Spawn()
{
	BaseClass::Spawn();

	Precache();
#ifndef CLIENT_DLL
	SetSolid(SOLID_BBOX);

	AddSolidFlags( FSOLID_NOT_SOLID );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetModel("models/editor/camera.mdl");

	AddEffects(EF_NOSHADOW);
	AddEffects(EF_NORECEIVESHADOW);

	if(HasSpawnFlags(SF_START_DISABLED))
		Disable();
#endif
	CREATE_TURRET();
}

#ifndef CLIENT_DLL
void COverlordCamera::InputExit(inputdata_t &inputData)
{
	GET_OVERLORD_DATA_ASSERT(pData);
	
	if(pData->GetCamera() == this)
		pData->SwitchFromCamera();

}

void COverlordCamera::InputSwitch(inputdata_t &inputData)
{
	GET_OVERLORD_DATA_ASSERT(pData);

	pData->SwitchToCamera(this);
}
#endif

void COverlordCamera::ResetTurret()
{
	if(HasSpawnFlags(SF_DONT_REVERT_ANGLES))
		return;

	COverlordBaseTurret::ResetTurret();
}


#ifndef CLIENT_DLL

void COverlordCamera::FireOnModule(int module)
{
	COverlordBaseModule * pModule = static_cast<COverlordBaseModule*>(gEntList.GetNetworkableHandle(module).Get());

	if(!pModule)
		return;

	pModule->InputActivate(inputdata_t());	
}



void COverlordCamera::Enable()
{
	if(IsEnabled())
		return;

	m_bEnabled = true;
}

void COverlordCamera::Disable()
{
	if(!IsEnabled())
		return;

	m_bEnabled = false;
}

#endif

float COverlordCamera::GetMoveSpeed() const
{	
	const float baseSpeed = GetSpeedPerSecond() * CAMERA_THINK;

	if(UsesSpeedChanges())
	{
		const float DIFF_SLOWDOWN = 0.40f;
		const float DIFF_SPEEDUP = DIFF_SLOWDOWN + 1.250f;

		if(m_flFirstMove + DIFF_SLOWDOWN > gpGlobals->curtime)
		{
			// Percentage of the difference between curtime
			// and first move
			//float percent = (DIFF - ((flFirstMove + DIFF) - gpGlobals->curtime))/DIFF;
			float percent = (gpGlobals->curtime - m_flFirstMove)/DIFF_SLOWDOWN;

			// We need to make sure now that
			// we don't end up with speed being nearly 0
			// We square root the percent to acquire faster speed rate
			//float tmp = baseSpeed * FastSqrt(percent);
			float tmp = baseSpeed * percent;

			float speed = tmp > 0.25 ? tmp : 0.25;

			return speed;
		}
		else if(m_flFirstMove + DIFF_SPEEDUP <= gpGlobals->curtime)
		{
			float treshold = m_flFirstMove + DIFF_SPEEDUP * 2;

			float percent;

			if(treshold <= gpGlobals->curtime)
				 percent = 1.0f;
			else
			{
				percent = FastSqrt(1.0f - (treshold - gpGlobals->curtime)/DIFF_SPEEDUP);
			}

			return (baseSpeed * percent) + baseSpeed;

		}
	}
	
	return baseSpeed;

}

void COverlordCamera::OnCameraEnter()
{
#ifdef CLIENT_DLL

#endif
}

void COverlordCamera::OnCameraExit()
{
#ifdef CLIENT_DLL

#endif
}

#ifdef CLIENT_DLL
void  COverlordCamera::HandleKeyboardInput()
{
	static float flUseDelay = 0.0f;

	if(flUseDelay > gpGlobals->curtime)
		return;

	if(enginevgui->IsGameUIVisible())
		return;

	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	int module = -1;

	if(pPlayer->m_nButtons & IN_USE)
	{
		C_OverlordBaseModule * pModule = g_ConsoleManager.GetHoverStaticTrap();

		if(pModule)
		{
			module = pModule->entindex();
		}
	}
	if(module != -1)
	{
		flUseDelay = gpGlobals->curtime + 0.25f;
		engine->ServerCmd(VarArgs("eo_activate_module %i", module));
	}
}
#endif

bool  COverlordCamera::IsInMouseRotation() const
{
	if(GetOverlordData()->GetBuildMode() == BL_ROTATE)
		return false;

	return m_MouseRotation;
}

void COverlordCamera::ProcessMovement(const CUserCmd * cmd) 
{
	int nButtons = cmd->buttons;
	//int afButtons = pPlayer->m_afButtonPressed;

#ifdef CLIENT_DLL
	HandleKeyboardInput();

#else
	// Events
	if(nButtons & IN_FORWARD)
	FireOnForward();
	if(nButtons & IN_BACK)
	FireOnBack();
	if(nButtons & IN_MOVELEFT)
	FireOnLeft();
	if(nButtons & IN_MOVERIGHT)
	FireOnRight();
	if(nButtons & IN_JUMP)
	FireOnJump();
	if(nButtons & IN_DUCK)
	FireOnDuck();
	if(nButtons & IN_RELOAD)
	FireOnReload();
#endif
	if(!IsInMouseRotation())
	{
		m_flSpeedOverrideX = 0.0f;
		m_flSpeedOverrideY = 0.0f;

		if(nButtons & IN_FORWARD)
			MoveUp();
		if(nButtons & IN_BACK)
			MoveDown();
		if(nButtons & IN_MOVELEFT)
			RotateLeft();
		if(nButtons & IN_MOVERIGHT)
			RotateRight();
	}
	else
	{
		m_flSpeedOverrideX = (float)cmd->mousedx * MOUSE_SPEED;
		m_flSpeedOverrideY = (float)cmd->mousedy * MOUSE_SPEED;

		if(m_flSpeedOverrideX < 0)
			m_flSpeedOverrideX = -m_flSpeedOverrideX;

		if(m_flSpeedOverrideY < 0)
			m_flSpeedOverrideY = -m_flSpeedOverrideY;

		if(cmd->mousedx < 0)
			RotateLeft();
		else if(cmd->mousedx > 0)
			RotateRight();

		if(cmd->mousedy < 0)
			MoveUp();
		else if(cmd->mousedy > 0)
			MoveDown();
	}
	
}

