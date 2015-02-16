//==============Overlord mod=====================
//	Overlordess health display
//	
//===============================================

#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"
#include "hl2mp_gamerules.h"
#include "vguicenterprint.h"

using namespace vgui;

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "hudelement.h"
#include "hud_numericdisplay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define STATE_0 0
#define STATE_25 1
#define STATE_50 2
#define STATE_75 3
#define STATE_100 4

#define WARNING_SOUND ""

class COverlordHealth : public CHudElement, public CHudNumericDisplay
{
public:
	DECLARE_CLASS_SIMPLE(COverlordHealth, CHudNumericDisplay);

	COverlordHealth(const char *pElementName);
	virtual ~COverlordHealth() { };

	virtual void Init();
	virtual void Reset();
protected:
	virtual void OnThink();
	virtual void PaintNumbers(vgui::HFont font, int xpos, int ypos, int value);
private:
	void Notify(float percentage);

	int m_iState;

};


DECLARE_HUDELEMENT( COverlordHealth );

COverlordHealth::COverlordHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudOvHealth")
{
	m_iState = STATE_0;
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_REBEL | HIDEHUD_NEEDSUIT );
}

void COverlordHealth::Init()
{
	Reset();
}

void COverlordHealth::Reset()
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	// Restart our state only on initial spawn
	if(pPlayer && pPlayer->GetDeathsThisRound() <= 0)
	{
		m_iState = STATE_0;
	}

	wchar_t * displaystring = g_pVGuiLocalize->Find("#EO_OVHEALTH");
	
	if(displaystring)
	{
		SetLabelText(displaystring);
	}
	else
	{
		SetLabelText(L"OV HEALTH");
	}

	// Precache
/*	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(pPlayer)
	{
		pPlayer->PrecacheScriptSound(WARNING_SOUND);
	}*/
}

void COverlordHealth::OnThink()
{
	GET_OVERLORD_DATA_ASSERT(pData);

	if(pData->GetOverlord())
	{
		int health = ( pData->GetOverlord()->GetHealth() >= 0 ) ? pData->GetOverlord()->GetHealth() : 0;
		SetDisplayValue(health);
	
		if(pData->IsCountingHealth())
		{
			// Display at 25/50/75/100% GAINED
			int maxHealth = pData->GetHealthCap();
			int minHealth = pData->GetMinimumHealth();

			if((m_iState != STATE_100) && (health > minHealth))
			{
				int absoluteGain = maxHealth - minHealth;
				int relativeGain = health - minHealth;

				float factor = (float)relativeGain/(float)absoluteGain;

				switch(m_iState)
				{
				case STATE_0:
					if(factor >= 0.25)
					{
						m_iState = STATE_25;
						Notify(0.25f);
					}
					break;
				case STATE_25:
					if(factor >= 0.5)
					{
						m_iState = STATE_50;
						Notify(0.5f);
					}
					break;
				case STATE_50:
					if(factor >= 0.75)
					{
						m_iState = STATE_75;
						Notify(0.75f);
					}
					break;
				case STATE_75:
					if(factor >= 1.0)
					{
						m_iState = STATE_100;
						Notify(1.0f);
					}
					break;
				default:
					break;
				}
			}
		}
	}

	
}

void COverlordHealth::Notify(float percentage)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return;

	char text[256];
	if(percentage >= 1.0f)
	{
		Q_snprintf(text, ARRAYSIZE(text), "The Overlordess is at the peak of her abilities... good luck...");
	}
	else
	{
		int display = percentage * 100;
		Q_snprintf(text, ARRAYSIZE(text), "The Overlordess has charged %i percentage of her health...", display);
	}
	
	//pPlayer->EmitSound(WARNING_SOUND);
	internalCenterPrint->ColorPrint(255, 0, 0, 255, text);
	//ClientPrint(pPlayer, HUD_PRINTCENTER, text);
}

void COverlordHealth::PaintNumbers(vgui::HFont font, int xpos, int ypos, int value)
{
	C_OverlordData * data = GET_OVERLORD_DATA;

	if(!data)
		return;

	// Most of it taken from numeric display, we want to draw both max and current health
	surface()->DrawSetTextFont(m_hSmallNumberFont);

	
	wchar_t unicode[12];

	swprintf(unicode, L"%d/%d", value, data->GetHealthCap());

	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawUnicodeString( unicode );
	
}