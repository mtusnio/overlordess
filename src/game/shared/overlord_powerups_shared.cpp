//==============Overlord mod=====================
//	Client-side implementation of the powerups
//===============================================

#include "cbase.h"
#include "overlord_powerups_shared.h"
#include "hl2mp_gamerules.h"
#ifndef CLIENT_DLL
#include "team.h"
#include "overlord_pathnode.h"
#include "particle_parse.h"
#include "overlord_dynamictraps.h"
#else
#include "c_team.h"
#include "c_overlord_pathnode.h"
#endif

CUtlVector<COverlordPowerup*> g_PowerupsList;

void COverlordPowerup::Init()
{
	m_flCooldownEnd = 0.0f;

#ifdef CLIENT_DLL
	StopListeningForAllEvents();
	ListenForGameEvent("powerup_start");
	ListenForGameEvent("powerup_stop");
#endif
}

void COverlordPowerup::StartPowerup()
{
#ifndef CLIENT_DLL
	IGameEvent * event = gameeventmanager->CreateEvent("powerup_start");

	if(event)
	{
		event->SetString("name", GetName());
		gameeventmanager->FireEvent(event);
	}
#endif

	OnPowerupStart();
}

void COverlordPowerup::StopPowerup()
{
#ifndef CLIENT_DLL

	IGameEvent * event = gameeventmanager->CreateEvent("powerup_stop");
	if(event)
	{
		event->SetString("name", GetName());
		event->SetFloat("cooldownend", GetCooldownEnd());
		gameeventmanager->FireEvent(event);
	}
#endif

	OnPowerupStop();
	SetOnCooldown();
}
#ifndef CLIENT_DLL

void COverlordPowerup::AcceptCommand(const char * command)
{
	if(!Q_stricmp(command, "start"))
	{
		if(!IsActive() && !IsOnCooldown())
		{
			if(GetCost() <= GetOverlordData()->GetFerocity())
			{
				StartPowerup();
				GetOverlordData()->ChangeFerocity(-GetCost());
			}
		}
	}
}

void COverlordPowerup::SendCommand(const CCommand & args)
{
	const char * command = args[2];
	const char * powerup = args[1];

	COverlordPowerup * pPowerup = FindPowerupByName(powerup);

	if(!pPowerup)
	{
		Warning("No powerup %s found\n", powerup);
		return;
	}

	pPowerup->AcceptCommand(command);
}
#endif

void COverlordPowerup::FireGameEvent(IGameEvent * event)
{
	if(!Q_stricmp(event->GetString("name", ""), GetName()))
	{
#ifdef CLIENT_DLL
		if(!Q_stricmp(event->GetName(), "powerup_start"))
		{
			StartPowerup();
		}
		else if(!Q_stricmp(event->GetName(), "powerup_stop"))
		{
			StopPowerup();
		}
#endif
	}

}


COverlordPowerup * COverlordPowerup::FindPowerupByName(const char * name)
{
	for(int i = 0; i < g_PowerupsList.Count(); i++)
	{
		COverlordPowerup * pPowerup = g_PowerupsList[i];

		if(!pPowerup)
		{
			g_PowerupsList.Remove(i);
			i--;
			continue;
		}

		if(!Q_strcmp(pPowerup->GetName(), name))
			return pPowerup;
	}

	return NULL;
}

ConVar eo_powerup_stasis_length("eo_powerup_stasis_length", "8", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_powerup_stasis_cooldown("eo_powerup_stasis_cooldown", "600", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordPowerupStasis : public COverlordPowerup
{
public:
	DECLARE_CLASS(COverlordPowerupStasis, COverlordPowerup);

	COverlordPowerupStasis();

	virtual void Think() { };

	virtual bool ShouldEnd() const { return m_flEndTime <= gpGlobals->curtime; };
	virtual bool IsActive() const { return m_flEndTime != 0.0f; };

	virtual float GetCooldownTime() const { return eo_powerup_stasis_cooldown.GetFloat(); };

	virtual int GetCost() const { return 0; };
private:
	virtual void OnPowerupStart();
	virtual void OnPowerupStop();

	float m_flEndTime;
};

DEFINE_POWERUP("Stasis", stasis, COverlordPowerupStasis);

COverlordPowerupStasis::COverlordPowerupStasis()
{
	m_flEndTime = 0.0f;
}

void COverlordPowerupStasis::OnPowerupStart()
{
	m_flEndTime = gpGlobals->curtime + eo_powerup_stasis_length.GetFloat();

#ifndef CLIENT_DLL
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels)
		return;

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "Stasis enabled!");

	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsAlive() || pPlayer->IsOverlord())
			continue;

		pPlayer->SetInvulnerable(true);
		pPlayer->AddFrozenRef();
	}

#endif
}

void COverlordPowerupStasis::OnPowerupStop()
{
	m_flEndTime = 0.0f;

#ifndef CLIENT_DLL
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(!pRebels)
		return;

	for(int i = 0; i < pRebels->GetNumPlayers(); i++)
	{
		CBasePlayer * pPlayer = pRebels->GetPlayer(i);

		if(!pPlayer || !pPlayer->IsAlive())
			continue;

		pPlayer->SetInvulnerable(false);
		pPlayer->RemoveFrozenRef();
	}
#endif
}

ConVar eo_powerup_surge_length("eo_powerup_surge_length", "8", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordPowerupSurge : public COverlordPowerup
{
public:
	DECLARE_CLASS(COverlordPowerupSurge, COverlordPowerup);
	
	COverlordPowerupSurge();

#ifndef CLIENT_DLL
	virtual void Init();
	virtual void FireGameEvent(IGameEvent * event);
#endif

	virtual void Think() { };

	virtual bool ShouldEnd() const { return m_flEndTime <= gpGlobals->curtime; };
	virtual bool IsActive() const { return m_flEndTime != 0.0f; };

	virtual int GetCost() const { return 50; };
private:
	virtual void OnPowerupStart();
	virtual void OnPowerupStop();

	float m_flEndTime;
#ifndef CLIENT_DLL
	int m_iStartPower;
#endif
};

DEFINE_POWERUP("Power surge", surge, COverlordPowerupSurge);


COverlordPowerupSurge::COverlordPowerupSurge()
{
	m_flEndTime = 0.0f;
#ifndef CLIENT_DLL
	m_iStartPower = 0;
#endif
}
#ifndef CLIENT_DLL
void COverlordPowerupSurge::Init()
{
	BaseClass::Init();

	ListenForGameEvent("trap_built");
}

void COverlordPowerupSurge::FireGameEvent(IGameEvent * event)
{
	BaseClass::FireGameEvent(event);

	if(!IsActive())
		return;

	if(!Q_stricmp(event->GetName(), "trap_built"))
	{
		COverlordTrap * pTrap = static_cast<COverlordTrap*>(UTIL_EntityByIndex(event->GetInt("entindex")));

		if(pTrap)
		{
			pTrap->SetTrapDormant(false);
		}
	}
}


#endif

void COverlordPowerupSurge::OnPowerupStart()
{
	m_flEndTime = gpGlobals->curtime + eo_powerup_surge_length.GetFloat();

#ifndef CLIENT_DLL
	m_iStartPower = GetOverlordData()->GetPower();

	GetOverlordData()->SetPower(GetOverlordData()->GetMaxPower());
#endif

}

void COverlordPowerupSurge::OnPowerupStop()
{
	m_flEndTime = 0.0f;

#ifndef CLIENT_DLL
	if(GetOverlordData()->GetPower() > m_iStartPower)
		GetOverlordData()->SetPower(m_iStartPower);

	m_iStartPower = 0;
#endif
}

ConVar eo_powerup_wave_speed("eo_powerup_wave_speed", "28", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_powerup_wave_damage("eo_powerup_wave_damage", "13", FCVAR_REPLICATED | FCVAR_CHEAT);

class COverlordPowerupWave : public COverlordPowerup
{
public:
	DECLARE_CLASS(COverlordPowerupWave, COverlordPowerup);

	COverlordPowerupWave();
	virtual ~COverlordPowerupWave();

	virtual void Init()
	{
		BaseClass::Init();
		PrecacheParticleSystem("DisruptingWave");
	};

	virtual void Think();

	virtual bool ShouldEnd() const { return (m_Current - m_End).Length() <= 4.0f; };
	virtual bool IsActive() const { return m_Current != vec3_origin; };

	virtual int  GetCost() const { return 60; };

private:
	virtual void OnPowerupStart();
	virtual void OnPowerupStop();

#ifndef CLIENT_DLL
	void MoveForward();

	float m_flNextWave;
#endif
	Vector m_Current;
	Vector m_End;
};

//DEFINE_POWERUP("Disrupting wave", wave, COverlordPowerupWave);

COverlordPowerupWave::COverlordPowerupWave()
{
	m_Current = vec3_origin;
#ifndef CLIENT_DLL
	m_flNextWave = 0.0f;
#endif
}

COverlordPowerupWave::~COverlordPowerupWave()
{
}

void COverlordPowerupWave::OnPowerupStart()
{
	COverlordPathNode * pNode = COverlordPathNode::GetLastNode();

	if(!pNode || !pNode->GetNextNode())
	{
#ifdef CLIENT_DLL
		Warning("No node in wave powerup in CLIENT\n");
#endif
		return;
	}

	m_End = pNode->GetAbsOrigin();
	m_Current = pNode->GetNextNode()->GetAbsOrigin();
}

void COverlordPowerupWave::OnPowerupStop()
{
	m_Current = vec3_origin;
	m_End = vec3_origin;
}

void COverlordPowerupWave::Think()
{
#ifndef CLIENT_DLL
	const float DISTANCE = 128.0f;

	if(m_flNextWave > gpGlobals->curtime)
		return;

	Vector dir = m_End - m_Current;
	VectorNormalize(dir);
	
	for(int i = 0; i < gpGlobals->maxClients; i++)
	{
		CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

		if(!pPlayer || !pPlayer->IsRebel() || !pPlayer->IsAlive())
			continue;


		Vector playerDir = pPlayer->WorldSpaceCenter() - m_Current;
		VectorNormalize(playerDir);

		if(DotProduct(dir, playerDir) < 0.0f)
			continue;

		if((pPlayer->WorldSpaceCenter() - m_Current).Length() <= DISTANCE)
			pPlayer->OnTakeDamage(CTakeDamageInfo(NULL, NULL, eo_powerup_wave_damage.GetFloat(), DMG_GENERIC));

	}

	QAngle angle;
	VectorAngles(dir, angle);
	DispatchParticleEffect("DisruptingWave", m_Current, angle);
	MoveForward();

	m_flNextWave = gpGlobals->curtime + 0.05f;
#endif
}

#ifndef CLIENT_DLL
void COverlordPowerupWave::MoveForward()
{
	Vector dir = m_End - m_Current;
	VectorNormalize(dir);

	const float distance = (m_End - m_Current).Length() < eo_powerup_wave_speed.GetFloat() ? 
		(m_End - m_Current).Length() : eo_powerup_wave_speed.GetFloat();
	m_Current = m_Current + dir * distance;
}
#endif