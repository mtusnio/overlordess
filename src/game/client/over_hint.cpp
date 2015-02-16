//==============Overlord mod=====================
//	Displays charge, ammo etc. for the Overlord
//	and rebels
//===============================================


#include "cbase.h"
#include "over_hint.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "hl2mp_gamerules.h"
#include <vgui_controls/RichText.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(COverlordHint);

#define NEXT_UPDATE 2.8f
#define THRESHOLD 2

// This macro lets us control the way we handle the hint text
#define COPY_HINT(dest, text) Q_strncpy(dest, text, HINT_LENGTH)

COverlordHint::COverlordHint(const char * pElementName) :
BaseClass(NULL, "HudHint"),
CHudElement(pElementName)
{
	SetParent(g_pClientMode->GetViewport());

	m_RichText = new vgui::RichText(this, "HintRichText");

#ifdef SHOW_WEAPON_HINTS
	m_bWeaponFound = false;
#endif

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

COverlordHint::~COverlordHint()
{
	//SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	if(m_RichText)
	{
		delete m_RichText;
		m_RichText = NULL;
	}
}

void COverlordHint::Init()
{
	StartListening();

	Reset();
}

void COverlordHint::Reset()
{
	m_Hint.next = DEFAULT;
	m_Hint.current = DEFAULT;
	m_Hint.nextUpdate = gpGlobals->curtime + NEXT_UPDATE;

	// Reset the RichText
	m_RichText->SetText("");
	m_RichText->SetPos(TEXT_POS_X, TEXT_POS_Y);
	m_RichText->SetVerticalScrollbar(false);
	m_RichText->SetSize(GetWide() - THRESHOLD, GetTall() - THRESHOLD);

	UpdateDisplay(DEFAULT);
}

bool COverlordHint::ShouldDraw()
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
	
	if(!pPlayer)
		return CHudElement::ShouldDraw();

	// The hint bracket shouldn't be shown if there is nothing to show, which
	// applies to the overlord
	// OBSOLETE: Overlord also gets hints
#ifdef SHOW_WEAPON_HINTS
	//if(m_Hint.current == DEFAULT && pPlayer->IsRebel() && !WasWeaponFound())
	//	return false;

	if(m_Hint.current == DEFAULT 
		&& (pPlayer->IsOverlord() || pPlayer->GetPlayerClass() == CLASS_DEFAULT))
		return false;
#else
	if(m_Hint.current == DEFAULT)
		return false;	
#endif


	return CHudElement::ShouldDraw();
}

void COverlordHint::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	// Apply rich text scheme
	m_RichText->SetFont(pScheme->GetFont("DefaultVerySmall"));

	SetPaintBackgroundType(2);

	BaseClass::ApplySchemeSettings(pScheme);
}

// Due to some prediction errors etc. we must update
// this display each x seconds, otherwise we will
// be left with wrongly displayed mode
void COverlordHint::OnThink()
{
	UpdateDisplay();
}

// Decides what should be put here and paints it on the display
void COverlordHint::UpdateDisplay(EventType_t type, bool bUpdateNow)
{
	
#ifdef SHOW_WEAPON_HINTS
	// Reset weapon
	m_bWeaponFound = false;
#endif

	// Handle the future update, if current one differs from default
	if(type != DEFAULT)
	{
		if(m_Hint.current == DEFAULT)
		{
			// Just update, nothing serious
			m_Hint.current = type;
			bUpdateNow = true;
		}
		else
		{
			if(m_Hint.next == DEFAULT)
			{
				// Schedule it
				m_Hint.next = type;
			}
			else
			{
				// Immediately switch the display, this way we lose no events, although
				// at the cost of previous display
				m_Hint.JumpForward();
				m_Hint.next = type;
				bUpdateNow = true;
			}
		}
	}

	// Shall we update now?
	if((m_Hint.nextUpdate > gpGlobals->curtime) && !bUpdateNow)
		return;

	// Default got through at last, restart the hint
	if(m_Hint.current != DEFAULT && type == DEFAULT)
	{
		m_Hint.current = m_Hint.next;
		m_Hint.next = DEFAULT;
	}

	m_RichText->SetText("");
	char display[HINT_LENGTH];
#ifdef SHOW_WEAPON_HINTS
	if(m_Hint.current == DEFAULT && m_Hint.next == DEFAULT)
	{
		// Update with default strings
		const C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

		C_WeaponHL2MPBase * pClass = NULL;
		for(int i = 0; i < MAX_WEAPONS; i++)
		{
			if(!pPlayer)
				break;

			C_WeaponHL2MPBase * pWeapon = static_cast<C_WeaponHL2MPBase*>(pPlayer->GetWeapon(i));
	
			if(!pWeapon)
				continue;

			if(!pWeapon->IsClassWeapon() || !pWeapon->IsMainClassWeapon() || !pWeapon->IsActiveByLocalPlayer())
				continue;
			
				// Weapon found, break
			pClass = static_cast<C_WeaponHL2MPBase*>(pWeapon);		
			break;
		}

		char string[HINT_LENGTH];
		
		const char * primary = NULL;
		const char * secondary = NULL;

		// Assign pointers, should show "None" if weapon wasn't found
		if(pClass)
		{
			m_bWeaponFound = true;
			primary = pClass->GetPrimaryMode();
			secondary = pClass->GetSecondaryMode();
		}
		else
		{
			primary = "None";
			secondary = "None";
		}


		Q_snprintf(string, HINT_LENGTH, "Primary: %s\nSecondary: %s", primary, secondary);

		COPY_HINT(display, string);
		
		
		

	}
	else 
	#endif 
	if(m_Hint.current == DEFAULT && m_Hint.next != DEFAULT)
	{
		// This should never happen, read above
		COPY_HINT(display, "Current hint display is default, while the next is not. Switching...");
		m_Hint.JumpForward();
	}
	else if(m_Hint.current != DEFAULT)
	{
		switch(m_Hint.current)
		{
		case CUSTOM:
			COPY_HINT(display, m_Custom);
			break;
		case UNKNOWN:
			COPY_HINT(display, "Unknown event!");
			break;
		}
	}

	m_RichText->SetText(display);

	m_Hint.nextUpdate = gpGlobals->curtime + NEXT_UPDATE;
}

void COverlordHint::StartListening()
{
	// Example:
	//ListenForGameEvent("player_death");
	ListenForGameEvent("eo_customhint");
	ListenForGameEvent("eo_hint");
}

void COverlordHint::FireGameEvent(IGameEvent *event)
{
	if(!event)
		return;

	const char * type = event->GetName();

	if(Q_strcmp(type, "eo_customhint") == 0)
	{
		C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
		const int userid = event->GetInt("userid");

		// -1 userid means that everyone should get this hint
		if(!pPlayer || 	(userid != -1 && userid != pPlayer->GetUserID()))
			return;

		// It is for us, so display it
		// Copy the custom hint to the array and then update the display
		Q_strncpy(m_Custom, event->GetString("hint"), HINT_LENGTH);
		
		UpdateDisplay(CUSTOM);
	}
	else if(Q_strcmp(type, "eo_hint") == 0)
	{
		C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
		const int userid = event->GetInt("userid");

		// -1 userid means that everyone should get this hint
		if(!pPlayer || 	(userid != -1 && userid != pPlayer->GetUserID()))
			return;
		
		const int hint = event->GetInt("hint");
		
		// Make sure it is in the bounds of the enum
		EventType_t etype;
		if(hint > CUSTOM || hint < DEFAULT)
			etype = UNKNOWN;
		else
			etype = EventType_t(hint);
			

		UpdateDisplay(etype);
	}
}
