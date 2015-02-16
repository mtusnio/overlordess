//==============Overlord mod=====================
//	Displays and manages hint panels
//===============================================

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <game/client/iviewport.h>
#include "overlord_hintsystem.h"
#include "hl2mp_gamerules.h"
#include "overlord_dynamictraps.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "view_scene.h"
#include <baseviewport.h>
#include "overlord_hintevents.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "tier3/tier3.h"
#include "tier2/tier2.h"
#include "tier0/vprof.h"

#define SYSTEM_UPDATE_TIME 0.3f
#define CHAR_MULTIPLIER 0.12f
#define HINT_SOUND ""

#define DEATH_DELAY 2.0f

#define FILE_PATH "resource/hints.txt"

// Hint entity tables
BEGIN_PREDICTION_DATA(C_OverlordHintEntity)

END_PREDICTION_DATA()

IMPLEMENT_CLIENTCLASS_DT(C_OverlordHintEntity, DT_OverlordHintEntity, COverlordHintEntity)

	RecvPropString(RECVINFO(m_Hint)),
	RecvPropInt(RECVINFO(m_Team)),
	RecvPropBool(RECVINFO(m_bEnabled)),

END_RECV_TABLE()

LINK_ENTITY_TO_CLASS(over_hintentity, C_OverlordHintEntity);

COverlordHintSystem::EntityHint_t::EntityHint_t(const char *rhshint, const char *rhsclassName, int rhsteam)
{
	if(rhshint[0] == '#')
	{
		if(g_pVGuiLocalize->Find(rhshint))
		{
			wchar_t wchar[ARRAYSIZE(hint)];
			g_pVGuiLocalize->ConstructString(wchar, sizeof(wchar), g_pVGuiLocalize->Find(rhshint), 0);

			g_pVGuiLocalize->ConvertUnicodeToANSI(wchar, hint, sizeof(hint));
		}
		else
			Q_strncpy(hint, "!!ERROR. HINT NOT FOUND!!", ARRAYSIZE(hint));
	}
	else
	{
		Q_strncpy(hint, rhshint, ARRAYSIZE(hint));
	}

	Q_strncpy(className, rhsclassName, ARRAYSIZE(className));
	team = rhsteam;
}

ConVar eo_disablehints("eo_disablehints", "1", FCVAR_HIDDEN, "Disable ingame hints.");

// A hack to let us use vguilocalize
#define REGISTER_HINT_TEXT(function, x, y, text, team)	 \
	{													 \
		HintEvent_t hint(function, x, y, text, team);    \
		m_Events.AddToTail(hint);						 \
		int index = m_Events.Count() - 1;				 \
		CreateKeyValueForEvent(hint, index);			 \
	}				

#define REGISTER_HINT(function, x, y, localize, team)									      \
	{																				  \
		wchar_t wszString[128];														  \
		char string[ARRAYSIZE(wszString)];											  \
		g_pVGuiLocalize->ConstructString( wszString, sizeof(wszString), g_pVGuiLocalize->Find(localize), 0); \
		g_pVGuiLocalize->ConvertUnicodeToANSI(wszString, string, sizeof(string));	  \
		REGISTER_HINT_TEXT(function, x, y, string, team)									  \
	}

#define REGISTER_ENTITY(hint, entityname, team)						\
	{																\
		EntityHint_t entityhint(hint, entityname, team);			\
		m_EntityEvents.AddToTail(entityhint);						\
	}																\

class COverlordHintPanel : public vgui::Panel, public IViewPortPanel
{
public:
	DECLARE_CLASS_SIMPLE(COverlordHintPanel, vgui::Panel);

	COverlordHintPanel(IViewPort * pViewPort);
	virtual ~COverlordHintPanel();

	virtual const char * GetName() { return "HintPanel"; };

	void SetupPanel(const char * hint, float lifetime, const Vector & location);
	void SetupPanel(const char * hint, float lifetime, int x, int y);

	virtual void ApplySchemeSettings(vgui::IScheme * pScheme);

	virtual void OnTick();

	float GetDeathTime() { return m_flDeathTime; };

	virtual void ShowPanel(bool bShow) 
	{
		if(IsVisible() == bShow)
			return;

		SetVisible(bShow);
	}
	virtual void SetData(KeyValues *data) {};
	virtual void Reset() { };
	virtual void Update() { };

	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return false; }

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	float GetHintDeathTime() const { return m_flDeathTime; };
private:
	IViewPort * m_pViewPort;
	vgui::Label * m_pLabel;

	// Reference needed for moving traps
	const Vector * m_Location;

	int m_x;
	int m_y;

	float m_flDeathTime;
};

COverlordHintSystem g_HintManager;

COverlordHintPanel::COverlordHintPanel(IViewPort * pViewPort) :
vgui::Panel(NULL)
{
	m_pViewPort = pViewPort;

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(true);

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetEnabled(true);
	SetProportional(true);

	InvalidateLayout();


	m_pLabel = new vgui::Label(this, "", "");

	m_x = -1;
	m_y = -1;

	C_BasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();

	if(pLocal)
	{
		if(pLocal->IsOverlord())
			SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/OverlordessScheme.res", "OverlordessScheme"));
		else
			SetScheme("ClientScheme");
	}

	m_Location = &vec3_origin;
}

COverlordHintPanel::~COverlordHintPanel()
{
}

void COverlordHintPanel::SetupPanel(const char * hint, float lifetime, const Vector & location)
{
	ApplySchemeSettings(vgui::scheme()->GetIScheme(GetScheme()));

	m_Location = &location;

	SetVisible(true);

	m_flDeathTime = gpGlobals->curtime + lifetime;
	
	SetParent(static_cast<CBaseViewport*>(gViewPortInterface)->GetVPanel());

	if(m_pLabel)
	{
		wchar_t tempString[256];
		g_pVGuiLocalize->ConvertANSIToUnicode( hint, tempString, sizeof(tempString)  );

		int length, textheight;
		surface()->GetTextSize(m_pLabel->GetFont(), tempString, length, textheight);
		// Setup size
		SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 19));
		SetWide(length + vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 12));

		m_pLabel->SetPos(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 6), vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 5));
		m_pLabel->SetTall(vgui::scheme()->GetProportionalScaledValueEx(GetScheme(), 8));
		m_pLabel->SetWide(length);
		m_pLabel->SetText(hint);
		//m_pLabel->SetContentAlignment(vgui::Label::a_center);
	}

}

void COverlordHintPanel::SetupPanel(const char *hint, float lifetime, int x, int y)
{
	// Call usual function but set the position up here
	SetupPanel(hint, lifetime, vec3_origin);

	const CViewSetup * setup = view->GetViewSetup();

	if(!setup)
		return;

	m_x = ((float)x/640) * setup->width;
	m_y = ((float)y/480) * setup->height;

	SetPos(m_x, m_y);
}
void COverlordHintPanel::ApplySchemeSettings(vgui::IScheme * pScheme)
{
	SetPaintBackgroundType(2);

	BaseClass::ApplySchemeSettings(scheme()->GetIScheme(GetScheme()));

	if(m_pLabel)
	{
		m_pLabel->SetFont(pScheme->GetFont("DefaultVerySmall"));
	}
}

void COverlordHintPanel::OnTick()
{
	if(m_x != -1 && m_y != -1)
		return;

	// Update position here
	const CViewSetup * setup = view->GetViewSetup();
	if(!setup)
		return;

	const int wide = setup->width;
	const int tall = setup->height;

	Vector screen;
	ScreenTransform(*m_Location, screen);

	if((screen.x == 0 && screen.y == 0) 
		|| (screen.x < -1.0f || screen.x > 1.0f || screen.y > 1.0f || screen.y < -1.0f))
	{
		SetVisible(false);
		return;
	}
	else
	{
		SetVisible(true);
	}

	int yCenter = tall / 2;
	int xCenter = wide / 2;

	int x = xCenter + (xCenter * screen.x);
	int y = yCenter - (yCenter * screen.y);

	x -= GetWide()/2;
	y -= GetTall()/4;
	SetPos(x, y);
}

//###########################################################################################

COverlordHintSystem::COverlordHintSystem()
{
	m_flNextUpdate = 0.0f;
	m_flSuppressTime = 0.0f;
}

COverlordHintSystem::~COverlordHintSystem()
{
	for(int i = m_Panels.Count()-1; i >= 0; --i)
	{
		if(!m_Panels[i])
			continue;

		delete m_Panels[i];
		m_Panels[i] = NULL;
	}
	m_Panels.Purge();

	m_Entities.Purge();

	m_Events.Purge();
}

void COverlordHintSystem::Init()
{
	ListenForGameEvent("eo_suppresshints");
}

void COverlordHintSystem::Purge()
{
	for(int i = m_Panels.Count()-1; i >= 0; --i)
	{
		if(!m_Panels[i])
			continue;

		delete m_Panels[i];
		m_Panels[i] = NULL;
	}
	m_Panels.Purge();

	m_Entities.Purge();

	m_Events.Purge();

	CreateEvents();
}

void COverlordHintSystem::CreateEvents()
{
	// Init the file
	CreateGlobalFile();

	static bool bEntitiesRegistered = false;

	REGISTER_HINT(AccessedConsole, 20, 120, "#EO_ConsoleHint01", TEAM_OVERLORD);
	REGISTER_HINT(AccessedConsole, 20, 150, "#EO_ConsoleHint02", TEAM_OVERLORD);
	REGISTER_HINT(AccessedSentryGun, 20, 180, "#EO_SentryHint", TEAM_OVERLORD);
	REGISTER_HINT(BuildMenuOpen, 20, 210, "#EO_BuildHint01", TEAM_OVERLORD);
	REGISTER_HINT(BuildingTrap, 20, 250, "#EO_BuildHint02", TEAM_OVERLORD);
	REGISTER_HINT(PlayerKilled, 20, 150, "#EO_PlayerKilledOv", TEAM_OVERLORD);
	
	REGISTER_HINT(PlayerKilled, 20, 210, "#EO_PlayerKilled", TEAM_REBELS);

	if(!bEntitiesRegistered)
	{
		bEntitiesRegistered = true;

		REGISTER_ENTITY("#EO_SpawnerHint", "over_spawner", TEAM_REBELS);
		REGISTER_ENTITY("#EO_DoorLockHint", "over_doorconsole", TEAM_REBELS);
	}
	
}

void COverlordHintSystem::Run()
{
	VPROF_BUDGET("COverlordHintSystem::Run", VPROF_BUDGETGROUP_OTHER_VGUI);

	// Update panels	
	for(int i = m_Panels.Count()-1; i >= 0; --i)
	{
		if(!m_Panels[i])
			continue;

		if((m_Panels[i]->GetDeathTime() + DEATH_DELAY) <= gpGlobals->curtime)
		{
			delete m_Panels[i];
			m_Panels[i] = NULL;
			m_Panels.Remove(i);
			continue;
		}
		else if(m_Panels[i]->GetDeathTime() <= gpGlobals->curtime)
		{
			int alpha = m_Panels[i]->GetAlpha() - 4;
			m_Panels[i]->SetAlpha((alpha < 0) ? 0 : alpha);
		}
		m_Panels[i]->OnTick();		
	}

	// Don't update every frame
	if(m_flNextUpdate > gpGlobals->curtime)
		return;

	if(m_flSuppressTime != 0.0f && m_flSuppressTime <= gpGlobals->curtime)
		m_flSuppressTime = 0.0f;

	// Make sure our player EXISTS before we start building hints
	if(ShouldShowHints())
		UpdateHints();

	// Clean up the entity list so we don't end up with a huge vector
	for(int i = 0; i < m_Entities.Count(); i++)
	{
		if(!m_Entities[i])
			m_Entities.Remove(i);
	}

	m_flNextUpdate = gpGlobals->curtime + SYSTEM_UPDATE_TIME;
}

void COverlordHintSystem::DispatchPanel(const char * hint, const Vector * location)
{
	// No location. Return
	if(!location)
		return;

	COverlordHintPanel * pPanel = new COverlordHintPanel(gViewPortInterface);

	if(!pPanel)
	{
		Warning("Couldn't allocate hint panel resources!\n");
		return;
	}

	float lifetime = 0.0f;
	int length = Q_strlen(hint);

	lifetime = (float)length * CHAR_MULTIPLIER;

	pPanel->SetupPanel(hint, lifetime, *location);

	m_Panels.AddToTail(pPanel);
}

void COverlordHintSystem::DispatchPanel(const char * hint, int x, int y)
{
	COverlordHintPanel * pPanel = new COverlordHintPanel(gViewPortInterface);

	if(!pPanel)
	{
		Warning("Couldn't allocate hint panel resources!\n");
		return;
	}

	float lifetime = 0.0f;
	int length = Q_strlen(hint);

	lifetime = (float)length * CHAR_MULTIPLIER;

	// Make sure the life time is dependant on the life time of existing panels (if that makes sense)
	float highest = 0.0f;
	for(int i = m_Panels.Count() - 1; i >= 0; i--)
	{
		if(!m_Panels[i])
			continue;

		if(m_Panels[i]->GetDeathTime() > highest)
		{
			highest = m_Panels[i]->GetDeathTime();
		}
	}

	if(highest != 0.0f)
		lifetime += highest - gpGlobals->curtime;

	pPanel->SetupPanel(hint, lifetime, x, y);

	m_Panels.AddToTail(pPanel);
}

void COverlordHintSystem::UpdateHints()
{
	C_BasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();

	// Oh, and we want the player to be ALIVE
	if(!pLocal->IsAlive() || pLocal->GetTeamNumber() == TEAM_SPECTATOR)
		return;

	GET_OVERLORD_DATA_ASSERT(pData);

	// Trap hints dispatch
	const int MAX_TRAPS = pData->GetMaxTraps();
	for(int i = 0; i < MAX_TRAPS; i++)
	{
		C_OverlordTrap * pTrap = pData->GetTrap(i);

		if(!pTrap)
			continue;

		const char * hint = pTrap->GetHint();

		if(hint)
		{
			if(IsInEntityList(pTrap))
				continue;

			// Check the distance!
			if((pLocal->RealEyePosition() - pTrap->WorldSpaceCenter()).Length() > 1024)
				continue;

			// Check whether we can see the trap at all
			trace_t tr;
			UTIL_TraceLine(pLocal->RealEyePosition(), pTrap->WorldSpaceCenter(), MASK_SHOT_HULL, pLocal, COLLISION_GROUP_NONE, &tr);
			
			if(!tr.m_pEnt)
				continue;
			
			
			if(tr.m_pEnt->IsPlayer())
			{
				UTIL_TraceLine(tr.endpos, pTrap->WorldSpaceCenter(), MASK_SHOT_HULL, tr.m_pEnt, COLLISION_GROUP_NONE, &tr);
			}

			if(tr.m_pEnt != pTrap)
				continue;


			DispatchPanel(hint, &pTrap->GetAbsOrigin());
			m_Entities.AddToTail(pTrap);
		}
	}

	// Check for events
	for(int i = 0; i < m_Events.Count(); i++)
	{
		HintEvent_t & hint = m_Events[i];

		if(!hint.sFunc || hint.bFired)
			continue;

		if(hint.sFunc() && (hint.team == 0 || hint.team == pLocal->GetTeamNumber()))
		{
			hint.bFired = true;
			DispatchPanel(hint.sHint, hint.sX, hint.sY);
		}
	}

	// Check for predefined entity hints
		
	// Iterating through whole list. Some better solution?
	for(int i = 0; i <= ClientEntityList().GetHighestEntityIndex(); i++)
	{
		C_BaseEntity * pEnt = ClientEntityList().GetBaseEntity(i);
		if(!pEnt)
			continue;

		if((pEnt->WorldSpaceCenter() - pLocal->RealEyePosition()).Length() > 768)
			continue;
		
		if(IsInEntityList(pEnt))
			continue;

		trace_t tr; 
		UTIL_TraceLine(pLocal->RealEyePosition(), pEnt->WorldSpaceCenter(), MASK_SHOT_HULL, pLocal, COLLISION_GROUP_NONE, &tr);

		if(!tr.m_pEnt)
			continue;

		if(tr.m_pEnt->IsPlayer())
		{
			UTIL_TraceLine(tr.m_pEnt->RealEyePosition(), pEnt->WorldSpaceCenter(), MASK_SHOT_HULL, tr.m_pEnt, COLLISION_GROUP_NONE, &tr);
		}

		if(tr.m_pEnt != pEnt)
			continue;

		for(int j = 0; j < m_EntityEvents.Count(); j++)
		{
			if((m_EntityEvents[j].team == 0 || m_EntityEvents[j].team == pLocal->GetTeamNumber()) 
				&& !Q_stricmp(pEnt->GetClassname(), m_EntityEvents[j].className))
			{
				DispatchPanel(m_EntityEvents[j].hint, &pEnt->GetAbsOrigin());
				m_Entities.AddToTail(pEnt);
			}
		}
	}

	// Now check existing hint entities
	for(int i = 0; i < m_HintEntities.Count(); i++)
	{
		if(!m_HintEntities[i])
		{
			m_HintEntities.Remove(i);
			continue;
		}

		if(!m_HintEntities[i]->IsHintEnabled())
			continue;

		if(m_HintEntities[i]->WasDisplayed() || 
			((m_HintEntities[i]->GetHintTeam() != 0) && 
			m_HintEntities[i]->GetHintTeam() != pLocal->GetTeamNumber()))
			continue;

		if((m_HintEntities[i]->GetAbsOrigin() - pLocal->RealEyePosition()).Length() > 512)
			continue;

		trace_t tr;
		UTIL_TraceLine(pLocal->RealEyePosition(), m_HintEntities[i]->GetAbsOrigin(), MASK_SHOT_HULL, pLocal, COLLISION_GROUP_NONE, &tr);

		if(tr.fraction <= 0.95)
			continue;

		m_HintEntities[i]->SetDisplayed(true);

		char hint[256];
		Q_strncpy(hint, m_HintEntities[i]->GetHint(), ARRAYSIZE(hint));

		DispatchPanel(hint, &m_HintEntities[i]->GetAbsOrigin());
	}
}

bool COverlordHintSystem::IsInEntityList(C_BaseEntity * pEnt) const
{
	for(int i = 0; i < m_Entities.Count(); i++)
	{
		if(m_Entities[i] == pEnt)
		{
			return true;
		}
	}

	return false;
}


// Our filesystem
bool COverlordHintSystem::CreateGlobalFile() const
{
	FileHandle_t f = g_pFullFileSystem->Open(FILE_PATH, "r+");

	if(f)
	{
		g_pFullFileSystem->Close(f);
		return true;
	}
	else
	{
		f = g_pFullFileSystem->Open(FILE_PATH, "w+");

		if(!f)
		{
			Warning("Cannot create hints file!\n");
			return false;
		}
		else
		{
			return true;
		}
	}


}

FileHandle_t COverlordHintSystem::OpenGlobalFile() const
{
	//const char path[] = FILE_PATH;
	FileHandle_t f = g_pFullFileSystem->Open(FILE_PATH, "r+");

	if(!f)
	{
		Warning("Cannot open hints text file\n");
		return NULL;
	}

	return f;
}

bool COverlordHintSystem::ShouldShowHints() const
{
	return !eo_disablehints.GetBool() && 
		C_BasePlayer::GetLocalPlayer() && 
		m_flSuppressTime == 0.0f &&
		!GetOverlordData()->IsInTutorial();
}

void COverlordHintSystem::CreateKeyValueForEvent(HintEvent_t event, int index)
{
	FileHandle_t f = OpenGlobalFile();

	if(f)
	{
		KeyValues * key = new KeyValues("HINTS", "Key", 5);

		CUtlBuffer buf;
		
		key->ReadAsBinary(buf);

		g_pFullFileSystem->Write(buf.Base(), buf.TellPut(), f);
		g_pFullFileSystem->Close(f);

		key->deleteThis();
	}
	else
	{
		Warning("Cannot open hints file for keyvalue editing!\n");
	}
}

void COverlordHintSystem::FireGameEvent(IGameEvent *event)
{
	if(!event)
		return;

	const char * name = event->GetName();

	if(!Q_stricmp(name, "eo_suppresshints"))
	{
		int userid = event->GetInt("userid", -1);

		if((userid == -1) || (C_BasePlayer::GetLocalPlayer()->GetUserID() == userid))
			SuppressHints(event->GetFloat("delay"));
	}
}

void COverlordHintSystem::SuppressHints(float flTime)
{
	m_flSuppressTime = gpGlobals->curtime + flTime;
}

CON_COMMAND_F(eo_suppresshints, "Suppress hint system for 5 seconds", FCVAR_CHEAT)
{
	C_BasePlayer * pLocal = C_BasePlayer::GetLocalPlayer();

	if(!pLocal)
		return;

	IGameEvent * event = gameeventmanager->CreateEvent("eo_suppresshints");

	if(event)
	{
		event->SetInt("userid", pLocal->GetUserID());
		event->SetFloat("delay", 5.0f);
		gameeventmanager->FireEventClientSide(event);
	}
}