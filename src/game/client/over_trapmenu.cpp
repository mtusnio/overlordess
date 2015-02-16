//==============Overlord mod=====================
//	Trap menu
//===============================================

#include "cbase.h"
#include "over_trapmenu.h"
#include "overlord_consolemanager.h"
#include <vgui_controls/Label.h>
#include "vgui/IInput.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "overlord_data.h"
#include "gamestringpool.h"

// For the popup button
#include "over_popupsupport.h"

//#include <vgui_controls/Button.h>
#include <vgui_controls/Divider.h>
#include "basemodelpanel.h"


#define BUTTON_WIDE vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 40)
#define BUTTON_TALL vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 40)

#define LABEL_WIDE	vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 64)
#define LABEL_TALL	vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 8)

#define START_DIST_X vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 10)
#define START_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 6)

#define CATEGORY_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9)

#define SPACE_DIST_X vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 6)
#define SPACE_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 5)

#define DIVIDER_SUBTRACT_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 4)

// Sort methods
int SortByCost(const int * rhs, const int * lhs)
{
	C_OverlordTrap::STrapRecord record1 = C_OverlordTrap::GetRecord(*rhs);
	C_OverlordTrap::STrapRecord record2 = C_OverlordTrap::GetRecord(*lhs);

	return (record1.trapCost - record2.trapCost);
}

int SortByDuration(const int * rhs, const int * lhs)
{
	C_OverlordTrap::STrapRecord record1 = C_OverlordTrap::GetRecord(*rhs);
	C_OverlordTrap::STrapRecord record2 = C_OverlordTrap::GetRecord(*lhs);

	return (record1.trapDuration - record2.trapDuration);
}

int SortAlphabetically(const int * rhs, const int * lhs)
{
	C_OverlordTrap::STrapRecord record1 = C_OverlordTrap::GetRecord(*rhs);
	C_OverlordTrap::STrapRecord record2 = C_OverlordTrap::GetRecord(*lhs);

	return (Q_stricmp(record1.buttonName, record2.buttonName));
}

// Offense first, defense next, rest at the end
/*bool SortCategories(const string_t & rhs, const string_t & lhs)
{
	if(!Q_stricmp(STRING(rhs), "Offense"))
		return true;

	if(!Q_stricmp(STRING(lhs), "Offense"))
		return false;


	if(!Q_stricmp(STRING(rhs), "Offense"))
		return true;

	if(!Q_stricmp(STRING(lhs), "Offense"))
		return false;
}*/

// Model button class
class COverlordTrapButton : public CModelPanel, public COverlordPopupSupport
{
public:
	DECLARE_CLASS(COverlordTrapButton, CModelPanel);

	COverlordTrapButton( vgui::Panel *parent, const char *name ) :
	BaseClass(parent, name)
	{
		m_pLabel = new vgui::Label(this, "", "");
	};

	virtual ~COverlordTrapButton()
	{
		if(m_pLabel)
			delete m_pLabel;
	}

	virtual void OnCursorEntered()
	{
		BaseClass::OnCursorEntered();

		COverlordPopupSupport::OnCursorEntered();
	}

	virtual void OnCursorExited()
	{
		BaseClass::OnCursorExited();

		COverlordPopupSupport::OnCursorExited();
	}

	virtual void SetCommand(const char * pcmd)
	{
		Q_strncpy(m_Command, pcmd, ARRAYSIZE(m_Command));
	}

	virtual void OnMousePressed(vgui::MouseCode code)
	{
		BaseClass::OnMousePressed(code);

		// Run our panel's OnCommand. clean it up later though
		vgui::Panel * pPanel = GetParent();
		if(pPanel)
		{
			pPanel->OnCommand(m_Command);
		}
	}

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		if(m_pLabel)
		{
			m_pLabel->SetPos(1, GetTall()-vgui::scheme()->GetProportionalScaledValueEx(GetScheme(),9));
			m_pLabel->SetWide(GetWide());
			m_pLabel->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(),7));
			m_pLabel->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
			m_pLabel->SetContentAlignment(vgui::Label::a_center);
		}
	}

	void SetTrapName(const char * name)
	{
		if(m_pLabel)
			m_pLabel->SetText(name);
	}

private:
	vgui::Label * m_pLabel;
	char m_Command[32];
};
COverlordTrapMenu::COverlordTrapMenu(IViewPort * pViewPort) :
vgui::Frame(NULL, PANEL_TRAPMENU )
{
	SetDefLessFunc(m_Categories);
	SetDefLessFunc(m_Labels);
	m_pViewPort = pViewPort;

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));

	SetTitle("", true);

	SetMoveable(true);
	SetProportional(true);
	SetSizeable(false);

	SetPaintBorderEnabled(true);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);

	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);

	LoadControlSettings("Resource/UI/TrapMenu.res");
	InvalidateLayout();

	for(int i = 0; i < NUM_OF_TRAPS; i++)
		m_Buttons[i] = NULL;
}

COverlordTrapMenu::~COverlordTrapMenu()
{
	ClearCategories();

	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		if(m_Buttons[i])
			delete m_Buttons[i];
	}
}

void COverlordTrapMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		MoveToFront();
		MovePanelToCursor();
		SetVisible(true);
	}
	else
		SetVisible(false);
}

void COverlordTrapMenu::OnThink()
{
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		CModelPanel * pPanel = m_Buttons[i];

		if(!pPanel)
			continue;
		
		C_OverlordData * pData = GetOverlordData();

		if(!pData)
			return;

		// Button index = trap index
		const C_OverlordTrap::STrapRecord & record = C_OverlordTrap::GetRecord(i);


		// Green background!
		if(pData->GetPower() >= record.trapCost)
			pPanel->SetBgColor(Color(0, 80, 0, 160));
		else
			pPanel->SetBgColor(Color(80, 0, 0, 160));
	}
}

void COverlordTrapMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetBorder(pScheme->GetBorder("BaseBorder"));

	CreateCategories();
	BuildTrapButtons(pScheme);

	PaintBackground();
}

void COverlordTrapMenu::ClearCategories()
{
	for(unsigned int i = 0; i < m_Labels.Count(); i++)
	{
		if(m_Labels[i])
		{
			delete m_Labels[i];
			m_Labels[i] = NULL;
		}
	}

	m_Labels.Purge();


	for(int i = 0; i < m_Dividers.Count(); i++)
	{
		if(m_Dividers[i])
		{
			delete m_Dividers[i];
			m_Dividers[i] = NULL;
		}
	}

	m_Dividers.Purge();


	for(unsigned int i = 0; i < m_Categories.Count(); i++)
	{
		if(m_Categories.Element(i))
			delete m_Categories.Element(i);
	}

	m_Categories.Purge();
}

void COverlordTrapMenu::CreateCategories()
{
	ClearCategories();

	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		
		// Our current record
		C_OverlordTrap::STrapRecord record = C_OverlordTrap::GetRecord(i);

		// Check for existing category		
		int index = m_Categories.Count() == 0 ? m_Categories.InvalidIndex() : (m_Categories.Find(record.category));

		if(index == m_Categories.InvalidIndex())
		{
			CUtlVector<int> * blank = new CUtlVector<int>;
			index = m_Categories.Insert(AllocPooledString(record.category), blank);
		}

		m_Categories[index]->AddToTail(i);

		
	}

	for(unsigned int i = 0; i < m_Categories.Count(); i++)
	{
		m_Categories[i]->Sort(SortByCost);
	}
}

/*
#define START_DIST_X vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 8)
#define START_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 8)

#define CATEGORY_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 10)

#define SPACE_DIST_X vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9)
#define SPACE_DIST_Y vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 9)*/


int COverlordTrapMenu::BuildCategory(const char * name, int x, int y)
{
	if(!name)
		return y;

	int index = m_Categories.Find(MAKE_STRING(name));

	if(index == m_Categories.InvalidIndex())
		return y;

	// Add the label
	vgui::Label * pLabel = new vgui::Label(this, name, name);
	pLabel->SetWide(LABEL_WIDE);
	pLabel->SetTall(LABEL_TALL);
	pLabel->SetPos(x, y);
	m_Labels.Insert(pLabel->GetName(), pLabel);

	// Move the buttons lower
	y += SPACE_DIST_Y + LABEL_TALL;

	// Amount of spaces between the buttons, starting with 0
	int xspaces = 0;

	// Start creating the buttons
	const CUtlVector<int> & vTraps = *m_Categories[index];
	for(int i = 0; i < vTraps.Count(); i++)
	{
		int but = vTraps[i];

		if(but >= NUM_OF_TRAPS)
		{
			Warning("Button num exceeds the amount of traps\n");
			continue;
		}

		if(!m_Buttons[but])
		{
			Warning("No buttons when creating categories!\n");
			continue;
		}
		x += (xspaces * (SPACE_DIST_X + BUTTON_WIDE));

		m_Buttons[but]->SetPos(x, y);

		// Create another row if we exceeded the length of the panel
		if((i < vTraps.Count()-1) && ((x + 2*BUTTON_WIDE + SPACE_DIST_X) >= (GetWide() - START_DIST_X)))
		{
			xspaces = 0;
			y += SPACE_DIST_Y + BUTTON_TALL;
			x = START_DIST_X;
		}
		else
		{
			xspaces = 1;
		}
	}


	return (y += BUTTON_TALL + CATEGORY_DIST_Y);

}

void COverlordTrapMenu::BuildTrapButtons(vgui::IScheme *pScheme)
{
	//int row = 0;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		// Our current record
		C_OverlordTrap::STrapRecord record = C_OverlordTrap::GetRecord(i);

		char trapname[30];
		Q_snprintf(trapname, ARRAYSIZE(trapname), "trap%i", i);
		char buttonName[128];
		Q_snprintf(buttonName, ARRAYSIZE(buttonName), "%s", record.buttonName);

		// Delete current buttons
		if(m_Buttons[i])
		{
			delete m_Buttons[i];
			m_Buttons[i] = NULL;
		}
		m_Buttons[i] = new COverlordTrapButton(this, trapname);

		if(!m_Buttons[i])
			continue;

		m_Buttons[i]->SetWide(BUTTON_WIDE);
		m_Buttons[i]->SetTall(BUTTON_TALL);

		// Create command
		char command[sizeof("trap") + 3];

		Q_snprintf(command, ARRAYSIZE(command), "trap %i", i);

		C_OverlordTrap * pTrap = static_cast<C_OverlordTrap*>(CreateEntityByName(record.entName));
		
		if(!pTrap /*|| !(pTrap->InitializeAsClientEntity(pTrap->GetTrapModel(), RENDER_GROUP_OPAQUE_ENTITY))*/)
		{
			Warning("Error while initializing trap in menu\n");
			continue;
		}
		

		// Precache it or find it, the only way atm
		const model_t * model = modelinfo->FindOrLoadModel(pTrap->GetTrapModel());
		if(!model)
		{
			Warning("No model for one of the traps in trapmenu\n");
			continue;
		}
		
		Vector min, max;
		modelinfo->GetModelBounds(model, min, max);
		Vector distance = (max - min) * pTrap->GetPanelDistanceMultiplier();
	
		// Calculate view angle
		QAngle angle = pTrap->GetPanelAngle();


		KeyValues * values = new KeyValues("modelinfo");
		values->SetString("modelname", pTrap->GetTrapModel());
		values->SetFloat("angles_x", angle.x);
		values->SetFloat("angles_y", angle.y);	
		values->SetFloat("angles_z", angle.z);
		
		values->SetFloat("origin_x", distance.Length());
		values->SetFloat("origin_y", pTrap->GetPanelYDistance());
		// Use z position of the centre
		values->SetFloat("origin_z", pTrap->GetPanelZDistance());
		values->SetInt("spotlight", 1);
		

		// Animation data
		KeyValues * animData = new KeyValues("animation");


		values->AddSubKey(animData);

		static_cast<COverlordTrapButton*>(m_Buttons[i])->SetDescription(sTrapDescription(record.description, record.trapCost, record.trapDuration));
		static_cast<COverlordTrapButton*>(m_Buttons[i])->SetCommand(command);
		static_cast<COverlordTrapButton*>(m_Buttons[i])->SetTrapName(record.buttonName);;
		m_Buttons[i]->ParseModelInfo(values);
		m_Buttons[i]->SetBorder(pScheme->GetBorder("BaseBorder"));
		m_Buttons[i]->SetPaintBackgroundEnabled(true);
		m_Buttons[i]->SetPaintBackgroundType(2);
		//m_Buttons[i]->SetBgColor(Color(255, 255, 255));
		//m_Buttons[i]->SetContentAlignment(vgui::Button::a_center);
		//m_Buttons[i]->SetCommand(command);
		pTrap->Remove();

	}

	// Now move the buttons into position
	MoveButtons();
}

void COverlordTrapMenu::MoveButtons()
{
#if 0
	int row = 0;
	for(int i = 0; i < NUM_OF_TRAPS; i++)
	{
		int index = GetButtonIndexByOrder(i);

		if(!m_Buttons[index])
		{
			Warning("No button in trapmenu!\n");
			return;
		}

		int x = BUTTON_MARGIN_X;
		int y = BUTTON_MARGIN_Y + Y_DISTANCE;

		// Move the button
		if(i != 0)
		{
			x += BUTTON_DISTANCE_X * (i - row);
				
			// Move down if the line is full
			if((x + BUTTON_WIDE) > (GetWide() - BUTTON_MARGIN_X))
			{		
				row = i;
				//row = i;
				x = BUTTON_MARGIN_X;

				// Set relative y pos
				{
					int y0;
					int x0;

					m_Buttons[GetButtonIndexByOrder(i-1)]->GetPos(x0, y0);
					y = BUTTON_DISTANCE_Y + y0;
				}

				// Abort the creation if the button is out of bounds and make it invisible
				if((y+BUTTON_TALL) > (GetTall() - BUTTON_MARGIN_Y))
				{
					/*
					delete m_Buttons[index];
					m_Buttons[index] = NULL;
					break;*/

					for(int j = i; j < NUM_OF_TRAPS; j++)
						m_Buttons[GetButtonIndexByOrder(j)]->SetVisible(false);

					break;
				}
			}

			// Now y position if row isn't 0 and isn't this button
			if(row != 0 && row != i)
			{
				{
					int y0;
					int x0;

					m_Buttons[GetButtonIndexByOrder(row)]->GetPos(x0, y0);
					y = y0;
				}
			}
		}

		//DevMsg("X: %i, Y: %i, GetWide(): %i\n", x, y, GetWide());

		m_Buttons[index]->SetPos(x, y);
		m_Buttons[index]->SetVisible(true);

	}
#endif
	int startX = START_DIST_X;
	int startY = START_DIST_Y;

	for(unsigned int i = 0; i < m_Categories.Count(); i++)
	{
		startY = BuildCategory(STRING(m_Categories.Key(i)), startX, startY);

		if(i != m_Categories.Count()-1)
		{
			int divY = startY - DIVIDER_SUBTRACT_Y;

			vgui::Divider * pDivider = new vgui::Divider(this, "");
			m_Dividers.AddToTail(pDivider);

			pDivider->SetPos(0, divY);
			pDivider->SetWide(GetWide());
		}
	}
}

int COverlordTrapMenu::GetButtonIndexByOrder(const char * category, int order)
{
	int index = m_Categories.Find(MAKE_STRING(category));

	if(index == m_Categories.InvalidIndex())
		return order;

	return (*m_Categories[index])[order];
}

const char * COverlordTrapMenu::GetButtonCategory(int i) const
{
	return COverlordTrap::GetRecord(i).category;
}

void COverlordTrapMenu::MovePanelToCursor()
{
	int x, y;
	vgui::input()->GetCursorPos(x, y);

	const CViewSetup * setup = view->GetViewSetup();

	if(!setup)
		return;

	// Center to mouse
	SetPos(x - (GetWide()/2), y - (GetTall()/2));


	// Check whether it has not crossed screen boundries
	int xpos, ypos;
	GetPos(xpos, ypos);

	// X axis first
	if(xpos < 0)
	{
		// Crossed left, just align with 1
		xpos = 1;
	}
	else if((xpos + GetWide()) > setup->width)
	{
		// Crossed right
		int diff = xpos + GetWide() - setup->width;

		// Subtract needed amount
		xpos -= diff;
	}

	if((ypos + GetTall()) > setup->height)
	{
		// Too low
		int diff = ypos + GetTall() - setup->height;

		ypos -= diff;
	}
	else if(ypos < 0)
	{
		ypos = 1;
	}


	SetPos(xpos, ypos);
}


void COverlordTrapMenu::OnCommand(const char * command)
{
	const int STRINGLENGTH = 4;

	char com[64];
	Q_strncpy(com, command, ARRAYSIZE(com));

	if(!Q_strnicmp(com, "trap", STRINGLENGTH))
	{
		CCommand cmd;
		cmd.Tokenize(command);

		g_ConsoleManager.BuildGhost(C_OverlordTrap::GetRecord(atoi(cmd.Arg(1))).entName);

		Close();
	}
}