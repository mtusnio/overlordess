//==============Overlord mod=====================
//	Door console
//	
//===============================================

#include "cbase.h"
#include "overlord_doorconsole.h"
#include "overlord_area.h"
#include "hl2mp_gamerules.h"
#include "team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CHandle<COverlordDoorLock> COverlordDoorLock::m_LastHacked = NULL;

#define BOX_SIZE 1024
#define HACK_INTER NEXT_HACK + 0.50f
#define HACK_SOUND "Streetwar.d3_C17_13_beep"
#define DENY_SOUND "HL2Player.UseDeny"
#define ALARM_SOUND "General.BaseAlarm"

#define HACK_SOUND_LENGTH 2.0f
#define DENY_SOUND_LENGTH 4.0f

#define SF_DO_NOT_WARN 8
#define SF_LINE_OF_SIGHT 16
#define SF_NO_HINT 32

//ConVar eo_doorlock_alarm_duration("eo_doorlock_alarm_duration", "3.5f", FCVAR_REPLICATED | FCVAR_CHEAT);

ConVar eo_points_for_console("eo_points_for_console", "25", FCVAR_REPLICATED | FCVAR_CHEAT);

BEGIN_DATADESC(COverlordDoorLock)

	DEFINE_USEFUNC(ConsoleUse),
	DEFINE_KEYFIELD(m_flMaximumHackTime, FIELD_FLOAT, "HackTime"),
	DEFINE_KEYFIELD(m_flMinimumHackTime, FIELD_FLOAT, "MinimumHackTime"),

	DEFINE_KEYFIELD(m_PercentageAlive, FIELD_INTEGER, "PercentageAlive"),
	DEFINE_KEYFIELD(m_PlayerDistance, FIELD_INTEGER, "playerdistance"),

	DEFINE_KEYFIELD(m_iszAreaEntName[0], FIELD_STRING, "AreaName01"),
	DEFINE_KEYFIELD(m_iszAreaEntName[1], FIELD_STRING, "AreaName02"),

	DEFINE_KEYFIELD(m_HackingString, FIELD_STRING, "HackingString"),
	DEFINE_KEYFIELD(m_HackedString, FIELD_STRING, "HackedString"),

	DEFINE_OUTPUT(m_OnUnlocked, "OnUnlocked"),
	DEFINE_OUTPUT(m_OnLocked, "OnLocked"),
	
	DEFINE_INPUTFUNC(FIELD_VOID, "InputLock", InputLock),
	DEFINE_INPUTFUNC(FIELD_VOID, "InputUnlock", InputUnlock),

END_DATADESC()

LINK_ENTITY_TO_CLASS(over_doorconsole, COverlordDoorLock);

IMPLEMENT_SERVERCLASS_ST(COverlordDoorLock, DT_OverlordDoorLock)

	SendPropFloat(SENDINFO(m_flHackTime)),
	SendPropFloat(SENDINFO(m_flHacked)),
	SendPropBool(SENDINFO(m_bLocked)),

	SendPropStringT(SENDINFO(m_HackingString)),
	SendPropStringT(SENDINFO(m_HackedString)),

END_SEND_TABLE()

COverlordDoorLock::COverlordDoorLock()
{
	m_bLocked = true;

	m_hHacker = NULL;

	m_flSoundTime = 0.0f;
}

COverlordDoorLock::~COverlordDoorLock()
{

}

void COverlordDoorLock::Precache()
{
	BaseClass::Precache();

	PrecacheModel(STRING(GetModelName()));

	SetCollisionGroup(COLLISION_GROUP_NONE);
	SetSolid(SOLID_VPHYSICS);

	CreateVPhysics();

	PrecacheScriptSound(HACK_SOUND);
	PrecacheScriptSound(ALARM_SOUND);
}

void COverlordDoorLock::Spawn()
{
	Precache();
	
	SetModel(STRING(GetModelName()));

	SetSolid(SOLID_VPHYSICS);

	SetMoveType(MOVETYPE_NONE);

	SetCollisionGroup(COLLISION_GROUP_NONE);

	SetUse(&COverlordDoorLock::ConsoleUse);

	SetThink(&COverlordDoorLock::Think);

	SetNextThink(gpGlobals->curtime + 0.5f);

	CreateVPhysics();

	// Make sure the times are right
	if(m_flMinimumHackTime > m_flMaximumHackTime)
		m_flMinimumHackTime = m_flMaximumHackTime;

	m_flHackTime = m_flMaximumHackTime;
}

void COverlordDoorLock::Activate()
{
	BaseClass::Activate();

	ResolveAreas();
}

void COverlordDoorLock::Think()
{
	// Restarts the hack if console was left alone for too long
	if(m_flNextHack + HACK_INTER < gpGlobals->curtime)
	{	
		m_hHacker = NULL;
		m_flHacked = 0.0f;
	}

	if(IsLocked() && (m_PlayerDistance > 0))
	{
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);
		CTeam * pSpectators = GetGlobalTeam(TEAM_SPECTATOR);
		if(pRebels)
		{
			if(pRebels->GetNumPlayers() > 1)
			{
				// Subtract one player, the hacking one
				int inSphere = GetNumPlayersAround();
				int count = pRebels->GetNumPlayers();		
				float decrease = (m_flMaximumHackTime - m_flMinimumHackTime) * (float)inSphere/(float)count;

				m_flHackTime = m_flMaximumHackTime - decrease;
			}
			else if(pSpectators && pSpectators->GetNumPlayers() <= 0)
			{
				m_flHackTime = m_flMinimumHackTime;
			}
		}
		
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}

int COverlordDoorLock::ObjectCaps()
{
	return (BaseClass::ObjectCaps() | FCAP_CONTINUOUS_USE);
}

void COverlordDoorLock::ConsoleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if(!pCaller)
		return;

	CBasePlayer * pPlayer = ToBasePlayer(pCaller);

	if(!pPlayer)
		return;

	RunHack(pPlayer);

}

void COverlordDoorLock::ResolveAreas()
{
	bool overriden[AREAS];

	// First look for  the overrided names
	for(int i = 0; i < AREAS; i++)
	{
		overriden[i] = false;
		CBaseEntity * pEnt = gEntList.FindEntityByName(NULL, m_iszAreaEntName[i]);

		if(!pEnt || !dynamic_cast<COverlordArea*>(pEnt))
			continue;
		else
		{
			//Q_strncpy(m_iszAreaEntName[i], STRING(pEnt->GetEntityName()), ARRAYSIZE(m_iszAreaEntName[i]));
			Q_strncpy(m_iszAreaName[i], static_cast<COverlordArea*>(pEnt)->GetAreaName(), ARRAYSIZE(m_iszAreaName[i]));
			overriden[i] = true;
		}
	}

	// Now do the standard override
	CBaseEntity * list[1024];
	const int count = UTIL_EntitiesInSphere(list, ARRAYSIZE(list), GetAbsOrigin(), 2048, 0);

	if(count <= 0)
	{
		Warning("You've got a door console yet no areas nearby\n");
		return;
	}



	// Count out used areas
	for(int i = count - 1; i >= 0; i--)
	{
		if(!list[i] || !dynamic_cast<COverlordArea*>(list[i]))
			continue;

		for(int j = 0; j < AREAS; j++)
		{
			if(!Q_stricmp(STRING(list[i]->GetEntityName()), STRING(m_iszAreaEntName[j])))
			{
				list[i] = NULL;
				break;
			}
		}
	}

	for(int j = 0; j < AREAS; j++)
	{
		if(overriden[j])
			continue;

		CBaseEntity * pNearest = NULL;
		float dist = MAX_TRACE_LENGTH * MAX_TRACE_LENGTH;
		int index = 0;

		for(int i = count - 1; i >= 0; i--)
		{
			if(!list[i] || !dynamic_cast<COverlordArea*>(list[i]))
				continue;

			if(dist > (list[i]->GetAbsOrigin() - GetAbsOrigin()).LengthSqr())
			{
				pNearest = list[i];
				index = i;
				dist = (list[i]->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();
			}



		}

		if(pNearest)
		{
			list[index] = NULL;
			//Q_strncpy(m_iszAreaEntName[j], STRING(pNearest->GetEntityName()), ARRAYSIZE(m_iszAreaEntName[j]));
			m_iszAreaEntName[j] = pNearest->GetEntityName();
			Q_strncpy(m_iszAreaName[j], static_cast<COverlordArea*>(pNearest)->GetAreaName(), ARRAYSIZE(m_iszAreaName[j]));
		}
	}



}

void COverlordDoorLock::RunHack(CBasePlayer * pPlayer)
{
	if(!pPlayer)
		return;

	// Not the previous hacker? return, do not let him hack!
	if((m_hHacker && m_hHacker != pPlayer))
	{
		return;
	}

	// Make sure we got enough alive players around
	if(m_PercentageAlive > 0)
	{
		CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

		if(pRebels)
		{
			int inSphere = GetNumPlayersAround();
			int count = pRebels->GetNumPlayers();
			
			int percentage = int(float(inSphere)/float(count) * 100.0f);

			if(percentage < m_PercentageAlive)
				return;
		}
	}

	if(!IsLocked() && (m_flSoundTime < gpGlobals->curtime))
	{
		//EmitSound(DENY_SOUND);
		m_flSoundTime = gpGlobals->curtime + DENY_SOUND_LENGTH;
		return;
	}

	// Emits sound...
	if((m_flSoundTime < gpGlobals->curtime) && IsLocked())
	{
		EmitSound(HACK_SOUND);
		m_flSoundTime = gpGlobals->curtime + HACK_SOUND_LENGTH;
	}

	if(m_flNextHack > gpGlobals->curtime || !IsLocked())
		return;


	if(m_flHacked >= m_flHackTime)
	{
		if(!HasSpawnFlags(SF_DO_NOT_WARN))
		{
			COverlordDoorLock::m_LastHacked = this;
			CBasePlayer * pOverlord = (GET_OVERLORD_DATA)->GetOverlord();

			if(pOverlord)
			{
				Vector vPosition = pOverlord->EyePosition();
				ClientPrint(pOverlord, HUD_PRINTCENTER, "Security between areas %s1 - %s2 has been breached", m_iszAreaName[0], m_iszAreaName[1]);
				
				CSingleUserRecipientFilter filter(pOverlord);
				pPlayer->EmitSound(filter, pOverlord->entindex(), ALARM_SOUND, &vPosition);	
			}
		
		}

		//pPlayer->IncreaseOverlordDamage(eo_points_for_console.GetInt());
		Unlock();
	}
	else
	{
		m_hHacker = pPlayer;

		// Although we could calculate how much we should increase the
		// counter, this solution gives sense of some randomness
		// and is much more exploit-proof
		m_flHacked += NEXT_HACK;

		m_flNextHack = gpGlobals->curtime + NEXT_HACK;
	}

	// Only show if it's still locked
	if(m_flHackTime != 0.0f && IsLocked())
	{
		float perc = m_flHacked / m_flHackTime;

		if(perc > 1.0f)
			perc = 1.0f;

		char command[32];
		Q_snprintf(command, ARRAYSIZE(command), "progress_bar %f", perc);

		engine->ClientCommand(pPlayer->edict(), command);
	}
}

void COverlordDoorLock::Lock(bool bOutput)
{
	m_flHacked = 0.0f;
	//m_flHackTime = 0.0f;
	m_flNextHack = 0.0f;
	m_flSoundTime = 0.0f;

	m_hHacker = NULL;

	if(bOutput)
		m_OnLocked.FireOutput(NULL, this);

	SetLocked(true);
}

void COverlordDoorLock::Unlock(bool bOutput)
{
	//StopSound(HACK_SOUND);

	m_flHacked = 0.0f;
	//m_flHackTime = 0.0f;
	m_flNextHack = 0.0f;
	m_flSoundTime = 0.0f;

	if(bOutput)
		m_OnUnlocked.FireOutput(NULL, this);

	m_hHacker = NULL;

	SetLocked(false);
}

void COverlordDoorLock::InputLock(inputdata_t & inputData)
{
	if(!IsLocked())
		Lock(false);
}

void COverlordDoorLock::InputUnlock(inputdata_t & inputData)
{
	if(IsLocked())
		Unlock(false);
}

int COverlordDoorLock::GetNumPlayersAround()
{
	CTeam * pRebels = GetGlobalTeam(TEAM_REBELS);

	if(pRebels)
	{
		int inSphere = 0;
		for(int i = 0; i < pRebels->GetNumPlayers(); i++)
		{
			CBasePlayer * pPlayer = pRebels->GetPlayer(i);

			if(!pPlayer || !pPlayer->IsAlive() || pPlayer->IsInvisible())
				continue;
			
			if((pPlayer->WorldSpaceCenter() - WorldSpaceCenter()).Length() <= m_PlayerDistance)
			{
				if(HasSpawnFlags(SF_LINE_OF_SIGHT))
				{
					CTraceFilterNoNPCsOrPlayer filter(this, COLLISION_GROUP_NONE);

					trace_t tr;
					UTIL_TraceLine(WorldSpaceCenter(), pPlayer->WorldSpaceCenter(), MASK_SHOT, &filter, &tr);
					
					if(tr.fraction < 0.99f)
						continue;
				
				}

				inSphere++;
			}

		}
		return inSphere;
	}

	return 0;
}