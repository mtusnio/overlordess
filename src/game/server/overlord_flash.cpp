//==============Overlord mod=====================
//	Flash that blinds players
//	
//===============================================

#include "cbase.h"
#include "overlord_flash.h"
#include "sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FLASH_THINK 0.1f
#define FLASH_RADIUS m_flRadius
#define FLASH_TIME   5.0f
#define FLASH_HOLD	 3.0f
#define FLASH_SOUND m_szSoundOn
#define FLASH_SOUND_OFF m_szSoundOff

#define SF_NO_SPRITE 2

BEGIN_DATADESC(COverlordFlash)

	
	DEFINE_THINKFUNC(Think),

	DEFINE_KEYFIELD(m_szSoundOn, FIELD_SOUNDNAME, "FlashOnSound"),
	DEFINE_KEYFIELD(m_szSoundOff, FIELD_SOUNDNAME, "FlashOffSound"),
	DEFINE_KEYFIELD(m_flRadius, FIELD_FLOAT, "FlashRadius"),
	DEFINE_KEYFIELD(m_Alpha, FIELD_INTEGER, "FlashAlpha"),
	//DECLARE_MODULE(),

END_DATADESC()

LINK_ENTITY_TO_CLASS(point_overflash, COverlordFlash);

COverlordFlash::COverlordFlash()
{

}

COverlordFlash::~COverlordFlash()
{
	if(m_pFlash)
		UTIL_Remove(m_pFlash);

	m_pFlash = NULL;
}

void COverlordFlash::Spawn()
{
	BaseClass::Spawn();

	Precache();

	SetThink(&COverlordFlash::Think);

	SetNextThink(gpGlobals->curtime + FLASH_THINK);
}
void COverlordFlash::Activate()
{
	BaseClass::Activate();

	if(!HasSpawnFlags(SF_NO_SPRITE))
	{
		m_pFlash = CSprite::SpriteCreate( "sprites/animglow01.vmt", GetLocalOrigin(), FALSE );
		m_pFlash->SetTransparency( kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation );
		m_pFlash->SetAttachment( this, -1);
		m_pFlash->SetBrightness( 0 );
		m_pFlash->SetScale( 1.4 );
	}
}

void COverlordFlash::Precache()
{
	BaseClass::Precache();

	if(FLASH_SOUND)
		PrecacheScriptSound(FLASH_SOUND);

	if(FLASH_SOUND_OFF)
		PrecacheScriptSound(FLASH_SOUND_OFF);

	PrecacheModel("models/editor/axis_helper_thick.mdl");
}

void COverlordFlash::Think()
{
	BaseClass::Think();

	if(IsActivated())
		RunModule();

	SetNextThink(gpGlobals->curtime + FLASH_THINK);
}

void COverlordFlash::InputActivate(inputdata_t &inputData)
{
	if(!CanActivate())
		return;

	BaseClass::InputActivate(inputData);

	if(m_pFlash)
	{
		m_pFlash->SetScale( 1.4 );
		m_pFlash->SetBrightness( 255 );
		m_pFlash->SetColor(255,255,255);
	}

	if(FLASH_SOUND)
		EmitSound(FLASH_SOUND);

	SetNextThink(gpGlobals->curtime + FLASH_THINK);

}


void COverlordFlash::Deactivate()
{
	BaseClass::Deactivate();
	
	if(m_pFlash)
	{
		m_pFlash->SetScale( 1.4 );
		m_pFlash->SetBrightness( 0 );
		m_pFlash->SetColor(255,255,255);
	}

	if(FLASH_SOUND_OFF)
		EmitSound(FLASH_SOUND_OFF);
	
	SetNextThink(TICK_NEVER_THINK);

}
void COverlordFlash::RunModule()
{
	// We cycle through player list as only players will be affected by this flash
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{	
		CBasePlayer * pPlayer = ToBasePlayer(UTIL_PlayerByIndex(i));

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive())
			continue;

		if((GetAbsOrigin() - pPlayer->RealEyePosition()).Length() > FLASH_RADIUS)
			continue;

		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), pPlayer->RealEyePosition(), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if(tr.m_pEnt != pPlayer)
			continue;
		
		if(pPlayer->FInViewCone(GetAbsOrigin()))
			Flash(pPlayer);
	}
}

void COverlordFlash::Flash(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return;

	color32 white = { 255, 255, 255, m_Alpha };
	UTIL_ScreenFade(pPlayer,white,FLASH_TIME,FLASH_HOLD, FFADE_IN);
	
	IGameEvent * event = gameeventmanager->CreateEvent( "eo_suppresshints" );

	if(event)
	{
		event->SetInt("userid", pPlayer->GetUserID());
		event->SetFloat("delay", 7.0f);

		gameeventmanager->FireEvent(event);
	}
}