//==============Overlord mod=====================
//	Class menu
//	
//===============================================

#include "cbase.h"
#include "over_classmenu.h"
#include <FileSystem.h>
#include "baseplayer_shared.h"
#include <vgui_controls/RichText.h>
#include <vgui_controls/checkbutton.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TEXT_LENGTH 1024

//=================
// Buttons
COverlordClassButton::COverlordClassButton(vgui::RichText * RichText, Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd):
vgui::Button(parent, panelName, text, pActionSignalTarget, pCmd)
{
	m_pRichText = RichText;
	m_pText = new char[TEXT_LENGTH];
}

COverlordClassButton::COverlordClassButton( vgui::RichText * RichText, Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd):
vgui::Button(parent, panelName, text, pActionSignalTarget, pCmd)
{
	m_pRichText = RichText;
	if(m_pText)
		delete [] m_pText;
}

COverlordClassButton::~COverlordClassButton()
{
	if(m_pText)
		delete [] m_pText;
}
void COverlordClassButton::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	if(!m_pRichText)
		return;

	m_pRichText->SetText(m_pText);
}

void COverlordClassButton::InitClassFile(const int classnum)
{
	PlayerClass_t playerclass = classnum;
	char mszClassName[15];
	if(CLASS_ASSAULT & playerclass)
	{
			Q_strncpy(mszClassName, "class_assault", sizeof(mszClassName));
	}
	else if(CLASS_PSION & playerclass)
	{
			Q_strncpy(mszClassName, "class_psion", sizeof(mszClassName));
	}
	else if(CLASS_HACKER & playerclass)
	{
			Q_strncpy(mszClassName, "class_hacker", sizeof(mszClassName));
	}
	else if(CLASS_STEALTH & playerclass)
	{
			Q_strncpy(mszClassName, "class_stealth", sizeof(mszClassName));
	}
	
	char dest[40];
	Q_snprintf( dest, sizeof(dest), "resource/%s.txt", mszClassName);

	FileHandle_t f = g_pFullFileSystem->Open(dest, "rb");
	if(!f)
		return;

	char text[TEXT_LENGTH];

	// Text shouldn't be larger than 1 KB
	int size = min(g_pFullFileSystem->Size( f ) + 1, TEXT_LENGTH); 
 
	g_pFullFileSystem->Read(text, size, f);
	g_pFullFileSystem->Close(f);
	
	Q_strncpy(m_pText, text, size);
}



//=============
// Menu


COverlordClassMenu::COverlordClassMenu(IViewPort *pViewPort) : vgui::Frame(NULL, PANEL_CLASS )
{
	m_pViewPort = pViewPort;

	SetScheme("ClientScheme");

	SetTitle("", true);

	SetMoveable(false);

	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);

	SetProportional(true);

	SetSizeable(false);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);

	CreateControls();

	LoadControlSettings("resource/ui/ClassMenu.res");
	InvalidateLayout();

	for(int i = 0; i < NUM_OF_CLASSES; i++)
	{
		m_pButton[i] = NULL;
	}

	m_AssaultLabel = NULL;
	m_PsionLabel = NULL;
	m_HackerLabel = NULL;
	m_StealtherLabel = NULL;

}

COverlordClassMenu::~COverlordClassMenu()
{
	for(int i = 0; i < NUM_OF_CLASSES; i++)
	{
		if(m_pButton[i])
			delete m_pButton[i];
	}

	if(m_pDesc)
		delete m_pDesc;

	if(m_AssaultLabel)
		delete m_AssaultLabel;

	if(m_PsionLabel)
		delete m_PsionLabel;

	if(m_HackerLabel)
		delete m_HackerLabel;

	if(m_StealtherLabel)
		delete m_StealtherLabel;
}

void COverlordClassMenu::OnThink()
{
	// A workaround for the "no cursor" bug when joining a server 
	// and closing the MotD
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	int psions = 0;
	int assault = 0;
	int hackers = 0;
	int stealthers = 0;

	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		C_BasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive() || pPlayer == C_BasePlayer::GetLocalPlayer())
			continue;
		
		if(pPlayer->GetPlayerClass() == CLASS_ASSAULT)
			assault++;
		else if(pPlayer->GetPlayerClass() == CLASS_PSION)
			psions++;
		else if(pPlayer->GetPlayerClass() == CLASS_HACKER)
			hackers++;
		else if(pPlayer->GetPlayerClass() == CLASS_STEALTH)
			stealthers++;
	}

	if(m_AssaultLabel)
	{
		char text[64];
			
		if(assault == 1)
			Q_snprintf(text, ARRAYSIZE(text), "1 person");
		else
			Q_snprintf(text, ARRAYSIZE(text), "%i people", assault);

		m_AssaultLabel->SetText(text);
	}

	if(m_PsionLabel)
	{
		char text[64];
		
		if(psions == 1)
			Q_snprintf(text, ARRAYSIZE(text), "1 person");
		else
			Q_snprintf(text, ARRAYSIZE(text), "%i people", psions);

		m_PsionLabel->SetText(text);
	}

	if(m_HackerLabel)
	{
		char text[64];
		
		if(hackers == 1)
			Q_snprintf(text, ARRAYSIZE(text), "1 person");
		else
			Q_snprintf(text, ARRAYSIZE(text), "%i people", hackers);

		m_HackerLabel->SetText(text);
	}

	if(m_StealtherLabel)
	{
		char text[64];
		
		if(stealthers == 1)
			Q_snprintf(text, ARRAYSIZE(text), "1 person");
		else
			Q_snprintf(text, ARRAYSIZE(text), "%i people", stealthers);

		m_StealtherLabel->SetText(text);
	}
}

void COverlordClassMenu::CreateControls()
{

	// Initing our richtext
	m_pDesc = new vgui::RichText(this, "classdesc");
	m_pDesc->SetUnusedScrollbarInvisible(true);

	char buttonname[14];

	for(int i = 0; i < NUM_OF_CLASSES;  i++)
	{
		if(m_pButton[i])
			continue;


		if(m_pButton[i])
		{
			delete m_pButton[i];
			m_pButton[i] = NULL;
		}
		sprintf(buttonname, "Button0%d", i);

		m_pButton[i] = new COverlordClassButton(m_pDesc, this, buttonname, "");

		// We need to add one to the classnum, otherwise it loads the wrong file
		m_pButton[i]->InitClassFile((1<<(i+1)));
	}

	m_AssaultLabel = new vgui::Label(this, "AssaultLabel", "");
	m_PsionLabel = new vgui::Label(this, "PsionLabel", "");
	m_HackerLabel = new vgui::Label(this, "HackerLabel", "");
	m_StealtherLabel = new vgui::Label(this, "StealtherLabel", "");
}

void COverlordClassMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

//	SetKeyBoardInputEnabled(false);

	if (bShow)
	{
		Activate();

		/*SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(true);
		SetVisible(true);*/
	}
	else
	{
		SetKeyBoardInputEnabled(false);
		SetMouseInputEnabled(false);
		SetVisible(false);
	}
}

void COverlordClassMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("BaseBorder"));

	// We need to init it here, as we can't get pScheme pointer before
	// this function is called
	if(m_pDesc)
		m_pDesc->SetBorder( pScheme->GetBorder("BaseBorder") );

	vgui::CheckButton * pCheck = static_cast<vgui::CheckButton*>(FindChildByName("overlordcheck"));

	if(pCheck)
	{
		bool bSet = false;
		C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
		if(pPlayer)
		{
			bSet = !pPlayer->WantsToBeOverlord();
		}

		pCheck->SetSelected(bSet);
		pCheck->SetCommand("OverlordCheckPressed");
	}
	else
		Warning("No check button found!\n");

	SetPaintBorderEnabled(true);
	SetPaintBackgroundType(2);
}

void COverlordClassMenu::OnCommand(const char *command)
{
	if(!Q_stricmp(command, "OverlordCheckPressed"))
	{
		engine->ServerCmd("ToggleOverlordPreference");
		return;
	}
	/*if(!Q_strcmp(command, "classassault") 
		|| !Q_strcmp(command, "classpsion")
		|| !Q_strcmp(command, "classhacker")
		|| !Q_strcmp(command, "classrandom")
		|| !Q_strcmp(command, "classstealth"))*/
		engine->ServerCmd(VarArgs("eo_selectclass %s\n", command));

	Close();
}