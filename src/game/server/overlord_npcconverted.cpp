//==============Overlord mod=====================
//	Converted NPC
//===============================================

#include "cbase.h"
#include "overlord_npcconverted.h"

BEGIN_DATADESC(COverlordNPCConverted)

	DEFINE_KEYFIELD(m_WeaponName, FIELD_STRING, "WeaponName"),

END_DATADESC()

LINK_ENTITY_TO_CLASS(npc_overconverted, COverlordNPCConverted);

COverlordNPCConverted::COverlordNPCConverted()
{
}

COverlordNPCConverted::~COverlordNPCConverted()
{
}

void COverlordNPCConverted::Precache()
{
	BaseClass::Precache();

	PrecacheModel("models/combine_soldier.mdl");
}

void COverlordNPCConverted::Spawn()
{
	Precache();
	SetModel("models/combine_soldier.mdl");

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 200;
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_TURN_HEAD );
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
	CapabilitiesAdd( bits_CAP_SQUAD);
	CapabilitiesAdd( bits_CAP_USE_WEAPONS );

	CapabilitiesAdd( bits_CAP_DUCK );			

	CapabilitiesAdd( bits_CAP_NO_HIT_SQUADMATES );

	NPCInit();

	if(m_WeaponName)
	{
		CBaseCombatWeapon *pWeapon = Weapon_Create(m_WeaponName);

		if(pWeapon)
			Weapon_Equip(pWeapon);
	}
}
