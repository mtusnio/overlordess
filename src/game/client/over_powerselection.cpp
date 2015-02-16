//==============Overlord mod=====================
//	Ov power selection menu
//===============================================

#include "cbase.h"
#include "over_powerselection.h"
#include "iclientmode.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "iinput.h"

#include "vgui/ILocalize.h"

#include "basecombatweapon_shared.h"

#define IMAGE_WIDTH 24
#define IMAGE_HEIGHT 24
#define LABEL_IMAGE_MARGIN 2
#define LABEL_TALL 7

#define IMAGE_SPACE 2

DECLARE_HUDELEMENT(COverlordPowerSelection);

COverlordPowerSelection::COverlordPowerSelection( const char *pElementName ) : 
BaseClass( NULL, "HudPowerSelection" ), CBaseHudWeaponSelection( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();

	if(pParent)
		SetParent(pParent->GetVPanel());

	SetProportional(true);

	SetHiddenBits( HIDEHUD_WEAPONSELECTION | HIDEHUD_NEEDSUIT 
		| HIDEHUD_PLAYERDEAD | HIDEHUD_INVEHICLE 
		| HIDEHUD_CAMERA | HIDEHUD_OVERLORD);

	for(int i = 0; i < MAX_POWERS; i++)
		m_Image[i] = NULL;

	for(int i = 0; i < MAX_POWERS; i++)
		m_Label[i] = NULL;

	m_hSelectedWeapon = NULL;

	m_InfoPanel = NULL;
	m_InfoWeaponName = NULL;
	m_hFont = 0;
	m_InfoPrimary = NULL;
	m_InfoSecondary = NULL;
	m_InfoCost = NULL;
}

COverlordPowerSelection::~COverlordPowerSelection()
{
	for(int i = 0; i < MAX_POWERS; i++)
	{
		if(m_Image[i])
			delete m_Image[i];
	}
}

void COverlordPowerSelection::Init()
{
	CBaseHudWeaponSelection::Init();

	for(int i = 0; i < MAX_POWERS; i++)
	{
		if(m_Image[i])
		{
			delete m_Image[i];
			m_Image[i] = NULL;
		}

		if(m_Label[i])
		{
			delete m_Label[i];
			m_Label[i] = NULL;
		}

		char name[32];
		Q_snprintf(name, ARRAYSIZE(name), "Image%i", i);
		m_Image[i] = new vgui::ImagePanel(this, name);
		m_Label[i] = new vgui::Label(this, "", "");
	}
	RevertAllImages();
}

void COverlordPowerSelection::OnThink()
{
	
}

void COverlordPowerSelection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	// We need a counter for this, the first time the menu is created
	// is when we launch the game, THEN we need to create all panels etc.
	static int bOnce = 0;
	SetPaintBackgroundType(2);

	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("DefaultVerySmall");
	bOnce++;
	if(bOnce == 2)
	{
		// Set position for all image buttons
		for(int i = 0; i < MAX_POWERS; i++)
		{
			if(!m_Image[i])
				continue;

			int y = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 3);
			int x = vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 5) +
				i * vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), IMAGE_WIDTH) +
				i * vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), IMAGE_SPACE);

			m_Image[i]->SetPos(x, y);
			m_Image[i]->SetBorder(pScheme->GetBorder("BaseBorder"));

			m_Image[i]->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), IMAGE_HEIGHT));
			m_Image[i]->SetWide(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), IMAGE_WIDTH));
			m_Image[i]->SetVisible(false);
			m_Image[i]->SetShouldScaleImage(true);

			// Now do the labels
			if(!m_Label[i])
				continue;

			int slot = IndexToSlot(i);

			char text[2];
			itoa(slot, text, 10);

			m_Label[i]->SetText(text);
			y += vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), IMAGE_HEIGHT) + 
				vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), LABEL_IMAGE_MARGIN);

			m_Label[i]->SetPos(x, y);
			m_Label[i]->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), LABEL_TALL));
			m_Label[i]->SetWide(m_Image[i]->GetWide());
			m_Label[i]->SetContentAlignment(vgui::Label::a_center);
			m_Label[i]->SetFont(m_hFont);
			m_Label[i]->SetVisible(false);
		}

		// Create the info panel
		m_InfoPanel = new vgui::Frame(this, "PowerSelectionInfo");

		// Configure it
		if(m_InfoPanel)
		{
			m_InfoWeaponName = new vgui::Label(m_InfoPanel, "InfoPanelWeapon", "");
			m_InfoPrimary = new vgui::Label(m_InfoPanel, "InfoPanelPrimary", "");
			m_InfoSecondary = new vgui::Label(m_InfoPanel, "InfoPanelSecondary", "");
			m_InfoCost = new vgui::Label(m_InfoPanel, "InfoPanelCost", "");

			// Create all labels etc. here
			int x, y;
			GetPos(x, y);
			m_InfoPanel->SetMouseInputEnabled(false);
			m_InfoPanel->SetKeyBoardInputEnabled(false);
			m_InfoPanel->SetProportional(true);
			m_InfoPanel->LoadControlSettings("Resource/UI/selectioninfo.res");
			m_InfoPanel->SetPos(x, y - m_InfoPanel->GetTall());
			m_InfoPanel->SetTitleBarVisible(false);
			m_InfoPanel->SetMaximizeButtonVisible(false);
			m_InfoPanel->SetMinimizeButtonVisible(false);
			m_InfoPanel->SetPaintBackgroundType(2);
			m_InfoPanel->SetVisible(false);

			
			if(m_InfoWeaponName)
			{
				m_InfoWeaponName->SetFont(m_hFont);
			}

			if(m_InfoPrimary)
			{
				m_InfoPrimary->SetFont(m_hFont);
			}

			if(m_InfoSecondary)
			{
				m_InfoSecondary->SetFont(m_hFont);
			}

			if(m_InfoCost)
			{
				m_InfoCost->SetContentAlignment(vgui::Label::a_east);
				m_InfoCost->SetFont(m_hFont);
			}
		}
	}

	// Apply scheme every time it's changed
	if(m_InfoPanel)
	{
		m_InfoPanel->SetScheme(GetScheme());
	}

}

void COverlordPowerSelection::SelectWeaponSlot( int iSlot )
{
	if(iSlot == 0)
		iSlot =9;
	else
		iSlot--;

	C_BaseCombatWeapon * pWeap = GetWeaponInSlot(iSlot);

	if(!pWeap)
	{
		return;
	}

	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(pPlayer)
		pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );

	SetImageAsSelected(iSlot);
	m_hSelectedWeapon = pWeap;
	SetWeaponSelected();
	UpdateWeaponDisplay();
}


C_BaseCombatWeapon * COverlordPowerSelection::GetWeaponInSlot(int iSlot) const
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon * pWeapon = pPlayer->GetWeapon(i);

		if(!pWeapon)
			continue;

		if(pWeapon->GetSlot() == iSlot)
			return pWeapon;
	}

	return NULL;
}

C_BaseCombatWeapon * COverlordPowerSelection::GetCurrentWeapon() const
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;

	return pPlayer->GetActiveWeapon();
}

C_BaseCombatWeapon * COverlordPowerSelection::GetNextWeapon() const
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;

	C_BaseCombatWeapon * pCurrent = GetCurrentWeapon();

	if(!pCurrent)
		return NULL;

	// Look for the next weapon...
	C_BaseCombatWeapon * pNext = NULL;
	int distance = MAX_WEAPONS+1;
	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon * pWeapon = pPlayer->GetWeapon(i);

		if(!pWeapon || pWeapon == pCurrent)
			continue;

		int diff = pWeapon->GetSlot() - pCurrent->GetSlot();

		if(diff > 0)
		{
			if(distance > diff)
			{
				distance = diff;
				pNext = pWeapon;
			}
		}
	}

	if(pNext)
		return pNext;

	
	// Now look for the lowest slot possible
	int slot = MAX_POWERS;
	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon * pWeapon = pPlayer->GetWeapon(i);

		if(!pWeapon || pWeapon == pCurrent)
			continue;

		int thisslot = pWeapon->GetSlot();
		if(slot > thisslot)
		{
			slot = thisslot;
			pNext = pWeapon;
		}
	}

	if(pNext)
		return pNext;

	return NULL;
}

C_BaseCombatWeapon * COverlordPowerSelection::GetPreviousWeapon() const
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer)
		return NULL;

	C_BaseCombatWeapon * pCurrent = GetCurrentWeapon();

	if(!pCurrent)
		return NULL;

	// Look for the next weapon...
	C_BaseCombatWeapon * pPrevious = NULL;
	int distance = -(MAX_WEAPONS+1);
	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon * pWeapon = pPlayer->GetWeapon(i);

		if(!pWeapon || pWeapon == pCurrent)
			continue;

		int diff = pWeapon->GetSlot() - pCurrent->GetSlot();

		if(diff < 0)
		{
			if(distance < diff)
			{
				distance = diff;
				pPrevious = pWeapon;
			}
		}
	}

	if(pPrevious)
		return pPrevious;

	
	// Now look for the lowest slot possible
	int slot = -MAX_POWERS;
	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon * pWeapon = pPlayer->GetWeapon(i);

		if(!pWeapon || pWeapon == pCurrent)
			continue;

		int thisslot = pWeapon->GetSlot();
		if(slot < thisslot)
		{
			slot = thisslot;
			pPrevious = pWeapon;
		}
	}

	if(pPrevious)
		return pPrevious;

	return NULL;
}

void COverlordPowerSelection::UpdateWeaponDisplay()
{
	if(m_hSelectedWeapon)
	{
		int index = m_hSelectedWeapon->GetSlot();
		if(m_InfoPanel)
		{
			if(m_Image[index])
			{
				int x, y;
				m_Image[index]->GetPos(x, y);

				int pY, pX;
				GetPos(pX, pY);

				int iY, iX;
				m_InfoPanel->GetPos(iX, iY);
				m_InfoPanel->SetPos(pX + x, iY);
				m_InfoPanel->SetVisible(true);
			}

			if(m_InfoWeaponName)
			{
				char name[128];
				wchar_t * text = g_pVGuiLocalize->Find(m_hSelectedWeapon->GetPrintName());

				if(text)
				{
					g_pVGuiLocalize->ConvertUnicodeToANSI(text, name, ARRAYSIZE(name));
				}
				else
				{
					Q_strncpy(name, m_hSelectedWeapon->GetPrintName(), ARRAYSIZE(name));
				}
				m_InfoWeaponName->SetText(name);
			}

			if(m_InfoPrimary)
			{
				char name[128];
				wchar_t * text = g_pVGuiLocalize->Find(m_hSelectedWeapon->GetPrimaryMode());

				if(text)
				{
					g_pVGuiLocalize->ConvertUnicodeToANSI(text, name, ARRAYSIZE(name));
				}
				else
				{
					Q_strncpy(name, m_hSelectedWeapon->GetPrimaryMode(), ARRAYSIZE(name));
				}

				char final[128];
				Q_snprintf(final, ARRAYSIZE(final), "Primary: %s", name);
				m_InfoPrimary->SetText(final);
			}

			if(m_InfoSecondary)
			{
				char name[128];
				wchar_t * text = g_pVGuiLocalize->Find(m_hSelectedWeapon->GetSecondaryMode());

				if(text)
				{
					g_pVGuiLocalize->ConvertUnicodeToANSI(text, name, ARRAYSIZE(name));
				}
				else
				{
					Q_strncpy(name, m_hSelectedWeapon->GetSecondaryMode(), ARRAYSIZE(name));
				}
				char final[128];
				Q_snprintf(final, ARRAYSIZE(final), "Secondary: %s", name);
				m_InfoSecondary->SetText(final);
			}

			if(m_InfoCost)
			{
				char name[128];
				wchar_t * text = g_pVGuiLocalize->Find(m_hSelectedWeapon->GetCostLabel());

				if(text)
				{
					g_pVGuiLocalize->ConvertUnicodeToANSI(text, name, ARRAYSIZE(name));
				}
				else
				{
					Q_strncpy(name, m_hSelectedWeapon->GetCostLabel(), ARRAYSIZE(name));
				}
				m_InfoCost->SetText(name);
			}
			
		}
	}
}

int COverlordPowerSelection::IndexToSlot(int index) const
{
	int slot = index;
	
	slot++;

	if(slot >= 10)
		slot = 0;

	return slot;
		
}

void COverlordPowerSelection::OnWeaponPickup( C_BaseCombatWeapon *pWeapon )
{
	if(!pWeapon)
		return;

	int slot = pWeapon->GetSlot();

	char path[64];
	CreatePath(ARRAYSIZE(path), path, pWeapon->GetName());

	m_Image[slot]->SetImage(path);
	m_Image[slot]->SetVisible(true);
	m_Label[slot]->SetVisible(true);
}

void COverlordPowerSelection::RevertAllImages()
{
	for(int i = 0; i < MAX_POWERS; i++)
	{
		RevertImage(i);
	}
}

void COverlordPowerSelection::RevertImage(int iSlot)
{
	if(!m_Image[iSlot])
		return;

	C_BaseCombatWeapon * pWeapon = GetWeaponInSlot(iSlot);

	if(!pWeapon)
		return;

	DevMsg("Reverting %i image\n", iSlot);
	char path[64];
	CreatePath(ARRAYSIZE(path), path, pWeapon->GetName()); 
	m_Image[iSlot]->SetImage(path);
}

void COverlordPowerSelection::SetImageAsSelected(int index)
{
	if(!m_Image[index])
	{
		Warning("Trying to selected nonexistant image\n");
		return;
	}

	if(m_hSelectedWeapon)
	{
		RevertImage(m_hSelectedWeapon->GetSlot());
	}

	DevMsg("Setting %i as selected\n", index);

	C_BaseCombatWeapon * pWeapon = GetWeaponInSlot(index);

	if(!pWeapon)
		return;

	char path[64];
	CreatePath(ARRAYSIZE(path), path, pWeapon->GetName(), true);
	m_Image[index]->SetImage(path);
}

void COverlordPowerSelection::CreatePath(int size, char * path, const char * weapon, bool bSelected /*= false*/)
{
	if(!path || !weapon || size == 0)
		return;

	if(!bSelected)
		Q_snprintf( path, size, "weapons/icons/%s.vmt", weapon );
	else
		Q_snprintf( path, size, "weapons/icons/%s_selected.vmt", weapon );
}