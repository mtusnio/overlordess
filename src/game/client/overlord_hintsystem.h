//==============Overlord mod=====================
//	Displays and manages hint panels
//===============================================

#ifndef C_H_OV_HINTSYSTEM
#define C_H_OV_HINTSYSTEM

class COverlordHintPanel;
class C_OverlordHintEntity;

typedef bool (*ShouldLaunchHint)();

#include "GameEventListener.h"

class COverlordHintSystem : public CGameEventListener
{
public:
	DECLARE_CLASS_NOBASE(COverlordHintSystem);

	COverlordHintSystem();
	~COverlordHintSystem();

	void Init();
	void Purge();
	void Run();

	bool IsInEntityList(C_BaseEntity * pEnt) const;
	void InsertHintEntity(C_OverlordHintEntity * pEnt)
	{
		for(int i = 0; i < m_HintEntities.Count(); i++)
		{
			if(pEnt == m_HintEntities[i])
			{
				// Already added
				return;
			}
		}
		m_HintEntities.AddToTail(pEnt);
	}

	virtual void FireGameEvent(IGameEvent *event);

	// Does not show hints, use -1 for permanent suppression (until it's lifted)
	void SuppressHints(float flTime);

	bool		 CreateGlobalFile() const;

	// Remember to close!
	FileHandle_t OpenGlobalFile() const;

	virtual bool ShouldShowHints() const;
private:
	struct HintEvent_t
	{
		HintEvent_t(ShouldLaunchHint function, int x, int y, const char * hint, int playerTeam)
		{
			sFunc = function;
			sX = x;
			sY = y;
			Q_strncpy(sHint, hint, ARRAYSIZE(sHint));
			bFired = false;
			team = playerTeam;
		}

		ShouldLaunchHint sFunc;
		int sX;
		int sY;
		char sHint[128];
		bool bFired;
		int team;
	};

	// Hints for entities to display
	struct EntityHint_t
	{
		EntityHint_t(const char * rhshint, const char * rhsclassName, int rhsteam);

		char className[128];
		char hint[256];
		int team;
	};

	void CreateKeyValueForEvent(HintEvent_t event, int index);
	void CreateEvents();

	void UpdateHints();

	void DispatchPanel(const char * hint, const Vector * location);

	// Don't place it in worldpsace but in our local space
	void DispatchPanel(const char * hint, int x, int y);

	// Our panels
	CUtlVector<COverlordHintPanel*> m_Panels;
	// Entities already hinted
	CUtlVector<EHANDLE> m_Entities;
	// Our list of hints we need to check
	CUtlVector<HintEvent_t> m_Events;
	CUtlVector<EntityHint_t> m_EntityEvents;
	// Hint entity lists. DO NOT PURGE IT!!!!
	CUtlVector<CHandle<C_OverlordHintEntity>> m_HintEntities;

	
	float m_flNextUpdate;
	// 0.0f - no suppression, anything other is gametime
	float m_flSuppressTime;
};

extern COverlordHintSystem g_HintManager;

// The hint entity
class C_OverlordHintEntity : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordHintEntity, C_BaseEntity);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	C_OverlordHintEntity() { };
	virtual ~C_OverlordHintEntity() { };

	virtual void Spawn()
	{
		BaseClass::Spawn();

		g_HintManager.InsertHintEntity(this);

		m_bDisplayed = false;
	}

	bool		 WasDisplayed() const { return m_bDisplayed; };
	const char * GetHint() const { return m_Hint; };
	int			 GetHintTeam() const { return m_Team; };

	void		 SetDisplayed(bool bDisplayed) { m_bDisplayed = bDisplayed; };

	bool		 IsHintEnabled() const { return m_bEnabled; };
private:
	// Networked
	char m_Hint[256];
	int m_Team;

	bool m_bEnabled;
	bool m_bDisplayed;
};
#endif