//==============Overlord mod=====================
//	Overlord's camera
//	
//===============================================

#include "cbase.h"
#include "overlord_camera.h"
#include "overlord_area.h"
#include "overlord_data.h"
#include "hl2mp_gamerules.h"
#include "overlord_basemodule.h"
#include "in_buttons.h"

#include "tier0/vprof.h"

#define RADIUS 0
#define CAMERA_THINK 0.1f



BEGIN_DATADESC(COverlordCamera)

	DEFINE_THINKFUNC(Think),

	DEFINE_KEYFIELD(m_iszAreaName,	 FIELD_STRING, "AreaOverride"), 
	DEFINE_KEYFIELD(m_iszDesc,		 FIELD_STRING, "Description"), 

	DEFINE_KEYFIELD(m_iszPrevCamera, FIELD_STRING, "PreviousCamera"),
	DEFINE_KEYFIELD(m_iszNextCam, FIELD_STRING, "NextCamera"),
	DEFINE_KEYFIELD(m_iszUpperCam, FIELD_STRING, "UpperCamera"),
	DEFINE_KEYFIELD(m_iszLowerCam, FIELD_STRING, "LowerCamera"),
	DEFINE_KEYFIELD(m_iszOriginBrush, FIELD_STRING, "OriginBrush"),

	DEFINE_KEYFIELD(m_iszList[0], FIELD_STRING, "List01"),
	DEFINE_KEYFIELD(m_iszList[1], FIELD_STRING, "List02"),
	DEFINE_KEYFIELD(m_iszList[2], FIELD_STRING, "List03"),
	DEFINE_KEYFIELD(m_iszList[3], FIELD_STRING, "List04"),
	DEFINE_KEYFIELD(m_iszList[4], FIELD_STRING, "List05"),
	DEFINE_KEYFIELD(m_iszList[5], FIELD_STRING, "List06"),
	DEFINE_KEYFIELD(m_iszList[6], FIELD_STRING, "List07"),
	DEFINE_KEYFIELD(m_iszList[7], FIELD_STRING, "List08"),

	DEFINE_TURRET(),

	DEFINE_INPUTFUNC(FIELD_VOID, "ExitConsole", InputExit),         
	DEFINE_INPUTFUNC(FIELD_VOID, "SwitchToThis", InputSwitch),  
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),  
	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "SetPreviousCamera", SetPreviousCamera),

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

END_DATADESC()

LINK_ENTITY_TO_CLASS(point_overcamera, COverlordCamera);

IMPLEMENT_SERVERCLASS_ST(COverlordCamera, DT_OverlordCamera)

	SendPropArray3( SENDINFO_ARRAY3(m_lLists), SendPropEHandle( SENDINFO_ARRAY(m_lLists) ) ),
	SendPropInt(SENDINFO(m_CurrentList)),
	SendPropStringT(SENDINFO(m_iszDesc) ),
	SendPropVector(SENDINFO(m_vOffset), SPROP_COORD),
	SendPropEHandle(SENDINFO(m_pNext) ),
	SendPropEHandle(SENDINFO(m_pPrevious) ),
	SendPropEHandle(SENDINFO(m_pUpper)),
	SendPropEHandle(SENDINFO(m_pLower)),
	SendPropEHandle(SENDINFO(m_pArea)),

END_SEND_TABLE()

COverlordCamera::COverlordCamera()
{ 
	m_bEnabled = true; 
	m_CurrentList.Set(FIRST_LIST); 

	m_flLastMove = 0.0f;
	m_flFirstMove = 0.0f;
}

COverlordCamera::~COverlordCamera()
{
}

void COverlordCamera::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/editor/camera.mdl");
}

void COverlordCamera::Spawn()
{
	Precache();

	SetSolid(SOLID_BBOX);

	AddSolidFlags( FSOLID_NOT_SOLID );

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetThink(&COverlordCamera::Think);
	
	SetNextThink(gpGlobals->curtime + CAMERA_THINK);

	SetModel("models/editor/camera.mdl");

	AddEffects(EF_NOSHADOW);
	AddEffects(EF_NORECEIVESHADOW);

	if(HasSpawnFlags(SF_START_DISABLED))
		Disable();

	CREATE_TURRET();

	// Create our offset for the client
	m_vOffset.GetForModify() = GetCameraVector() - GetAbsOrigin();

}


void COverlordCamera::Activate()
{
	BaseClass::Activate();

	HandleLists();

	HandleCameras();

	m_pArea = GetNearestArea();

	// Find the origin brush
	CBaseEntity * pEnt = NULL;

	if(m_iszOriginBrush)
		pEnt = gEntList.FindEntityByNameNearest(m_iszOriginBrush, GetAbsOrigin(), 0);

	if(pEnt)
	{
		SetLocalOrigin(pEnt->GetAbsOrigin());
	}

}

void COverlordCamera::Think()
{
	VPROF_BUDGET( "COverlordCamera::Think", VPROF_BUDGETGROUP_GAME );

	COverlordData * pData = GET_OVERLORD_DATA;
	
	if(pData->GetCamera() == this && pData->GetOverlord())
	{
		CBasePlayer * pPlayer = pData->GetOverlord();
		
		int nButtons = pPlayer->m_nButtons;
		//int afButtons = pPlayer->m_afButtonPressed;

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

		 if(nButtons & IN_FORWARD)
			 MoveUp();
		 if(nButtons & IN_BACK)
			 MoveDown();
		 if(nButtons & IN_MOVELEFT)
			 RotateLeft();
		 if(nButtons & IN_MOVERIGHT)
			 RotateRight();
	}

	SetNextThink(gpGlobals->curtime + CAMERA_THINK);
}

void COverlordCamera::HandleLists()
{
	for(int i = 0; i < NUM_OF_LISTS; i++)
	{
		CBaseEntity * pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszList[i]);

		if(!pEnt)
			continue;

		// Assume that the mapper is clever enough to select
		// a module list
		COverlordModuleList * pList = static_cast<COverlordModuleList*>(pEnt);

		if(!pList)
			continue;

		m_lLists.Set(i, pList);
	}
}

void COverlordCamera::HandleCameras()
{
	CBaseEntity * pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszNextCam);

	if(pEnt)
	{

		// Static cast, mapper is responsible for any crashes
		COverlordCamera * pCam = static_cast<COverlordCamera*>(pEnt);

		//if(pCam)
		//{
			m_pNext = pCam;

			// We are this camera's previous camera

			if(!m_pNext->GetPreviousCamera())
				m_pNext->SetPreviousCamera(this);
		//}
	}

	pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszPrevCamera);

	if(pEnt)
	{
		// same as above
		COverlordCamera * pCam = static_cast<COverlordCamera*>(pEnt);
		//if(pCam)
		//{
			m_pPrevious = pCam;
		//}
	}

	pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszUpperCam);

	if(pEnt)
	{
		COverlordCamera * pCam = static_cast<COverlordCamera*>(pEnt);

		m_pUpper = pCam;

		pCam->SetLowerCamera(this);
	}

	pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), m_iszLowerCam);

	if(pEnt)
	{
		COverlordCamera * pCam = static_cast<COverlordCamera*>(pEnt);

		m_pLower = pCam;

		pCam->SetUpperCamera(this);
	}
}

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

void COverlordCamera::ResetTurret()
{
	if(HasSpawnFlags(SF_DONT_REVERT_ANGLES))
		return;

	COverlordBaseTurret::ResetTurret();
}

void COverlordCamera::SetPreviousCamera(inputdata_t &inputData)
{
	if(m_pNext)
	{
		Warning("Ignoring SetPreviousCamera, next camera is specified, use OverridePreviousCamera at your own risk\n");
		return;
	}

	CBaseEntity * pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), inputData.value.String());

	if(!pEnt)
		return;

	m_pPrevious = static_cast<COverlordCamera*>(pEnt);
}
 void COverlordCamera::OverrideNextCamera(inputdata_t &inputData)
 {
	CBaseEntity * pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), inputData.value.String());

	if(!pEnt)
	{
		m_pNext = NULL;
		return;
	}

	m_pNext = static_cast<COverlordCamera*>(pEnt);
 }
 void COverlordCamera::OverridePreviousCamera(inputdata_t &inputData)
 {
	 CBaseEntity * pEnt = gEntList.FindEntityByName(gEntList.FirstEnt(), inputData.value.String());

	if(!pEnt)
	{
		m_pPrevious = NULL;
		return;
	}

	m_pPrevious = static_cast<COverlordCamera*>(pEnt);
 }


COverlordArea * COverlordCamera::GetNearestArea() const
{
	CBaseEntity * pEnt = NULL;
	COverlordArea * pArea = NULL;

	if(m_iszAreaName != MAKE_STRING("\0") && m_iszAreaName != NULL_STRING  )
	{
		pEnt = gEntList.FindEntityByName(pEnt, m_iszAreaName);

		if(pEnt)
		{
			pArea = dynamic_cast<COverlordArea*>(pEnt);
			if(pArea)
				return pArea;	

			pArea = NULL;
		}
		pEnt = NULL;
	}

	pEnt = gEntList.FindEntityByClassnameNearest("func_overarea", GetAbsOrigin(), RADIUS);

	if(!pEnt)
		return NULL;

	// Switch to dynamic_cast in case of problems
	pArea = static_cast<COverlordArea*>(pEnt);

	//if(!pArea)
		//return NULL;

	return pArea;
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

void COverlordCamera::ListForward()
{ 
	if(m_CurrentList + 1 > LAST_LIST)
		m_CurrentList = FIRST_LIST;
	else if(!m_lLists[m_CurrentList + 1])
		m_CurrentList = FIRST_LIST;
	else
		m_CurrentList++;
}

void COverlordCamera::ListBackward()
{ 
	if(m_CurrentList - 1 < FIRST_LIST)
		m_CurrentList = GetLastListNumber();
	else if(!m_lLists[m_CurrentList - 1])
		m_CurrentList = GetLastListNumber();
	else
		m_CurrentList--;
}

int COverlordCamera::GetLastListNumber() const
{
	for(int i = LAST_LIST; i > FIRST_LIST; i--)
	{
		if(m_lLists[i])
			return i;
	}

	return FIRST_LIST;
}

float COverlordCamera::GetMoveSpeed() const
{	
	const float baseSpeed = COverlordBaseTurret::GetMoveSpeed();

	if(!HasSpawnFlags(SF_NOSPEEDCHANGES))
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
