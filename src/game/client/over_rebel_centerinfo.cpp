/*
	Center bars for Overlordess mod
*/

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HEALTH_CRITICAL 25 //Red blink
#define HEALTH_LOW 60 //Yellow bar colour

static ConVar eo_cl_rebel_newhud("eo_cl_rebel_newhud", "1", FCVAR_ARCHIVE);

extern ConVar crosshair;

using namespace vgui;

class RHudCenterInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(RHudCenterInfo, vgui::Panel);
		
		public:
	
	RHudCenterInfo(const char *pElementName);
	void Init();
	void VidInit();
	virtual void OnThink();
	virtual void Paint();
	bool ShouldDraw();
	virtual void ApplySchemeSettings(IScheme *scheme);
	//bool UpdateEventTime();
	//bool EventTimeElapsed();
		private:

	int m_health, m_healthPrevious, m_ammo, m_ammoPrevious;
	bool m_isHealthLow, m_isHealthCritical, m_isAmmoCritical;
	
	CHudTexture *m_barHealth;
	CHudTexture *m_barHealthGreen;
	
	CHudTexture *m_crosshair;
	CHudTexture *m_ammobar;
	CHudTexture *m_ammobarEmpty;
		
};

//DECLARE_HUDELEMENT(RHudCenterInfo);

RHudCenterInfo::RHudCenterInfo(const char *pElementName):CHudElement(pElementName), BaseClass(NULL, "RHudCenterInfo")
{
		vgui::Panel *pParent = g_pClientMode->GetViewport();
		SetParent(pParent);
		SetHiddenBits(HIDEHUD_CROSSHAIR | HIDEHUD_CAMERA | HIDEHUD_REBEL | HIDEHUD_PLAYERDEAD);
}

void RHudCenterInfo::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

void RHudCenterInfo::Init()
{
	m_health = 100;
	m_healthPrevious = 0;
	m_ammo = 0;
	m_ammoPrevious = 0;
	m_isHealthLow = false;
	m_isHealthCritical = false;
	m_isAmmoCritical = false;
}

void RHudCenterInfo::VidInit()
{
	Init();
	m_crosshair = gHUD.GetIcon("crosshair");
	
	if(m_crosshair == NULL)
		return;

	m_barHealth = gHUD.GetIcon("over_healthbar");
	m_barHealthGreen = gHUD.GetIcon("over_healthbarGreen");
			
	if(m_barHealth == NULL || m_barHealthGreen == NULL)
		return;

	m_ammobar = gHUD.GetIcon("over_ammobar");
	m_ammobarEmpty = gHUD.GetIcon("over_ammobarEmpty");

	if(m_ammobar == NULL || m_ammobarEmpty == NULL)
		return;
}

bool RHudCenterInfo::ShouldDraw()
{
	if(!m_crosshair || !m_barHealth)
		return false;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if(player == null)
			return false;
	
	if(!crosshair.GetBool())
		return false;

	return (CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage());


}

void RHudCenterInfo::OnThink()
{
	BaseClass::OnThink();

	if(m_crosshair == NULL || m_barHealth == NULL || m_ammobar == NULL){
		return;
	}

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( player == NULL ){
		return;
	}
}

void RHudCenterInfo::Paint()
{
	int xCen = 0;
	int yCen = 0;

	float healthPerc;
	float ammoPerc;

	ISchemeManager* schem = vgui::scheme();

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if(player==NULL)
			return;

	C_BaseCombatWeapon *playerWeapon = GetActiveWeapon(); 
		if(playerWeapon==NULL)
			return;

	Color clrNormal = gHUD.m_clrNormal;

	xCen = (ScreenWidth() - m_crosshair->Width())/2;
	yCen = (ScreenHeight() - m_crosshair->Height())/2;

	int hp = player->GetHealthOverride() > 0 ? player->GetHealthOverride() : player->GetHealth();
	int ammo = 0;
	int maxAmmo = 0;

    int tHeight = (schem->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_barHealth->Height())/2);
	int tWidth = (schem->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_barHealth->Width())/2);

		
	if(playerWeapon->Clip1() < 0)
	{
		ammo = player->GetAmmoCount(playerWeapon->GetPrimaryAmmoType());
		maxAmmo = GetAmmoDef()->MaxCarry( GetAmmoDef()->Index("Grenade") );
	}
	else
	{
		ammo = playerWeapon->Clip1();
		maxAmmo = playerWeapon->GetMaxClip1();
	}

//====HEALTH BAR====
	healthPerc = (((float)hp)/100.0f);
	healthPerc = clamp( healthPerc, 0.0f, 1.0f );
	
		xCen = (ScreenWidth() - tWidth)/2;
		yCen = ((ScreenHeight() - tHeight)/2);

	gHUD.DrawIconProgressBar((xCen + m_barHealthGreen->Width()), (yCen), m_barHealthGreen, m_barHealth, (1.0f - healthPerc), clrNormal, CHud::HUDPB_VERTICAL, tWidth, tHeight);
	
//====AMMO BAR====
	
	if(maxAmmo == 0)
	{
		ammoPerc = 0.0f;
	}
	else
	{
		ammoPerc = ( (float)ammo / (float) maxAmmo );
		ammoPerc = clamp( ammoPerc, 0.0f, 1.0f );
	}

	gHUD.DrawIconProgressBar((xCen - m_ammobar->Width()), yCen, m_ammobarEmpty, m_ammobar, (1.0f - ammoPerc), clrNormal, CHud::HUDPB_VERTICAL, tWidth, tHeight);

}