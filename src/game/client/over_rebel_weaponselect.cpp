#include "cbase.h"
#include "weapon_selection.h"
#include "iclientmode.h"
#include "history_resource.h"
#include "input.h"

#include "hl2mp_gamerules.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Panel.h>

#include "vgui/ILocalize.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class COverRebelWeaponselect : public CBaseHudWeaponSelection, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(COverRebelWeaponselect, vgui::Panel);

	public:
		COverRebelWeaponselect(const char *pElementName);

		virtual bool ShouldDraw();
		virtual void OnWeaponPickup( C_BaseCombatWeapon *pWeapon );

		virtual void CycleToNextWeapon();
		virtual void CycleToPrevWeapon();
		virtual C_BaseCombatWeapon *GetWeaponInSlot( int iSlot, int iSlotPos );

		virtual void SelectWeaponSlot( int iSlot );	

		virtual void LevelInit();

		virtual C_BaseCombatWeapon	*GetSelectedWeapon( void )
	{ 
		return m_hSelectedWeapon;
	}

		virtual int GetTeam() const { return TEAM_REBELS; };

	protected:

		virtual void OnThink();
		virtual void Paint();
		virtual void ApplySchemeSettings(vgui::IScheme *pScheme);	
		virtual bool IsWeaponSelectable()
			{ 
				if (IsInSelectionMode())
					return true;

					return false;
			}
	
	private:

		C_BaseCombatWeapon *FindNextWeapon(int iCurrentSlot, int iCurrentPosition);
		C_BaseCombatWeapon *FindPrevWeapon(int iCurrentSlot, int iCurrentPosition);
		C_BaseCombatWeapon *GetPlayerWeapon(int slot);


		void ActivateHighlight (C_BaseCombatWeapon *pWeapon);
		void FastWeaponSwitch (int iSlot);
		void PlusTypeFastWeaponSwitch (int iSlot);
		void DrawWeaponIcon(CHudTexture *full, CHudTexture *empty, int slot, int xOffset = 0, int yOffset = 0);

		virtual void SetSelectedWeapon (C_BaseCombatWeapon *pWeapon)
		{
			m_hSelectedWeapon = pWeapon;
		}

		CHudTexture *m_weaponSidearm;
		CHudTexture *m_weaponPrimary;
		CHudTexture *m_weaponSecondary;
		CHudTexture *m_weaponSpecial;
		CHudTexture *m_bar;
		CUtlVector<C_BaseCombatWeapon* > m_weaponList;


		CPanelAnimationVar(vgui::HFont, m_ammoindicator, "NumberFont", "HudSelectionNumbers");

		C_BaseCombatWeapon *m_pLastWeapon;

		int m_xCen;
		int m_yCen;
};

//DECLARE_HUDELEMENT(COverRebelWeaponselect);

using namespace vgui;




COverRebelWeaponselect::COverRebelWeaponselect(const char *pElementName) : CBaseHudWeaponSelection(pElementName), BaseClass(NULL, "COverRebelWeaponselect")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits(HIDEHUD_CROSSHAIR | HIDEHUD_CAMERA | HIDEHUD_REBEL);
}

void COverRebelWeaponselect::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

void COverRebelWeaponselect::LevelInit()
{
	CHudElement::LevelInit();
	m_pLastWeapon = NULL;

	
	m_weaponPrimary = gHUD.GetIcon("over_wPrimaryIcon");
	m_weaponSecondary = gHUD.GetIcon("over_wSecondaryIcon");
	m_weaponSidearm = gHUD.GetIcon("over_wSidearmIcon");
	m_weaponSpecial = gHUD.GetIcon("over_wSpecialIcon");
	//Get ammobar for positioning purposes.
	m_bar = gHUD.GetIcon("over_ammobar");

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if(!pPlayer)
			return;
}



bool COverRebelWeaponselect::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if(!pPlayer)
		{
			return false;
		}
		if(pPlayer->IsOverlord()){
			return false;
		}
	return true;
}

void COverRebelWeaponselect::Paint()
{
	double aspectRatio = (double)ScreenWidth()/(double)ScreenHeight();
	ISchemeManager* scheme = vgui::scheme();
		if(!scheme)
			return;
		
	if(aspectRatio*10 > 16)
	{
		m_xCen = ((ScreenWidth() - (scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_bar->Width()))/((2*aspectRatio)/5))/2);
	}else{
		m_xCen = (ScreenWidth() - scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_bar->Width()))/2;	
	}
		m_yCen = (ScreenHeight())/2 + ((scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_bar->Height()))/4);
		
	if(!ShouldDraw())
				return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if(!pPlayer)
			return;

	C_BaseCombatWeapon *pSelectedWeapon = NULL;		
		pSelectedWeapon = pPlayer->GetActiveWeapon();

	
	//DRAW INFO_SIDEARM
		DrawWeaponIcon(m_weaponSidearm, m_weaponSidearm, 2, (m_bar->Width()-10), m_weaponSecondary->Height());
	//DRAW INFO_SECONDARY
		DrawWeaponIcon(m_weaponSecondary, m_weaponSecondary, 1, (m_bar->Width()+5), (m_weaponSecondary->Height()*2 + 27));
	//DRAW INFO_PRIMARY
		DrawWeaponIcon(m_weaponPrimary, m_weaponPrimary, 0, (m_bar->Width()-10), (m_weaponSecondary->Height()*3 + 27));
	//DRAW INFO_SPECIALS

	int m_specialXOffsetMult = -1;

	m_xCen = (ScreenWidth() - (scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), m_weaponSpecial->Width()))/2)/2;

	char tDebug[10];

		for(int i=0; i<3; i++)
		{
			DrawWeaponIcon(m_weaponSpecial, m_weaponSpecial, 5-i, m_specialXOffsetMult*150, -20);
			m_specialXOffsetMult++;
			sprintf(tDebug, "%d:%d\n", i, m_specialXOffsetMult);
			DevMsg(tDebug);
		}
}

void COverRebelWeaponselect::DrawWeaponIcon(CHudTexture *full, CHudTexture *empty, int slot, int xOffset, int yOffset)
{
	int m_tempIconHeight = 0;
	int m_tempIconWidth = 0;
	wchar_t ammoCount[32];

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
			if(!pPlayer)
				return;

	C_BaseCombatWeapon *pWeapon = GetPlayerWeapon(slot);
		if(pWeapon == NULL)
			return;

	ISchemeManager* scheme = vgui::scheme();
		if(scheme == NULL)
			return;

		yOffset = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), yOffset)/2;
		xOffset = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), xOffset)/2;

		m_tempIconHeight = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), full->Height())/2;
		m_tempIconWidth = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), full->Width())/2;

		gHUD.DrawIconProgressBar((m_xCen-xOffset), (m_yCen-yOffset), full, empty, 1.0f, gHUD.m_clrNormal, CHud::HUDPB_VERTICAL, m_tempIconWidth, m_tempIconHeight);
		
		m_tempIconHeight = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), pWeapon->GetSpriteActive()->Height())/2;
		m_tempIconWidth = scheme->GetProportionalScaledValueEx(BaseClass::GetScheme(), pWeapon->GetSpriteActive()->Width())/2;
		

	if(pPlayer->GetActiveWeapon() == pWeapon)
	{
		pWeapon->GetSpriteActive()->DrawSelf((m_xCen-xOffset), (m_yCen-yOffset), m_tempIconWidth, m_tempIconHeight, gHUD.m_clrNormal);
	}
	else
	{
		pWeapon->GetSpriteInactive()->DrawSelf((m_xCen-xOffset), (m_yCen-yOffset), m_tempIconWidth, m_tempIconHeight, gHUD.m_clrNormal);
	}

		
		
		
		surface()->DrawSetTextColor(gHUD.m_clrNormal);
		surface()->DrawSetTextFont(m_ammoindicator);
		surface()->DrawSetTextPos((m_xCen-xOffset), (m_yCen-yOffset));		
		if( pWeapon->Clip1() > 0)
		{
			swprintf(ammoCount, L"%d/%d", pWeapon->Clip1(), pWeapon->GetMaxClip1());
		}
		else
		{
			swprintf(ammoCount, L"%d/%d", pPlayer->GetAmmoCount((pWeapon->GetPrimaryAmmoType())), GetAmmoDef()->MaxCarry(GetAmmoDef()->Index("Grenade")));
		}

		surface()->DrawUnicodeString(ammoCount);
}	

//PORTED FROM VALVE's HUD_WEAPONSELECTION

C_BaseCombatWeapon *COverRebelWeaponselect::GetPlayerWeapon(int slot)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if(!pPlayer)
			return NULL;

	for(int i=0; i<MAX_WEAPONS; i++)
	{
		if(pPlayer->GetWeapon(i) != NULL && ((pPlayer->GetWeapon(i))->GetSlot()==slot) )
			return pPlayer->GetWeapon(i);
	}
		return NULL;
}

C_BaseCombatWeapon *COverRebelWeaponselect::FindNextWeapon(int iCurrentSlot, int iCurrentPosition)
{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return NULL;

	C_BaseCombatWeapon *pNextWeapon = NULL;

	// search all the weapons looking for the closest next
	int iLowestNextSlot = MAX_WEAPON_SLOTS;
	int iLowestNextPosition = MAX_WEAPON_POSITIONS;
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if ( !pWeapon )
			continue;

		if ( CanBeSelectedInHUD( pWeapon ) )
		{
			int weaponSlot = pWeapon->GetSlot(), weaponPosition = pWeapon->GetPosition();

			// see if this weapon is further ahead in the selection list
			if ( weaponSlot > iCurrentSlot || (weaponSlot == iCurrentSlot && weaponPosition > iCurrentPosition) )
			{
				// see if this weapon is closer than the current lowest
				if ( weaponSlot < iLowestNextSlot || (weaponSlot == iLowestNextSlot && weaponPosition < iLowestNextPosition) )
				{
					iLowestNextSlot = weaponSlot;
					iLowestNextPosition = weaponPosition;
					pNextWeapon = pWeapon;
				}
			}
		}
	}

	return pNextWeapon;
}
//DITTO
C_BaseCombatWeapon *COverRebelWeaponselect::FindPrevWeapon(int iCurrentSlot, int iCurrentPosition)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return NULL;

	C_BaseCombatWeapon *pPrevWeapon = NULL;

	// search all the weapons looking for the closest next
	int iLowestPrevSlot = -1;
	int iLowestPrevPosition = -1;
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if ( !pWeapon )
			continue;

		if ( CanBeSelectedInHUD( pWeapon ) )
		{
			int weaponSlot = pWeapon->GetSlot(), weaponPosition = pWeapon->GetPosition();

			// see if this weapon is further ahead in the selection list
			if ( weaponSlot < iCurrentSlot || (weaponSlot == iCurrentSlot && weaponPosition < iCurrentPosition) )
			{
				// see if this weapon is closer than the current lowest
				if ( weaponSlot > iLowestPrevSlot || (weaponSlot == iLowestPrevSlot && weaponPosition > iLowestPrevPosition) )
				{
					iLowestPrevSlot = weaponSlot;
					iLowestPrevPosition = weaponPosition;
					pPrevWeapon = pWeapon;
				}
			}
		}
	}

	return pPrevWeapon;
}

void COverRebelWeaponselect::OnThink()
{
}


void COverRebelWeaponselect::CycleToNextWeapon()
{
		// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	m_pLastWeapon = pPlayer->GetActiveWeapon();

	C_BaseCombatWeapon *pNextWeapon = NULL;
	if ( IsInSelectionMode() )
	{
		// find the next selection spot
		C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();
		if ( !pWeapon )
			return;

		pNextWeapon = FindNextWeapon( pWeapon->GetSlot(), pWeapon->GetPosition() );
	}
	else
	{
		// open selection at the current place
		pNextWeapon = pPlayer->GetActiveWeapon();
		if ( pNextWeapon )
		{
			pNextWeapon = FindNextWeapon( pNextWeapon->GetSlot(), pNextWeapon->GetPosition() );
		}
	}

	if ( !pNextWeapon )
	{
		// wrap around back to start
		pNextWeapon = FindNextWeapon(-1, -1);
	}

	if ( pNextWeapon )
	{
		SetSelectedWeapon( pNextWeapon );
		//SetSelectedSlideDir( 1 );

		if ( !IsInSelectionMode() )
		{
			OpenSelection();
		}

		// Play the "cycle to next weapon" sound
		pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
	}
}

void COverRebelWeaponselect::CycleToPrevWeapon()
{
		// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	m_pLastWeapon = pPlayer->GetActiveWeapon();

	C_BaseCombatWeapon *pNextWeapon = NULL;
	if ( IsInSelectionMode() )
	{
		// find the next selection spot
		C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();
		if ( !pWeapon )
			return;

		pNextWeapon = FindPrevWeapon( pWeapon->GetSlot(), pWeapon->GetPosition() );
	}
	else
	{
		// open selection at the current place
		pNextWeapon = pPlayer->GetActiveWeapon();
		if ( pNextWeapon )
		{
			pNextWeapon = FindPrevWeapon( pNextWeapon->GetSlot(), pNextWeapon->GetPosition() );
		}
	}

	if ( !pNextWeapon )
	{
		// wrap around back to end of weapon list
		pNextWeapon = FindPrevWeapon(MAX_WEAPON_SLOTS, MAX_WEAPON_POSITIONS);
	}

	if ( pNextWeapon )
	{
		SetSelectedWeapon( pNextWeapon );
//		SetSelectedSlideDir( -1 );

		if ( !IsInSelectionMode() )
		{
			OpenSelection();
		}

		// Play the "cycle to next weapon" sound
		pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
	}
}

C_BaseCombatWeapon *COverRebelWeaponselect::GetWeaponInSlot(int iSlot, int iSlotPos)
{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		return pPlayer->GetWeapon(iSlot);
}

void COverRebelWeaponselect::OnWeaponPickup(C_BaseCombatWeapon *pWeapon)
{
}

void COverRebelWeaponselect::FastWeaponSwitch( int iWeaponSlot )
{
	// get the slot the player's weapon is in
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	m_pLastWeapon = NULL;

	// see where we should start selection
	int iPosition = -1;
	C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();
	if ( pActiveWeapon && pActiveWeapon->GetSlot() == iWeaponSlot )
	{
		// start after this weapon
		iPosition = pActiveWeapon->GetPosition();
	}

	C_BaseCombatWeapon *pNextWeapon = NULL;

	// search for the weapon after the current one
	pNextWeapon = FindNextWeapon(iWeaponSlot, iPosition);
	// make sure it's in the same bucket
	if ( !pNextWeapon || pNextWeapon->GetSlot() != iWeaponSlot )
	{
		// just look for any weapon in this slot
		pNextWeapon = FindNextWeapon(iWeaponSlot, -1);
	}

	// see if we found a weapon that's different from the current and in the selected slot
	if ( pNextWeapon && pNextWeapon != pActiveWeapon && pNextWeapon->GetSlot() == iWeaponSlot )
	{
		// select the new weapon
		::input->MakeWeaponSelection( pNextWeapon );
	}
	else if ( pNextWeapon != pActiveWeapon )
	{
		// error sound
		pPlayer->EmitSound( "Player.DenyWeaponSelection" );
	}

	if ( HUDTYPE_CAROUSEL != hud_fastswitch.GetInt() )
	{
		// kill any fastswitch display
		m_flSelectionTime = 0.0f;
	}
}

void COverRebelWeaponselect::PlusTypeFastWeaponSwitch( int iWeaponSlot )
{
	
}


//-----------------------------------------------------------------------------
// Purpose: Moves selection to the specified slot
//-----------------------------------------------------------------------------
void COverRebelWeaponselect::SelectWeaponSlot( int iSlot )
{
	// iSlot is one higher than it should be, since it's the number key, not the 0-based index into the weapons
	--iSlot;

	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;

	// Don't try and read past our possible number of slots
	if ( iSlot > MAX_WEAPON_SLOTS )
		return;
	
	// Make sure the player's allowed to switch weapons
	if ( pPlayer->IsAllowedToSwitchWeapons() == false )
		return;

	switch( hud_fastswitch.GetInt() )
	{
	case HUDTYPE_FASTSWITCH:
	case HUDTYPE_CAROUSEL:
		{
			FastWeaponSwitch( iSlot );
			return;
		}
		
	case HUDTYPE_PLUS:
		{
			if ( !IsInSelectionMode() )
			{
				// open the weapon selection
				OpenSelection();
			}
				
			PlusTypeFastWeaponSwitch( iSlot );

		}
		break;

	case HUDTYPE_BUCKETS:
		{
			int slotPos = 0;
			C_BaseCombatWeapon *pActiveWeapon = GetSelectedWeapon();

			// start later in the list
			if ( IsInSelectionMode() && pActiveWeapon && pActiveWeapon->GetSlot() == iSlot )
			{
				slotPos = pActiveWeapon->GetPosition() + 1;
			}

			// find the weapon in this slot
			pActiveWeapon = GetNextActivePos( iSlot, slotPos );
			if ( !pActiveWeapon )
			{
				pActiveWeapon = GetNextActivePos( iSlot, 0 );
			}
			
			if ( pActiveWeapon != NULL )
			{
				if ( !IsInSelectionMode() )
				{
					// open the weapon selection
					OpenSelection();
				}

				// Mark the change
				SetSelectedWeapon( pActiveWeapon );

			}
		}

	default:
		{
			// do nothing
		}
		break;
	}

	pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
}