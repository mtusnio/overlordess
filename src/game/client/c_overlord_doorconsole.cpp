//==============Overlord mod=====================
//	Door console
//	
//===============================================

#include "cbase.h"
#include "c_overlord_doorconsole.h"
#include "hud.h"
#include "view_scene.h"

BEGIN_DATADESC(C_OverlordDoorLock)

END_DATADESC()

BEGIN_PREDICTION_DATA(C_OverlordDoorLock)

END_PREDICTION_DATA()

IMPLEMENT_CLIENTCLASS_DT(C_OverlordDoorLock, DT_OverlordDoorLock, COverlordDoorLock)

	RecvPropFloat(RECVINFO(m_flHackTime)),	
	RecvPropFloat(RECVINFO(m_flHacked)),
	RecvPropBool(RECVINFO(m_bLocked)),

	RecvPropString(RECVINFO(m_HackedString)),
	RecvPropString(RECVINFO(m_HackingString)),

END_RECV_TABLE()

LINK_ENTITY_TO_CLASS(over_doorconsole, C_OverlordDoorLock);


C_OverlordDoorLock::C_OverlordDoorLock()
{
}

C_OverlordDoorLock::~C_OverlordDoorLock()
{
}

void C_OverlordDoorLock::Spawn()
{
	BaseClass::Spawn();

	SetNextClientThink(gpGlobals->curtime + 0.1f);
}

const char * C_OverlordDoorLock::GetTargetText() const
{
	static char chText[64];

	int iTime = m_flHackTime;

	if(!m_bLocked)
	{
		Q_strncpy(chText, m_HackedString, ARRAYSIZE(chText)); 
	}
	else
	{
		Q_snprintf(chText, ARRAYSIZE(chText), m_HackingString, iTime);
	}

	return chText;
}

#define BAR_WIDTH 512
#define BAR_HEIGHT 128

void C_OverlordDoorLock::ClientThink()
{
	/*if(m_flHacked > 0.0f)
	{
		C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

		if(pPlayer && pPlayer->IsRebel())
		{
			float dist = (pPlayer->RealEyePosition() - WorldSpaceCenter()).Length();

			if(dist <= 384.0f)
			{
				Vector pos = WorldSpaceCenter() + Vector(0, 0, 8);
				
				Vector screen;
				ScreenTransform(pos, screen);

				bool bShow = !((screen.x == 0 && screen.y == 0) 
					|| (screen.x < -1.0f || screen.x > 1.0f || screen.y > 1.0f || screen.y < -1.0f));

				DevMsg("Testing...\n");
				if(bShow)
				{
					
					const CViewSetup * setup = view->GetViewSetup();

					if(setup)
					{
						int yCenter = setup->height / 2;
						int xCenter = setup->width / 2;

						int x = xCenter + (xCenter * screen.x);
						int y = yCenter - (yCenter * screen.y);

						DevMsg("Drawing...\n");

						float percentage = m_flHacked/m_flHackTime;

						if(percentage > 1.0f)
							percentage = 1.0f;

						gHUD.DrawProgressBar(770, 450, BAR_WIDTH, BAR_HEIGHT, percentage, COLOR_YELLOW, CHud::HUDPB_HORIZONTAL);
					}
				}
			}
		}
	}

	SetNextClientThink(CLIENT_THINK_ALWAYS);*/
}