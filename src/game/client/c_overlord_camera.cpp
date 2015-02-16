//==============Overlord mod=====================
//	Finally a client-side overlord camera 
//	(hooray!)
//===============================================

#include "cbase.h"
#include "c_overlord_camera.h"
#include "c_overlord_area.h"
#include "over_popupsupport.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"

ConVar eo_predict_cameras("eo_predict_cameras", "1");

IMPLEMENT_CLIENTCLASS_DT( C_OverlordCamera, DT_OverlordCamera, COverlordCamera)

	RecvPropArray3( RECVINFO_ARRAY(m_lLists), RecvPropEHandle( RECVINFO(m_lLists[0]) ) ),
	RecvPropInt(RECVINFO(m_CurrentList) ),
	RecvPropString(RECVINFO(m_iszDesc) ),
	RecvPropVector( RECVINFO( m_vOffset ) ),
	RecvPropEHandle(RECVINFO(m_pNext) ),
	RecvPropEHandle(RECVINFO(m_pPrevious) ),
	RecvPropEHandle(RECVINFO(m_pUpper)),
	RecvPropEHandle(RECVINFO(m_pLower)),
	RecvPropEHandle(RECVINFO(m_pArea)),

	RecvPropInt(RECVINFO(m_spawnflags)),

	RecvPropFloat(RECVINFO(m_flFirstMove)),
	RecvPropFloat(RECVINFO(m_flLastMove)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_OverlordCamera)

	DEFINE_PRED_FIELD( m_angRotation, FIELD_VECTOR, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK),
	DEFINE_PRED_FIELD( m_angAbsRotation, FIELD_VECTOR,  FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK),
	DEFINE_PRED_FIELD( m_angNetworkAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK),

	DEFINE_PRED_FIELD_TOL( m_flFirstMove, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.01f),
	DEFINE_PRED_FIELD_TOL( m_flLastMove, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.01f),

END_PREDICTION_DATA()

BEGIN_DATADESC(C_OverlordCamera)

END_DATADESC()

C_OverlordCamera::C_OverlordCamera()
{
	//SetPredictionEligible(true);
}

C_OverlordCamera::~C_OverlordCamera()
{
}

void C_OverlordCamera::Spawn()
{
	BaseClass::Spawn();

	CREATE_TURRET();

	SetNextClientThink(gpGlobals->curtime + CAMERA_THINK);
}

int C_OverlordCamera::DrawModel(int flags)
{
	C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer || !pPlayer->IsOverlord())
		return 0;

	C_OverlordData * pData = GET_OVERLORD_DATA;

	if(!pData || !pData->IsInVehicle() || pData->GetCamera() == this)
		return 0;

	return BaseClass::DrawModel(flags);
}

void C_OverlordCamera::ClientThink()
{
	GET_OVERLORD_DATA_ASSERT(pData);

	if(eo_predict_cameras.GetBool() && pData->GetOverlord() == C_BasePlayer::GetLocalPlayer())
	{		
		if(pData->GetCamera() == this)
		{
			C_BasePlayer * pPlayer = C_BasePlayer::GetLocalPlayer();
			
			int nButtons = pPlayer->m_nButtons;
			//int afButtons = pPlayer->m_afButtonPressed;

			//QAngle angle = GetLocalAngles();
			//DevMsg("Before: %f %f %f\n", angle.x, angle.y, angle.z);

			if(nButtons & IN_FORWARD)
				 MoveUp();
			if(nButtons & IN_BACK)
				 MoveDown();
			if(nButtons & IN_MOVELEFT)
				 RotateLeft();
			if(nButtons & IN_MOVERIGHT)
				 RotateRight();

			QAngle angle = GetLocalAngles();
			SetNetworkAngles(angle);
			SetAbsAngles(angle);
			//DevMsg("After: %f %f %f\n", angle.x, angle.y, angle.z);
		}
	}
	else
	{
		SetNextClientThink(gpGlobals->curtime + 0.65f);
	}


	SetNextClientThink(gpGlobals->curtime + CAMERA_THINK);
}

sTrapDescription C_OverlordCamera::GetButtonRecord(int i) const
{
	GET_OVERLORD_DATA_RETURN(pData, sTrapDescription());

	if(!pData->GetCamera())
		return sTrapDescription();

	C_OverlordBaseModule * pModule = pData->GetCamera()->GetModule(i);

	if(!pModule)
		return sTrapDescription();

	return sTrapDescription(pModule->GetModuleDescription(), pModule->GetModuleCost(), pModule->GetDuration());

}

const char * C_OverlordCamera::GetButtonCaption(int i) const
{
	GET_OVERLORD_DATA_RETURN(pData, NULL);

	if(!pData->GetCamera())
		return NULL;

	C_OverlordBaseModule * pModule = pData->GetCamera()->GetModule(i);

	if(!pModule)
		return NULL;

	return pModule->GetModuleName();

}

Color C_OverlordCamera::GetButtonColor(int i) const
{
	GET_OVERLORD_DATA_RETURN(pData, Color(0, 0, 0, 0));

	C_OverlordBaseModule * pModule = GetModule(i);

	if(!pModule)
		return Color(0, 0, 0, 0);

	Color bgColor(0, 0, 0, BACKGROUND_ALPHA);
	if(!pModule->IsEnabled())
	{
		bgColor.SetColor(180, 0, 0, BACKGROUND_ALPHA);
	}
	else if(pModule->IsActivated())
	{
		bgColor.SetColor(60, 0, 0, BACKGROUND_ALPHA);
	}
	else if(pModule->GetModuleCost() > pData->GetPower())
	{
		bgColor.SetColor(120, 0, 0, BACKGROUND_ALPHA);
	}
	else if(pModule->IsCoolingDown())
	{
		// Calculate color for the cooldown
		int blue = 255 - (pModule->GetRemainingCooldown() / pModule->GetCooldownLength()) * 225;

		//if(blue < 100 || blue > 255)
			//blue = 255;

		if(blue < 30 || blue > 255)
			blue = 30;

		bgColor.SetColor(0, 0, blue, BACKGROUND_ALPHA);
	}
	else
	{
		//if(pModule->GetModuleCost() <= 0 && pModule->GetDuration() <= 0 && pModule->GetCooldownLength() <= 0)
		//	bgColor.SetColor(0, 0, 0, BACKGROUND_ALPHA);
		//else
			bgColor.SetColor(0, 85, 0, BACKGROUND_ALPHA);
	}

	return bgColor;
}

bool C_OverlordCamera::ShouldShowListButtons() const
{
	if(!GetList(0) || !GetList(1))
		return false;

	return true;
}

float C_OverlordCamera::GetMoveSpeed() const
{	
	// The speed in Hammer is in units per 0.1 ms.
	const float baseSpeed = COverlordBaseTurret::GetMoveSpeed() * CAMERA_THINK / 0.1f;

	if(!(m_spawnflags & SF_NOSPEEDCHANGES))
	{
		const float DIFF_SLOWDOWN = 0.40f;
		const float DIFF_SPEEDUP = DIFF_SLOWDOWN + 1.250f;

		if(m_flFirstMove + DIFF_SLOWDOWN > gpGlobals->curtime)
		{
			// Percentage of the difference between curtime
			// and first move
			//float percent = (DIFF - ((flFirstMove + DIFF) - gpGlobals->curtime))/DIFF;
			float percent = (gpGlobals->curtime - m_flFirstMove)/DIFF_SLOWDOWN;

			// We need to make sure now that
			// we don't end up with speed being nearly 0
			// We square root the percent to acquire faster speed rate
			//float tmp = baseSpeed * FastSqrt(percent);
			float tmp = baseSpeed * percent;

			float speed = tmp > 0.25 ? tmp : 0.25;

			return speed;
		}
		else if(m_flFirstMove + DIFF_SPEEDUP <= gpGlobals->curtime)
		{
			float treshold = m_flFirstMove + DIFF_SPEEDUP * 2;

			float percent;

			if(treshold <= gpGlobals->curtime)
				 percent = 1.0f;
			else
			{
				percent = FastSqrt(1.0f - (treshold - gpGlobals->curtime)/DIFF_SPEEDUP);
			}

			return (baseSpeed * percent) + baseSpeed;

		}
	}
	
	return baseSpeed;

}