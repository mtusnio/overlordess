//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"

#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "GameStats.h"
#include "tier0/vprof.h"
#include "bone_setup.h"
#include "viewport_panel_names.h"
#include "gib.h"
#include "particle_parse.h"

#include "overlord_dynamictraps.h"
#include "overlord_tutorial.h"
#include "overlord_pathnode.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "obstacle_pushaway.h"

#include "ilagcompensationmanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

ConVar eo_points_per_death("eo_points_per_death", "-10", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_gib_damage("eo_gib_damage", "20", FCVAR_REPLICATED | FCVAR_CHEAT, "Minimum blast damage needed to gib");
ConVar eo_gib_chance("eo_gib_chance", "100", FCVAR_REPLICATED | FCVAR_CHEAT, "Chance that you will be gibbed if enough damage has been dealt");
ConVar eo_spawn_time("eo_spawn_time", "20", 0, "Spawn time");
CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

//LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_overlord, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPLocalPlayerExclusive )
	// send a hi-res origin to the local player for use in prediction
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
//	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),
	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),
END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),

	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),

	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	SendPropExclude( "DT_BaseFlex", "m_flexWeight" ),
	SendPropExclude( "DT_BaseFlex", "m_blinktoggle" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

	// Data that only gets sent to the local player.
	SendPropDataTable( "hl2mplocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
	// Data that gets sent to all other players
	SendPropDataTable( "hl2mpnonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropBool( SENDINFO( m_bSpawnInterpCounter) ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),
		
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()
const char *g_ppszRandomRebelModels[] = 
{
	/*"models/humans/group03/male_01.mdl",
	"models/humans/group03/male_02.mdl",
	"models/humans/group03/female_01.mdl",
	"models/humans/group03/male_03.mdl",
	"models/humans/group03/female_02.mdl",
	"models/humans/group03/male_04.mdl",
	"models/humans/group03/female_03.mdl",
	
	"models/humans/group03/female_04.mdl",
	"models/humans/group03/male_06.mdl",
	"models/humans/group03/female_06.mdl",

	"models/humans/group03/male_08.mdl",
	"models/humans/group03/male_09.mdl",*/
	"models/humans/group03/male_05.mdl",
	"models/humans/group03/male_07.mdl",
	"models/humans/group03/female_07.mdl",
	"models/police.mdl",
};

const char *g_ppszRandomOverlordModels[] =
{
	"models/combine_soldier.mdl",
	"models/combine_soldier_prisonguard.mdl",
	"models/combine_super_soldier.mdl",
	
};

#define MAX_COMBINE_MODELS 4
#define MODEL_CHANGE_INTERVAL 1.0f
#define TEAM_CHANGE_INTERVAL 1.0f

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#define BURN_MODEL "models/Humans/Charple01.mdl"

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player()
{
	//Tony; create our player animation state.
	m_PlayerAnimState = CreateHL2MPPlayerAnimState( this );
	UseClientSideAnimation();

	m_angEyeAngles.Init();

	m_iLastWeaponFireUsercmd = 0;

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_bSpawnInterpCounter = false;

    m_bEnterObserver = false;
	m_bReady = false;

	BaseClass::ChangeTeam( 0 );

	m_PlayerClass = CLASS_DEFAULT;
	
	m_nBurnModel = 0;
//	UseClientSideAnimation();
}

CHL2MP_Player::~CHL2MP_Player( void )
{
	m_PlayerAnimState->Release();

	COverlordPathNode * node = COverlordPathNode::GetLastNode();
	if(node)
		node->RemoveFromChain(this);
}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	//Precache Citizen models
	int nHeads = ARRAYSIZE( g_ppszRandomRebelModels );
	int i;	

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomRebelModels[i] );

	//Precache Combine Models
	nHeads = ARRAYSIZE( g_ppszRandomOverlordModels );

	for ( i = 0; i < nHeads; ++i )
	   	 PrecacheModel( g_ppszRandomOverlordModels[i] );

	PrecacheFootStepSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
	PrecacheParticleSystem("BloodMist");
	PrecacheParticleSystem("BurnSmoke");

	m_nBurnModel = PrecacheModel(BURN_MODEL);
}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo( 255,	"Pistol");
	CBasePlayer::GiveAmmo( 255,	"AR2" );
	CBasePlayer::GiveAmmo( 5,	"AR2AltFire" );
	CBasePlayer::GiveAmmo( 255,	"SMG1");
	CBasePlayer::GiveAmmo( 1,	"smg1_grenade");
	CBasePlayer::GiveAmmo( 255,	"Buckshot");
	CBasePlayer::GiveAmmo( 32,	"357" );
	CBasePlayer::GiveAmmo( 3,	"rpg_round");

	CBasePlayer::GiveAmmo( 1,	"grenade" );
	CBasePlayer::GiveAmmo( 2,	"slam" );

	GiveNamedItem( "weapon_crowbar" );
	GiveNamedItem( "weapon_stunstick" );
	GiveNamedItem( "weapon_pistol" );
	GiveNamedItem( "weapon_357" );

	GiveNamedItem( "weapon_smg1" );
	GiveNamedItem( "weapon_ar2" );
	
	GiveNamedItem( "weapon_shotgun" );
	GiveNamedItem( "weapon_frag" );
	
	GiveNamedItem( "weapon_crossbow" );
	
	GiveNamedItem( "weapon_rpg" );

	GiveNamedItem( "weapon_slam" );

	GiveNamedItem( "weapon_physcannon" );
	
}

void CHL2MP_Player::GiveDefaultItems( void )
{
	EquipSuit();

	RemoveAllItems(false);

	GiveClassWeapons();

	const char *szDefaultWeaponName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_defaultweapon" );

	CBaseCombatWeapon *pDefaultWeapon = Weapon_OwnsThisType( szDefaultWeaponName );

	if ( pDefaultWeapon )
	{
		Weapon_Switch( pDefaultWeapon );
	}
	else
	{
		Weapon_Switch( Weapon_OwnsThisType( "weapon_physcannon" ) );
	}
}

void CHL2MP_Player::PickDefaultSpawnTeam( void )
{
	// Handle tutorial first
	if(GetOverlordData()->IsInTutorial())
	{
		COverlordTutorial * pEnt = static_cast<COverlordTutorial*>(gEntList.FindEntityByClassname(NULL, "ov_tutorial"));

		if(pEnt)
		{
			ChangeTeam(pEnt->GetTutorialTeam());
			return;
		}
	}

	if ( GetTeamNumber() == 0 )
	{
		if ( HL2MPRules()->IsTeamplay() == false )
		{
			if ( GetModelPtr() == NULL )
			{
				const char *szModelName = NULL;
				szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

				if ( ValidatePlayerModel( szModelName ) == false )
				{
					char szReturnString[512];

					Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel models/combine_super_soldier.mdl\n" );
					engine->ClientCommand ( edict(), szReturnString );
				}

				ChangeTeam( TEAM_UNASSIGNED );
			}
		}
		else
		{
			CTeam *pOverlord = g_Teams[TEAM_OVERLORD];
			CTeam *pRebels = g_Teams[TEAM_REBELS];

			if ( pOverlord == NULL && pRebels == NULL )
			{
				//ChangeTeam( random->RandomInt( TEAM_OVERLORD, TEAM_REBELS ) );
				ChangeTeam(TEAM_OVERLORD);
			}
			else
			{
				bool bAddSpawns = false;
				if (pOverlord->GetNumPlayers() != 0)
				{
					
					if((HL2MPRules()->GetRoundTime() > eo_allow_spawn_time.GetFloat()) && (eo_allow_spawn_time.GetFloat() > 0))
					{
						ChangeTeam(TEAM_SPECTATOR);
						m_SpawnTime = gpGlobals->curtime + eo_spawn_time.GetFloat();
					}
					else
					{
						ChangeTeam(TEAM_REBELS);
						bAddSpawns = true;
					}

				}
				else if (pOverlord->GetNumPlayers() == 0 && pRebels->GetNumPlayers() != 0)
				{
					if((HL2MPRules()->GetRoundTime() > eo_allow_spawn_time.GetFloat()) && (eo_allow_spawn_time.GetFloat() > 0))
						ChangeTeam(TEAM_SPECTATOR);
					else
					{
						ChangeTeam(TEAM_REBELS);
						bAddSpawns = true;
					}
				}
				else
				{
					ChangeTeam(TEAM_OVERLORD);
				}

				if(bAddSpawns)
				{
					GetOverlordData()->SetSpawnsAmount(GetOverlordData()->GetSpawnsAmount() + 
						GetOverlordData()->GetSpawnsPerPlayer());
					GetOverlordData()->SetInitialSpawns(GetOverlordData()->GetInitialSpawns() + 
						GetOverlordData()->GetSpawnsPerPlayer());
					HL2MPRules()->SetInitialPlayers(HL2MPRules()->GetInitialPlayers() + 1);
				}
				
				/*if ( pOverlord->GetNumPlayers() > pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_REBELS );
				}
				else if ( pOverlord->GetNumPlayers() < pRebels->GetNumPlayers() )
				{
					ChangeTeam( TEAM_OVERLORD );
				}
				else
				{
					ChangeTeam( random->RandomInt( TEAM_OVERLORD, TEAM_REBELS ) );
				}*/
			}
				
		
		}
	}
}
#define HL2MP_PUSHAWAY_THINK_CONTEXT	"HL2MPPushawayThink"
void CHL2MP_Player::HL2MPPushawayThink(void)
{
	// Push physics props out of our way.
	PerformObstaclePushaway( this );
	SetNextThink( gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, HL2MP_PUSHAWAY_THINK_CONTEXT );
}


void CHL2MP_Player::SwitchToClass(const PlayerClass_t playerclass)
{
	BaseClass::SwitchToClass(playerclass);

	SetPlayerModel();
}
//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_SpawnTime = 0.0f;

	PickDefaultSpawnTeam();

	if(/*GetPlayerClass() != GetFuturePlayerClass() 
		&&*/ GetFuturePlayerClass() != CLASS_DEFAULT)
	{
		SetPlayerClass(GetFuturePlayerClass());
		m_bCanSwitchClass = false;
	}
	else if (/*GetPlayerClass() == CLASS_DEFAULT 
		&&*/ GetFuturePlayerClass() == CLASS_DEFAULT)
	{	
		if(IsRebel())
			ShowViewPortPanel(PANEL_CLASS, true);
		else if(IsOverlord())
			ShowViewPortPanel(PANEL_OVCLASS, true);
		m_bCanSwitchClass = true;
	}


	BaseClass::Spawn();
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
		
		GiveDefaultItems();

		ResolveClassFeatures();
	}

	RemoveEffects( EF_NOINTERP );

	m_nRenderFX = kRenderNormal;

	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	if(!IsOverlord())
	{
		m_Local.m_iHideHUD |= HIDEHUD_OVERLORD;
	}
	else
	{
		m_Local.m_iHideHUD &= ~HIDEHUD_OVERLORD;
	}

	if(!IsRebel())
	{
		m_Local.m_iHideHUD |= HIDEHUD_REBEL;
	}
	else
	{
		m_Local.m_iHideHUD &= ~HIDEHUD_REBEL;
	}

	// Set classes on initial spawns
	if(IsOverlord())
		m_PlayerClass = CLASS_TANK;
	else if(IsRebel() && GetDeathsThisRound() == 0)
		m_PlayerClass = CLASS_ASSAULT;

	m_bSpawnInterpCounter = !m_bSpawnInterpCounter;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	// Update class model
	SetPlayerModel();

	//Tony; do the spawn animevent
	DoAnimationEvent( PLAYERANIMEVENT_SPAWN );

	SetContextThink( &CHL2MP_Player::HL2MPPushawayThink, gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, HL2MP_PUSHAWAY_THINK_CONTEXT );

	// Force hud reload
	engine->ClientCommand(edict(), "hud_reloadscheme");
}

void CHL2MP_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	
}

bool CHL2MP_Player::ValidatePlayerModel( const char *pModel )
{
	int iModels = ARRAYSIZE( g_ppszRandomRebelModels );
	int i;	

	for ( i = 0; i < iModels; ++i )
	{
		if ( !Q_stricmp( g_ppszRandomRebelModels[i], pModel ) )
		{
			return true;
		}
	}

	iModels = ARRAYSIZE( g_ppszRandomOverlordModels );

	for ( i = 0; i < iModels; ++i )
	{
	   	if ( !Q_stricmp( g_ppszRandomOverlordModels[i], pModel ) )
		{
			return true;
		}
	}

	return false;
}

/*void CHL2MP_Player::SetPlayerTeamModel( void )
{
	const char *szModelName = NULL;
	szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	int modelIndex = modelinfo->GetModelIndex( szModelName );

	if ( modelIndex == -1 || ValidatePlayerModel( szModelName ) == false )
	{
		szModelName = "models/Combine_Super_Soldier.mdl";
		m_iModelType = TEAM_OVERLORD;

		char szReturnString[512];

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}

	if ( GetTeamNumber() == TEAM_OVERLORD )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomOverlordModels );
		
		g_iLastCombineModel = ( g_iLastCombineModel + 1 ) % nHeads;
		szModelName = g_ppszRandomOverlordModels[g_iLastCombineModel];

		m_iModelType = TEAM_OVERLORD;
	}
	else if ( GetTeamNumber() == TEAM_REBELS )
	{
		int nHeads = ARRAYSIZE( g_ppszRandomRebelModels );

		g_iLastCitizenModel = ( g_iLastCitizenModel + 1 ) % nHeads;
		szModelName = g_ppszRandomRebelModels[g_iLastCitizenModel];

		m_iModelType = TEAM_REBELS;
	}
	
	SetModel( szModelName );
	SetupPlayerSoundsByModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}*/

void CHL2MP_Player::SetPlayerModel( void )
{
	const char *szModelName = NULL;
//	const char *pszCurrentModelName = modelinfo->GetModelName( GetModel());

	szModelName = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_playermodel" );

	/*if ( ValidatePlayerModel( szModelName ) == false )
	{
		char szReturnString[512];

		if ( ValidatePlayerModel( pszCurrentModelName ) == false )
		{
			pszCurrentModelName = "models/Combine_Super_Soldier.mdl";
		}

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", pszCurrentModelName );
		engine->ClientCommand ( edict(), szReturnString );

		szModelName = pszCurrentModelName;
	}*/

	if ( GetTeamNumber() == TEAM_OVERLORD )
	{
		if(GetPlayerClass() == CLASS_TANK)
		{
			szModelName = "models/combine_super_soldier.mdl";
		}
		else if(GetPlayerClass() == CLASS_PHASING)
		{
			szModelName = "models/combine_soldier_prisonguard.mdl";
		}
		else
		{
			szModelName = "models/combine_soldier.mdl";
		}
		/*int nHeads = ARRAYSIZE( g_ppszRandomOverlordModels );
		
		g_iLastCombineModel = ( g_iLastCombineModel + 1 ) % nHeads;
		szModelName = g_ppszRandomOverlordModels[g_iLastCombineModel];*/

		m_iModelType = TEAM_OVERLORD;
	}
	else if ( GetTeamNumber() == TEAM_REBELS )
	{
		// Select class models
		if(GetPlayerClass() == CLASS_ASSAULT)
		{
			szModelName = "models/police.mdl";
		}
		else if(GetPlayerClass() == CLASS_HACKER)
		{
			szModelName = "models/humans/group03/female_07.mdl";
		}
		else if(GetPlayerClass() == CLASS_PSION)
		{
			szModelName = "models/humans/group03/female_07.mdl";
		}
		else if(GetPlayerClass() == CLASS_STEALTH)
		{
			szModelName = "models/humans/group03/male_05.mdl";
		}
		else
		{
			int nHeads = ARRAYSIZE( g_ppszRandomRebelModels );

			g_iLastCitizenModel = ( g_iLastCitizenModel + 1 ) % nHeads;
			szModelName = g_ppszRandomRebelModels[g_iLastCitizenModel];
		}

		m_iModelType = TEAM_REBELS;
	}
	else
	{
		if ( Q_strlen( szModelName ) == 0 ) 
		{
			szModelName = g_ppszRandomRebelModels[0];
		}

		if ( Q_stristr( szModelName, "models/human") )
		{
			m_iModelType = TEAM_REBELS;
		}
		else
		{
			m_iModelType = TEAM_OVERLORD;
		}
	}

	int modelIndex = modelinfo->GetModelIndex( szModelName );

	if ( modelIndex == -1 )
	{
		Warning("Modelindex of player's model is equal to -1\n");
		szModelName = "models/humans/group03/male_07.mdl";
		m_iModelType = TEAM_REBELS;

		char szReturnString[512];

		Q_snprintf( szReturnString, sizeof (szReturnString ), "cl_playermodel %s\n", szModelName );
		engine->ClientCommand ( edict(), szReturnString );
	}

	SetModel( szModelName );
	SetupPlayerSoundsByModel( szModelName );

	m_flNextModelChangeTime = gpGlobals->curtime + MODEL_CHANGE_INTERVAL;
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models/human") )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();
    m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}

	if(IsRebel() && gpGlobals->curtime - GetDeathTime() >= 3.25f)
	{
		ChangeTeam(TEAM_SPECTATOR);
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_iDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CHL2MP_Player::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBaseEntity * pEnt, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	return BaseClass::WantsLagCompensationOnEntity( pEnt, pCmd, pEntityTransmitBits );
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_OVERLORD )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;



extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
/*	if ( GetNextTeamChangeTime() >= gpGlobals->curtime )
	{
		char szReturnString[128];
		Q_snprintf( szReturnString, sizeof( szReturnString ), "Please wait %d more seconds before trying to switch teams again.\n", (int)(GetNextTeamChangeTime() - gpGlobals->curtime) );

		ClientPrint( this, HUD_PRINTTALK, szReturnString );
		return;
	}*/

	bool bKill = false;

	if ( HL2MPRules()->IsTeamplay() != true && iTeam != TEAM_SPECTATOR )
	{
		//don't let them try to join combine or rebels during deathmatch.
		iTeam = TEAM_UNASSIGNED;
	}

	/*if ( HL2MPRules()->IsTeamplay() == true )
	{
		// hack that prevents the new Overlord from getting killed,
		// and still changes his model etc.
		if ( iTeam != TEAM_OVERLORD && iTeam != GetTeamNumber() && GetTeamNumber() != TEAM_UNASSIGNED )
		{
			bKill = true;
		}
	}*/

	if(IsRebel() && iTeam != TEAM_REBELS)
	{
		COverlordPathNode * node = COverlordPathNode::GetLastNode();
		if(node)
			node->RemoveFromChain(this);
	}

	// Reset classes
	if(IsRebel() && iTeam == TEAM_OVERLORD)
	{
		SetPlayerClass(CLASS_TANK);
		SetFuturePlayerClass(CLASS_DEFAULT);
	}
	else if(IsOverlord() && iTeam == TEAM_REBELS)
	{
		SetPlayerClass(CLASS_ASSAULT);
		SetFuturePlayerClass(CLASS_DEFAULT);
	}
	
	// Switch from camera!
	if(IsOverlord() && GetOverlordData()->IsInVehicle())
		GetOverlordData()->SwitchFromCamera();

	BaseClass::ChangeTeam( iTeam );

	m_flNextTeamChangeTime = gpGlobals->curtime + TEAM_CHANGE_INTERVAL;

	/*if ( HL2MPRules()->IsTeamplay() == true )
	{
		SetPlayerTeamModel();
	}
	else
	{*/
		SetPlayerModel();
	//}

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );

		State_Transition( STATE_OBSERVER_MODE );
	}
	else if ( iTeam == TEAM_OVERLORD )
	{
		//m_PlayerClass = CLASS_DEFAULT;
		UTIL_ClientPrintAll( HUD_PRINTTALK, "%s has become the new Overlordess.", GetPlayerName()  );

		(GET_OVERLORD_DATA)->UpdateOverlord(this);
	}
	else if ( iTeam == TEAM_REBELS )
	{
		//m_PlayerClass = CLASS_ASSAULT;
	}

	if ( bKill == true )
	{
		CommitSuicide();
	}



}

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		if ( GetTeamNumber() != TEAM_UNASSIGNED && !IsDead() )
		{
			m_fNextSuicideTime = gpGlobals->curtime;	// allow the suicide to work

			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	/*if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if ( FStrEq( args[0], "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		if ( ShouldRunRateLimitedCommand( args ) )
		{
			int iTeam = atoi( args[1] );
			HandleCommand_JoinTeam( iTeam );
		}
		return true;
	}
	else */if ( FStrEq( args[0], "joingame" ) )
	{
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

//=================================================================
//				Overlord player commands
//=================================================================

void CC_EO_MakeOverlord(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	pPlayer->ChangeTeam( TEAM_OVERLORD );
	pPlayer->Spawn();

	pPlayer->ResetDeathCount();
}

static ConCommand eo_makeoverlord("eo_makeoverlord", CC_EO_MakeOverlord, "Switch the player to Overlord.", FCVAR_CHEAT);

void CC_EO_MakeRebel(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	pPlayer->ChangeTeam( TEAM_REBELS );
	pPlayer->Spawn();

	pPlayer->ResetDeathCount();
}
static ConCommand eo_makerebel("eo_makerebel", CC_EO_MakeRebel, "Switch the player to Rebel team.", FCVAR_CHEAT);

void CC_EO_MakeConverted(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	pPlayer->ChangeTeam( TEAM_CONVERTED );
	pPlayer->Spawn();
	
	pPlayer->ResetDeathCount();
}
static ConCommand eo_makeconverted("eo_makeconverted", CC_EO_MakeConverted, "Switch the player to the Converted team.", FCVAR_CHEAT);

void CC_EO_GiveUp(void)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer || !pPlayer->IsOverlord())
		return;

	if(!pPlayer->IsAlive())
		return;

	mp_restartgame.SetValue(5);

	UTIL_ClientPrintAll(HUD_PRINTCENTER,"The Overlordess has given up!");
}

static ConCommand eo_giveup("eo_giveup", CC_EO_GiveUp, "Give up being the Overlord");


#ifdef OVER_DEBUG
void CC_EO_DEB_DSP(const CCommand &args)
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
		return;

	int dsp = atoi(args[1]);

	CSingleUserRecipientFilter user(pPlayer);
	enginesound->SetPlayerDSP(user, 0, false );
	enginesound->SetPlayerDSP(user, dsp, false );
	
}
#endif

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //


LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),
	SendPropString( SENDINFO( m_particleName)),
END_SEND_TABLE()

CHL2MPRagdoll::CHL2MPRagdoll()
{
	Q_strncpy(m_particleName.GetForModify(), "\0", sizeof(m_particleName));
}
void CHL2MP_Player::CreateRagdollEntity( int damagetype /* = 0 */ )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
		// Decide on the model index
		if(!IsOverlord() && (damagetype == DMG_BURN))
		{
			pRagdoll->m_nModelIndex = m_nBurnModel;
			// particles!
			Q_strncpy(pRagdoll->m_particleName.GetForModify(), "BurnSmoke", sizeof(pRagdoll->m_particleName));
		}
		else
			pRagdoll->m_nModelIndex = m_nModelIndex;
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/*bool CHL2MP_Player::IsOverlord()
{
	if(GetTeamNumber() == TEAM_OVERLORD)
	{
		return true;
	}

	return false;
}*/


/*void CHL2MP_Player::SwitchToCamera(COverlordCamera * cam)
{
	COverlordData * pData = GET_OVERLORD_DATA;

	if(!pData)
	{
		Warning("COverlordData does not exist\n");
		return;
	}

	if(!cam)
	{
		Warning("Overlord's camera does not exist!\n");
		return;
	}

	//pData->SetCamera(cam);
	if(!(GetFlags() & FL_FROZEN))
	{
		AddFlag(FL_FROZEN);
	}
	pData->SetPosition(cam);
	pData->SetInVehicle(true);
};

void CHL2MP_Player::SwitchFromCamera(void)
{
	COverlordData * pData = GET_OVERLORD_DATA;

	if(!pData)
	{
		Warning("COverlordData does not exist\n");
		return;
	}

	COverlordCamera * cam = pData->GetCamera();

	if(!cam)
	{
		Warning("Overlord's camera does not exist!\n");
		return;
	}

	//pData->SetCamera(cam);
	if(!(GetFlags() & FL_FROZEN))
	{
		RemoveFlag(FL_FROZEN);
	}
	
	Vector vNull;
	QAngle aNull;
	vNull.Init(0, 0, 0);
	aNull.Init(0, 0, 0);


	pData->SetPosition(NULL);
	pData->SetInVehicle(false);
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( IsInvisible() )
		return;

	
	/*if(IsOverlord())
	{
		GET_OVERLORD_DATA_ASSERT(pData);

		if(pData->IsInVehicle())
			return;

	}*/

	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}


void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

bool CHL2MP_Player::CorpseGib( const CTakeDamageInfo &info)
{
	const int GIB_DAMAGE = eo_gib_damage.GetInt();
	const int GIB_CHANCE = eo_gib_chance.GetInt();
	const int GIB_OVERLAY_DISTANCE = 164;

	if((info.GetDamageType() & DMG_BLAST) && (info.GetDamage() >= GIB_DAMAGE) 
		&& (random->RandomInt(0, 100) <= GIB_CHANCE))
	{
		// Blood
		trace_t bloodTrace;
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() - Vector( 0, 0, 16 ), MASK_SOLID_BRUSHONLY,
			this, COLLISION_GROUP_NONE, &bloodTrace);
		
		if ( bloodTrace.fraction < 1.0f )
		{
			UTIL_BloodDecalTrace( &bloodTrace, BLOOD_COLOR_RED );
		}

		DispatchParticleEffect("BloodMist", WorldSpaceCenter(), GetAbsAngles());

		CGib::SpawnHeadGib( this );
		CGib::SpawnRandomGibs( this, 5, GIB_HUMAN );	// throw some human gibs.

		for(int i = 1; i <= MAX_PLAYERS; i++)
		{
			CBasePlayer * pPlayer = UTIL_PlayerByIndex(i);

			if(!pPlayer || !(pPlayer->IsRebel() || pPlayer->IsOverlord()) || !pPlayer->IsAlive())
				continue;

			if(pPlayer->FInViewCone(WorldSpaceCenter()) && (WorldSpaceCenter() - pPlayer->RealEyePosition()).Length() <= GIB_OVERLAY_DISTANCE)
			{
				trace_t tr;
				UTIL_TraceLine(WorldSpaceCenter(), pPlayer->RealEyePosition(), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

				if(tr.m_pEnt == pPlayer || tr.fraction == 1.0f)
				{
					CSingleUserRecipientFilter user(pPlayer);
					user.MakeReliable();
					UserMessageBegin( user, "ScreenOverlay" );
						WRITE_SHORT( 0 );
					MessageEnd();
				}
			}
		}
		return true;
	}

	return false;

}
void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	m_SpawnTime = gpGlobals->curtime + eo_spawn_time.GetFloat();

	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );


	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.

	// Decide whether w should be gibbed or ragdolled
	if(IsOverlord() || !CorpseGib(info))
	{
		CreateRagdollEntity(info.GetDamageType());
	}
	else
	{
		if ( m_hRagdoll )
		{
			UTIL_RemoveImmediate( m_hRagdoll );
			m_hRagdoll = NULL;
		}
	}

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	if ( pAttacker )
	{
		int iScoreToAdd = 1;

		if ( pAttacker == this )
		{
			iScoreToAdd = -1;
		}

		GetGlobalTeam( pAttacker->GetTeamNumber() )->AddScore( iScoreToAdd );
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();

	// Switch to spectators
	if(IsRebel())
	{
		// Take some points
		IncreaseOverlordDamage(eo_points_per_death.GetInt());
		
		// Little hack to make the bots swap teams
		if(IsBot())
			ChangeTeam(TEAM_SPECTATOR);
	}

	COverlordPathNode * pLast = COverlordPathNode::GetLastNode();

	if(pLast)
		pLast->RemoveFromChain(this);

	// Increment trap's kills
	COverlordTrap * pTrap = COverlordTrap::EntityToTrap(pAttacker);

	if(pTrap)
		pTrap->IncrementKills();
}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{

	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	// Don't take damage while restarting round
	if(HL2MPRules()->IsRestarting())
		return 0;
	
	if(GetProtection() != NO_PROTECTION)
	{
		int bits = inputInfo.GetDamageType();

		// If we have a physical shield activated, we receive no
		// DMG_CRUSH damage
		if(GetProtection() & PHYS_SHIELD)
		{
			if((bits & DMG_CRUSH) ||
			   (bits & DMG_FALL)  || 
			   (bits & DMG_CLUB))
				return 0;
		}
		
		if(GetProtection() & BULLET_SHIELD)
		{
			if((bits & DMG_BULLET) && inputInfo.GetAttacker())
			{
				// A little hack, basically works like the backstabber
				// and takes the attacker's angles
				Vector dmgPos, eyePos;
				AngleVectors(QAngle(0.0f, GetAbsAngles().y, 0.0f), &eyePos);
				AngleVectors(QAngle(0.0f, inputInfo.GetAttacker()->GetAbsAngles().y, 0.0f), &dmgPos);
				VectorNormalize(dmgPos);
				VectorNormalize(eyePos);

				float flDot = DotProduct(dmgPos, eyePos);

				if(flDot <= 0.0)
					return 0;
			}
		}
	}

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	////gamestats->Event_PlayerDamage( this, inputInfo );

	CBasePlayer * pPlayer = ToBasePlayer(inputInfo.GetAttacker());
	// If I am the Overlord and have been attacked, increase
	// my attacker's damage statistics (only if he is a rebel)
	if(IsOverlord() && !HL2MPRules()->IsRestarting() && pPlayer && pPlayer->IsRebel())
	{
		pPlayer->IncreaseOverlordDamage(inputInfo.GetDamage());
	}

	return BaseClass::OnTakeDamage( inputInfo );
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	char szStepSound[128];

	Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.Die", GetPlayerModelSoundPrefix() );

	const char *pModelName = STRING( GetModelName() );

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, pModelName ) == false )
		return;

	Vector vecOrigin = GetAbsOrigin();
	
	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

	EmitSound_t ep;
	ep.m_nChannel = params.channel;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = params.volume;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_OVERLORD )
		{
			pSpawnpointName = "info_player_overlord";
			pLastSpawnPoint = g_pLastCombineSpawn;
		}
		else if ( GetTeamNumber() == TEAM_REBELS )
		{
			pSpawnpointName = "info_player_rebel";
			pLastSpawnPoint = g_pLastRebelSpawn;
		}

		if ( gEntList.FindEntityByClassname( NULL, pSpawnpointName ) == NULL )
		{
			//pSpawnpointName = "info_player_deathmatch";
			pSpawnpointName = "info_player_rebel";
			pLastSpawnPoint = g_pLastRebelSpawn;
			//pLastSpawnPoint = g_pLastSpawn;
		}
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 128 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if ( HL2MPRules()->IsTeamplay() == true )
	{
		if ( GetTeamNumber() == TEAM_OVERLORD )
		{
			g_pLastCombineSpawn = pSpot;
		}
		else if ( GetTeamNumber() == TEAM_REBELS ) 
		{
			g_pLastRebelSpawn = pSpot;
		}
	}

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset(bool bChangeTeam /*= true*/)
{	
	ResetOverlordStats();
	ResetDeathCount();
	ResetFragCount();
	
	if(bChangeTeam)
		ChangeTeam(TEAM_REBELS);

	SetFuturePlayerClass(CLASS_DEFAULT);
	if(IsOverlord())
	{
		SetPlayerClass(CLASS_TANK);
	}
	else
	{
		SetPlayerClass(CLASS_ASSAULT);
	}
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}
//-----------------------------------------------------------------------------
// Purpose: Do nothing multiplayer_animstate takes care of animation.
// Input  : playerAnim - 
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	return;
}
// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //
class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();
					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};
IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	//Tony; use prediction rules.
	filter.UsePredictionRules();
	
	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}


void CHL2MP_Player::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	m_PlayerAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

//-----------------------------------------------------------------------------
// Purpose: Override setup bones so that is uses the render angles from
//			the HL2MP animation state to setup the hitboxes.
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetupBones( matrix3x4_t *pBoneToWorld, int boneMask )
{
	VPROF_BUDGET( "CHL2MP_Player::SetupBones", VPROF_BUDGETGROUP_SERVER_ANIM );

	// Set the mdl cache semaphore.
	MDLCACHE_CRITICAL_SECTION();

	// Get the studio header.
	Assert( GetModelPtr() );
	CStudioHdr *pStudioHdr = GetModelPtr( );

	Vector pos[MAXSTUDIOBONES];
	Quaternion q[MAXSTUDIOBONES];

	// Adjust hit boxes based on IK driven offset.
	Vector adjOrigin = GetAbsOrigin() + Vector( 0, 0, m_flEstIkOffset );

	// FIXME: pass this into Studio_BuildMatrices to skip transforms
	CBoneBitList boneComputed;
	if ( m_pIk )
	{
		m_iIKCounter++;
		m_pIk->Init( pStudioHdr, GetAbsAngles(), adjOrigin, gpGlobals->curtime, m_iIKCounter, boneMask );
		GetSkeleton( pStudioHdr, pos, q, boneMask );

		m_pIk->UpdateTargets( pos, q, pBoneToWorld, boneComputed );
		CalculateIKLocks( gpGlobals->curtime );
		m_pIk->SolveDependencies( pos, q, pBoneToWorld, boneComputed );
	}
	else
	{
		GetSkeleton( pStudioHdr, pos, q, boneMask );
	}

	CBaseAnimating *pParent = dynamic_cast< CBaseAnimating* >( GetMoveParent() );
	if ( pParent )
	{
		// We're doing bone merging, so do special stuff here.
		CBoneCache *pParentCache = pParent->GetBoneCache();
		if ( pParentCache )
		{
			BuildMatricesWithBoneMerge( 
				pStudioHdr, 
				m_PlayerAnimState->GetRenderAngles(),
				adjOrigin, 
				pos, 
				q, 
				pBoneToWorld, 
				pParent, 
				pParentCache );

			return;
		}
	}

	Studio_BuildMatrices( 
		pStudioHdr, 
		m_PlayerAnimState->GetRenderAngles(),
		adjOrigin, 
		pos, 
		q, 
		-1,
		pBoneToWorld,
		boneMask );
}