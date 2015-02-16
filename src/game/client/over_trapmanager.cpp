//==============Overlord mod=====================
//	Trap manager menu
//===============================================

#include "cbase.h"
#include "over_trapmanager.h"

#include "overlord_dynamictraps.h"
#include "overlord_data.h"
#include "overlord_consolemanager.h"

#include "inputsystem/iinputsystem.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>

#include <vgui/IInput.h>

// Some button consts
#define NUMBER_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 10)
#define NAME_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 60)
#define STATUS_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 45)
#define DURATION_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 40)
#define HEALTH_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(),40)
#define SPACE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9)
#define DEFAULT_TALL vgui::scheme()->GetProportionalScaledValueEx(GetScheme(),10)

#define MARGIN vgui::scheme()->GetProportionalScaledValueEx(GetScheme(),5)
#define Y_SPACE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 12)
#define RECORD_SPACE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 20)

class TrapRecord
{
public:
	TrapRecord(int x, int y, int trapindex, C_OverlordTrap * pTrap, vgui::Panel * parent);
	~TrapRecord();

	void Init();
	bool IsValid();

	void Update();
	int GetIndex() { return m_TrapIndex; };
	vgui::HScheme GetScheme() { return (m_Parent->GetScheme()); };

	int				 GetTrapIndex() const { return m_TrapIndex; }; 
	C_OverlordTrap * GetTrap() const { return m_Trap; };

	bool IsHoveringOver();
	void HealthRestored();

private:
	int m_StartX;
	int m_StartY;
	int m_TrapIndex;

	float m_flRepairDelay;

	CHandle<C_OverlordTrap> m_Trap;

	vgui::Panel * m_Parent;

	vgui::Label * m_Number;
	vgui::Button * m_Name;
	vgui::Button * m_Status;
	vgui::Button * m_Duration;
	vgui::Button * m_Health;
};

TrapRecord::TrapRecord(int x, int y, int trapindex, C_OverlordTrap * pTrap, vgui::Panel * parent)
{
	m_flRepairDelay = 0.0f;
	m_StartX = x;
	m_StartY = y;
	m_TrapIndex = trapindex;
	m_Trap = pTrap;
	m_Parent = parent;

	m_Number = new vgui::Label(parent, "", "");
	
	{
		char command[64];
		Q_snprintf(command, ARRAYSIZE(command), "DecayTrap %i", trapindex);
		m_Name = new vgui::Button(parent, "", "", parent, command);
	}

	{
		char command[64];
		Q_snprintf(command, ARRAYSIZE(command), "ChangeStatus %i", trapindex);
		m_Status = new vgui::Button(parent, "", "", parent, command);
	}

	{
		char command[64];
		Q_snprintf(command, ARRAYSIZE(command), "RestoreDuration %i", trapindex);
		m_Duration = new vgui::Button(parent, "", "", parent, command);
	}

	{
		char command[64];
		Q_snprintf(command, ARRAYSIZE(command), "RestoreHealth %i", trapindex);
		m_Health = new vgui::Button(parent, "", "", parent, command);
	}
}

TrapRecord::~TrapRecord()
{
	if(m_Number)
		delete m_Number;

	if(m_Name)
		delete m_Name;

	if(m_Status)
		delete m_Status;

	if(m_Duration)
		delete m_Duration;

	if(m_Health)
		delete m_Health;
}

void TrapRecord::Init()
{
	const int x = m_StartX;
	const int y = m_StartY;

	if(!m_Parent)
		return;

	// Move everything to place
	if(m_Number)
	{	
		//m_Number->SetProportional(true);
		m_Number->SetPos(x, y);
		m_Number->SetTall(DEFAULT_TALL);
		m_Number->SetWide(NUMBER_WIDE);
		
		char text[6];
		Q_snprintf(text, ARRAYSIZE(text), "%i.", m_TrapIndex+1);
		m_Number->SetText(text);
		//m_Number->SetContentAlignment(vgui::Label::a_center);
	}

	if(m_Name)
	{
		//m_Name->SetProportional(true);
		m_Name->SetPos(x + SPACE + NUMBER_WIDE, y);
		m_Name->SetTall(DEFAULT_TALL);
		m_Name->SetWide(NAME_WIDE);

		if(m_Trap)
		{
			char name[64];
			Q_strncpy(name, m_Trap->GetTrapName(), ARRAYSIZE(name));
			m_Name->SetText(name);
		}
		m_Name->SetContentAlignment(vgui::Label::a_center);
		m_Name->SetPaintBorderEnabled(false);
	}

	if(m_Status)
	{
		//m_Status->SetProportional(true);
		m_Status->SetPos(x + 2*SPACE + NUMBER_WIDE + NAME_WIDE, y);
		m_Status->SetTall(DEFAULT_TALL);
		m_Status->SetWide(STATUS_WIDE);
		m_Status->SetContentAlignment(vgui::Label::a_center);
	}

	if(m_Duration)
	{
		//m_Duration->SetProportional(true);
		m_Duration->SetPos(x + 3*SPACE + NUMBER_WIDE + NAME_WIDE + STATUS_WIDE, y);
		m_Duration->SetTall(DEFAULT_TALL);
		m_Duration->SetWide(DURATION_WIDE);
		m_Duration->SetContentAlignment(vgui::Label::a_center);
	}

	if(m_Health)
	{
		//m_Health->SetProportional(true);
		m_Health->SetPos(x + 4*SPACE + NUMBER_WIDE + NAME_WIDE + STATUS_WIDE + DURATION_WIDE, y);
		m_Health->SetTall(DEFAULT_TALL);
		m_Health->SetWide(HEALTH_WIDE);
		m_Health->SetContentAlignment(vgui::Label::a_center);
	}

	Update();
}

bool TrapRecord::IsValid()
{
	if(!m_Trap)
		return false;

	return true;
}

void TrapRecord::Update()
{
	if(!m_Trap)
	{
		Warning("One record has no trap!\n");
		return;
	}

	if(m_Name)
	{
		// Change label if we hover over the name
		if(m_Name->IsCursorOver())
		{
			m_Name->SetText("Decay");
		}
		else
		{
			m_Name->SetText(m_Trap->GetTrapName());
		}
	}
	if(m_Status)
	{
		if(m_Trap->CanBeDormant())
			m_Status->SetEnabled(true);
		else
			m_Status->SetEnabled(false);

		char status[64];

		if(m_Trap->IsTrapDormant())
		{
			if(m_Trap->IsInInitialDormant())
			{
				Q_strncpy(status, "Waiting...", ARRAYSIZE(status));
			}
			else
			{
				Q_strncpy(status, "Dormant", ARRAYSIZE(status));
			}
		}
		else
		{
			Q_strncpy(status, "Active", ARRAYSIZE(status));
		}

		m_Status->SetText(status);
	}

	if(m_Duration)
	{
		char chDuration[64];

		int maxDuration = m_Trap->GetTrapDefaultLifetime();
		int duration = m_Trap->GetRemainingLifetime();

		Q_snprintf(chDuration, ARRAYSIZE(chDuration), "%i/%i", duration, maxDuration);

		m_Duration->SetText(chDuration);

		if(duration == maxDuration) 
			m_Duration->SetEnabled(false);
		else
			m_Duration->SetEnabled(true);

		// Blinking!
		if(duration <= 10 && !m_Duration->IsBlinking())
			m_Duration->SetBlink(true);
		else if(duration > 10 && m_Duration->IsBlinking())
			m_Duration->SetBlink(false);
	}

	if(m_Health)
	{
		int maxHealth = m_Trap->GetMaxHealth();
		int health = m_Trap->GetHealth();

		char text[64];


		if(health > 0 && maxHealth > 0)
		{
			// Only if we are through the delay
			if(m_flRepairDelay <= gpGlobals->curtime)
			{
				if(health == maxHealth)
					m_Health->SetEnabled(false);
				else
					m_Health->SetEnabled(true);
			}

			if((health <= ((float)maxHealth * 0.25f)) && !m_Health->IsBlinking())
				m_Health->SetBlink(true);
			else if((health > ((float)maxHealth * 0.25f)) && m_Health->IsBlinking())
				m_Health->SetBlink(false);

			Q_snprintf(text, ARRAYSIZE(text), "%i/%i", health, maxHealth);
		}
		else
		{
			m_Health->SetEnabled(false);
			Q_snprintf(text, ARRAYSIZE(text), "N/A");
		}

		m_Health->SetText(text);
	}
}

bool TrapRecord::IsHoveringOver()
{
	if(!m_Number || !m_Health || !m_Parent)
		return false;

	int startX, startY;
	int endX, endY;

	m_Number->GetPos(startX, startY);
	m_Health->GetPos(endX, endY);

	endY += m_Health->GetTall();
	endX += m_Health->GetWide();

	int x, y;
	vgui::input()->GetCursorPosition(x, y);
	m_Parent->ScreenToLocal(x, y);

	// Check x coords first
	if(x >= startX && x <= endX)
	{
		// y now!
		if(y >= startY && y <= endY)
		{
			return true;
		}
	}

	return false;

}

void TrapRecord::HealthRestored()
{
	m_flRepairDelay = gpGlobals->curtime + eo_traps_repair_delay.GetFloat();

	if(m_Health)
		m_Health->SetEnabled(false);
}


// Trap manager implementation
//=============================
ConVar eo_trapmanager_delay("eo_trapmanager_delay", "0.55");
ConVar eo_duration_blink("eo_duration_blink", "10");
ConVar eo_health_blink_factor("eo_health_blink_factor", "0.25");

COverlordTrapManager::COverlordTrapManager(IViewPort * pViewPort) :
vgui::Frame(NULL, PANEL_TRAPMANAGER )
{
	m_flLockTime = 0.0f;
	m_iScaled = -1;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));
	
	SetZPos(1000);

	SetTitle("", true);

	SetMoveable(false);
	SetProportional(true);
	SetSizeable(false);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);

	LoadControlSettings("Resource/UI/TrapManager.res");
	InvalidateLayout();

	ListenForGameEvent( "trap_built" );
	ListenForGameEvent( "trap_decayed" );
	ListenForGameEvent( "trap_destroyed" );

	for(int i = 0; i < 64; i++)
	{
		int yPos = MARGIN + Y_SPACE * i;

		if(yPos + DEFAULT_TALL > GetTall())
		{
			m_LastIndex = i;
			break;
		}
	}
}

COverlordTrapManager::~COverlordTrapManager()
{
	for(int i = 0; i < m_TrapRecord.Count(); i++)
	{
		TrapRecord * record = m_TrapRecord.Element(i);
		delete record;
		m_TrapRecord[i] = NULL;
	}
}

void COverlordTrapManager::OnThink()
{
	// Check whether we should disappear or not
	if(m_flLockTime <= gpGlobals->curtime)
	{
		if(!IsCursorOver())
		{
			gViewPortInterface->ShowPanel(GetName(), false);
		}
	}

	UpdateDisplay();
}

void COverlordTrapManager::FireGameEvent(IGameEvent * event)
{
	if(event)
	{
		if(!Q_stricmp("trap_built", event->GetName()))
		{
			int entindex = event->GetInt("entindex", 0);
			if(entindex)
			{
				C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					AddTrapRecord(C_OverlordTrap::EntityToTrap(pEnt));
				}
			}
		}
		else if(!Q_stricmp("trap_decayed", event->GetName()))
		{
			int entindex = event->GetInt("entindex", 0);
			if(entindex)
			{
				C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					RemoveTrapRecord(pEnt);
				}
			}
		}
		else if(!Q_stricmp("trap_destroyed", event->GetName()))
		{
			int entindex = event->GetInt("entindex", 0);
			if(entindex)
			{
				C_BaseEntity * pEnt = static_cast<C_BaseEntity*>(cl_entitylist->EntIndexToHandle(entindex).Get());
				if(pEnt)
				{
					RemoveTrapRecord(pEnt);
				}
			}
		}
	}
}

void COverlordTrapManager::ApplySchemeSettings(vgui::IScheme * pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundType(2);
}

void COverlordTrapManager::ShowPanel(bool bState)
{
	if(IsVisible() == bState)
	{
		if(bState)
			m_flLockTime = gpGlobals->curtime + eo_trapmanager_delay.GetFloat();

		return;
	}

	if(bState && m_TrapRecord.Count() <= 0)
		return;


	if(bState)
	{
		UpdateDisplay();
		SetVisible(true);
		m_flLockTime = gpGlobals->curtime + eo_trapmanager_delay.GetFloat();
		MoveToFront();
		COverlordCamera::SetMouseRotationEnabled(false);
	}
	else
	{
		SetVisible(false);
		m_flLockTime = 0.0f;
		COverlordCamera::SetMouseRotationEnabled(true);
	}
}

void COverlordTrapManager::AddTrapRecord(C_OverlordTrap * pTrap)
{
	if(!pTrap)
		return;

	COverlordData * pData = GetOverlordData();

	// Scale it if needed
	if(m_iScaled != pData->GetMaxTraps())
	{
		HandleDimensions();
		m_iScaled = pData->GetMaxTraps();
	}

	int index = -1;
	for(int i = 0; i < pData->GetMaxTraps(); i++)
	{
		if(pData->GetTrap(i) == pTrap)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
	{
		Warning("Found a trap not in the list\n");
		return;
	}

	// Find a free spot first
	int yPos = MARGIN + Y_SPACE * index;
	int xPos = MARGIN;
	//if(yPos + DEFAULT_TALL > GetTall())
	//{
	//	xPos = MARGIN + RECORD_SPACE + RECORD_WIDE;
	//	yPos = MARGIN + Y_SPACE * (index - m_LastIndex);
	//}

	TrapRecord * record = new TrapRecord(xPos, yPos, index, pTrap, this);

	if(record)
	{
		m_TrapRecord.AddToTail(record);
		record->Init();
	}
	else
		Warning("Couldn't init trap record!\n");
	
}

void COverlordTrapManager::RemoveTrapRecord(C_BaseEntity * pTrap)
{
	for(int i = 0; i < m_TrapRecord.Count(); i++)
	{
		TrapRecord * record = m_TrapRecord.Element(i);

		if(!record)
			continue;

		if(record->GetTrap() == pTrap)
		{
			delete record;
			
			// Just a safeguard
			m_TrapRecord[i] = NULL;

			m_TrapRecord.Remove(i);
		}
	}

	if(IsVisible() && m_TrapRecord.Count() == 0)
		ShowPanel(false);
}

void COverlordTrapManager::HandleDimensions()
{
	LoadControlSettings("Resource/UI/TrapManager.res");
	COverlordData * pData = GetOverlordData();

	if(pData)
	{
		// Calculate our height
		int maxtraps = pData->GetMaxTraps();

		int capacity = (GetTall() - MARGIN + (Y_SPACE-DEFAULT_TALL))/(Y_SPACE);

		if(maxtraps >= capacity)
			return;

		// Scale it down
		int height = ((maxtraps*Y_SPACE) - (Y_SPACE-DEFAULT_TALL) + MARGIN) + MARGIN;

		//int initialheight = GetTall();
		SetTall(height);

		// Move it down appropriately
		/*int diff = initialheight - height;
		int x, y;
		GetPos(x, y);
		y += diff;
		SetPos(x, y);*/
	}
}

void COverlordTrapManager::UpdateDisplay()
{
	for(int i = 0; i < m_TrapRecord.Count(); i++)
	{
		TrapRecord * record = m_TrapRecord.Element(i);

		if(!record->IsValid())
		{
			Warning("Removing an invalid trap record\n");
			delete record;
			m_TrapRecord.Remove(i);
			i--;
			continue;
		}

		record->Update();
	}
}

C_OverlordTrap * COverlordTrapManager::GetTrapHover()
{
	for(int i = 0; i < m_TrapRecord.Count(); i++)
	{
		TrapRecord * record = m_TrapRecord.Element(i);

		if(!record || !record->IsValid())
			continue;

		if(record->IsHoveringOver())
			return record->GetTrap();
	}

	return NULL;
}

bool COverlordTrapManager::IsShiftPressed() const
{
	return g_pInputSystem->IsButtonDown(KEY_LSHIFT) || g_pInputSystem->IsButtonDown(KEY_LSHIFT);
}

void COverlordTrapManager::OnCommand(const char * command)
{
	// Create the command
	CCommand com;
	com.Tokenize(command);

	COverlordData * pData = GetOverlordData();

	if(!pData)
		return;

	// Index is always second
	int index = atoi(com.Arg(1));
	C_OverlordTrap * pTrap = pData->GetTrap(index);

	if(!pTrap)
		return;
	EHANDLE handle = pTrap;

	if(!Q_stricmp(com.Arg(0), "DecayTrap"))
	{
		if(IsShiftPressed())
		{
			for(int i = 0; i < pData->GetTrapsAmount(); i++)
			{
				if(!pData->GetTrap(i))
					continue;

				engine->ServerCmd(VarArgs("DecayTrap %i", i));
			}
		}
		else
		{
			engine->ServerCmd(VarArgs("DecayTrap %i", index));
		}
	}
	else if(!Q_stricmp(com.Arg(0), "ChangeStatus"))
	{
		if(pTrap->CanBeDormant())
		{
			// 1 = dormant
			int dormant = (pTrap->IsTrapDormant()) ? 0 : 1;

			// If shift is pressed we change the state of all traps
			// in the same state
			if(IsShiftPressed())
			{
				bool isDormant = pTrap->IsTrapDormant();

				for(int i = 0; i < pData->GetTrapsAmount(); i++)
				{
					if(!pData->GetTrap(i) || !pData->GetTrap(i)->CanBeDormant())
						continue;

					if(isDormant == pData->GetTrap(i)->IsTrapDormant())
					{
						handle = pData->GetTrap(i);

						engine->ServerCmd(VarArgs("SetTrapDormant %i %i", dormant, handle.GetEntryIndex()));
					}
				}
			}
			else
			{
				engine->ServerCmd(VarArgs("SetTrapDormant %i %i", dormant, handle.GetEntryIndex()));
			}
		}
	}
	else if(!Q_stricmp(com.Arg(0), "RestoreDuration"))
	{
		if(pTrap->CanRestoreDuration())
		{
			engine->ServerCmd(VarArgs("RestoreDuration %i", index));

			if(IsShiftPressed())
			{
				for(int i = 0; i < pData->GetTrapsAmount(); i++)
				{
					if(!pData->GetTrap(i) || 
						!pData->GetTrap(i)->CanRestoreDuration() ||
						(pData->GetTrap(i) == pTrap))
						continue;

					handle = pData->GetTrap(i);

					engine->ServerCmd(VarArgs("RestoreDuration %i", i));
				}
			}
		}
	}
	else if(!Q_stricmp(com.Arg(0), "RestoreHealth"))
	{
		if(pTrap->CanRestoreHealth())
		{
			engine->ServerCmd(VarArgs("RestoreHealth %i", index));

			if(IsShiftPressed())
			{
				for(int i = 0; i < pData->GetTrapsAmount(); i++)
				{
					if(!pData->GetTrap(i) || 
						!pData->GetTrap(i)->CanRestoreHealth() ||
						(pData->GetTrap(i) == pTrap))
						continue;

					handle = pData->GetTrap(i);

					engine->ServerCmd(VarArgs("RestoreHealth %i", i));
				}
			}	


			// Notify the record!
			/*for(int i = 0; i < m_TrapRecord.Count(); i++)
			{
				TrapRecord * record = m_TrapRecord.Element(i);

				if(!record || !record->IsValid())
					continue;

				if(record->GetTrapIndex() == index)
				{
					record->HealthRestored();
					break;
				}
			}*/
		}
	}
}