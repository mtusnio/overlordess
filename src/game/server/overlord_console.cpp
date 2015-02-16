//==============Overlord mod=====================
//	Overlord's console, should be also declared
//	client-side.
//===============================================

#include "cbase.h"
#include "overlord_console.h"
#include "overlord_camera.h"
#include "overlord_freeroam.h"
#include "hl2mp_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FLAG_CONSOLE_OVERRWITE 1

BEGIN_DATADESC(COverlordConsole)

	DEFINE_KEYFIELD(m_iszCamera, FIELD_STRING, "Camera"),
	DEFINE_USEFUNC(UseConsole),
	DEFINE_INPUTFUNC(FIELD_VOID, "Lock", InputLock),
	DEFINE_INPUTFUNC(FIELD_VOID, "Unlock", InputUnlock),

END_DATADESC()

LINK_ENTITY_TO_CLASS(func_overconsole, COverlordConsole);

IMPLEMENT_SERVERCLASS_ST(COverlordConsole, DT_OverlordConsole)
	SendPropBool(SENDINFO(m_bEnabled)),
	SendPropString(SENDINFO(m_iszArea)),
END_SEND_TABLE()



COverlordConsole::COverlordConsole()
{
	m_bEnabled = true;
}

COverlordConsole::~COverlordConsole()
{
}

void COverlordConsole::Spawn()
{
	Precache();

	SetSolid(SOLID_VPHYSICS);

	SetMoveType(MOVETYPE_PUSH);

	SetUse(&COverlordConsole::UseConsole);

	SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);

	SetModel(STRING(GetModelName()));

	CreateVPhysics();

}

bool COverlordConsole::CreateVPhysics()
{
	VPhysicsInitShadow(false, false);

	return true;
}

void COverlordConsole::Activate()
{
	
	ResolveNames();

	//m_iszArea = LinkToArea();

	BaseClass::Activate();
}

int	COverlordConsole::ObjectCaps(void)
{
	return (BaseClass::ObjectCaps() | FCAP_IMPULSE_USE);
}

void COverlordConsole::UseConsole(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	//DevMsg("Console used\n");

	if(!m_bEnabled)
		return;

	if(!m_pCamera)
		return;

	CHL2MP_Player * pPlayer = ToHL2MPPlayer(pCaller);
	
	// Only player can use
	if(!pPlayer)
		return;
	
	if(!pPlayer->IsOverlord())
		return;

	//pPlayer->SuspendUse(1.0f);
	//pPlayer->FlashlightTurnOff();

	(GET_OVERLORD_DATA)->SwitchToCamera(m_pCamera);
	(GET_OVERLORD_DATA)->SetConsole(this);
	
}

void COverlordConsole::ResolveNames()
{
	CBaseEntity * pEnt = NULL;
	
	pEnt = gEntList.FindEntityByName(pEnt, m_iszCamera);

	if(!pEnt)
		return;

	m_pCamera = dynamic_cast<COverlordCamera*>(pEnt);
	
	if(!m_pCamera)
	{
		Warning("Camera not resolved!\n");
		m_pCamera = NULL;
		m_iszCamera = NULL_STRING;
	}
}


void COverlordConsole::OverwriteCamera(COverlordCamera * pCam)
{
	m_pCamera = pCam;
}

void COverlordConsole::UpdateCamera(COverlordCamera * pCam)
{
	if(HasSpawnFlags(FLAG_CONSOLE_OVERRWITE))
	{
		OverwriteCamera(pCam);
	}
}

void COverlordConsole::InputLock(inputdata_t &data)
{
	m_bEnabled = false;
}

void COverlordConsole::InputUnlock(inputdata_t &data)
{
	m_bEnabled = true;
}