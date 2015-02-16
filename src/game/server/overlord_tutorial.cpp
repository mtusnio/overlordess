//==============Overlord mod=====================
//	Tutorial entity
//===============================================
#include "cbase.h"
#include "overlord_tutorial.h"
#include "hl2mp_gamerules.h"

LINK_ENTITY_TO_CLASS(ov_tutorial, COverlordTutorial);

BEGIN_DATADESC( COverlordTutorial )

DEFINE_KEYFIELD(m_Team, FIELD_INTEGER, "TutorialTeam"),
DEFINE_KEYFIELD(m_Spawns, FIELD_INTEGER, "TutorialSpawns"),

END_DATADESC()

COverlordTutorial::COverlordTutorial()
{
	
}

COverlordTutorial::~COverlordTutorial()
{
}

void COverlordTutorial::Activate()
{
	BaseClass::Activate();

	GET_OVERLORD_DATA->SetTutorial(true);
	GET_OVERLORD_DATA->SetSpawnsAmount(m_Spawns);
}