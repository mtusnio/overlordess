//==============Overlord mod=====================
//	Base interface for the powerups (both server & client)
//===============================================

#ifndef H_OV_POWERUPS
#define H_OV_POWERUPS
	
#ifdef CLIENT_DLL
#define COverlordPowerup C_OverlordPowerup
#endif

#include "GameEventListener.h"

class COverlordPowerup
	: public CGameEventListener
{
public:
	DECLARE_CLASS_NOBASE( COverlordPowerup );

	COverlordPowerup() 
	{
	
	}
	virtual ~COverlordPowerup() { }

	virtual void Init();

	const char * GetName() const { return m_Name; };
	void SetName(const char * name) { Q_strncpy(m_Name, name, ARRAYSIZE(m_Name)); };
	void SetDisplayName(const char * name) { Q_strncpy(m_DisplayName, name, ARRAYSIZE(m_DisplayName)); };
	const char * GetDisplayName() const { return m_DisplayName; };

	virtual void   Think() = 0;

	virtual bool   ShouldEnd() const = 0;

	virtual bool   IsActive() const = 0;
	
	virtual int	   GetCost() const = 0;

	void   StartPowerup();
	void   StopPowerup();

	virtual bool   IsOnCooldown() const { return m_flCooldownEnd > gpGlobals->curtime; };
	virtual float  GetCooldownTime() const { return 4.0f; };
	
	virtual float  GetCooldownEnd() const { return m_flCooldownEnd; };
	virtual void   SetOnCooldown() { m_flCooldownEnd = gpGlobals->curtime + GetCooldownTime(); };

#ifndef CLIENT_DLL
	virtual void AcceptCommand(const char * command);

	static void SendCommand(const CCommand & args);
#endif
	virtual void FireGameEvent(IGameEvent * event);

	static COverlordPowerup * FindPowerupByName(const char * name);
private:
	virtual void   OnPowerupStart() = 0;
	virtual void   OnPowerupStop() = 0;

	float m_flCooldownEnd;

	char m_Name[32];
	char m_DisplayName[32];

};

// Powerups list
extern CUtlVector<COverlordPowerup*> g_PowerupsList;

#define DEFINE_POWERUP(displayName, localName, className) class C##localName##FooPowerup										\
	{																														    \
	public:																											            \
		C##localName##FooPowerup()																								\
		{																														\
			COverlordPowerup * pPowerup = new className();																		\
			if(!pPowerup)																										\
				return;																											\
			pPowerup->SetName(#localName);																						\
			pPowerup->SetDisplayName(displayName);																				\
			g_PowerupsList.AddToTail(pPowerup);																					\
		}																														\
																																\
		~C##localName##FooPowerup()																								\
		{																														\
			for(int i = 0; i < g_PowerupsList.Count(); i++)																		\
			{																													\
				COverlordPowerup * pPowerup = g_PowerupsList[i];																\
				if(!pPowerup || Q_stricmp(pPowerup->GetName(), #localName))														\
					continue;																									\
				delete pPowerup;																								\
				g_PowerupsList.Remove(i);																						\
				break;																											\
			}																													\
		}																														\
	};																															\
	C##localName##FooPowerup g_##localName##PowerupFactory;	
#endif

