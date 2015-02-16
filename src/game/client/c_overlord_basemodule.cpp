//==============Overlord mod=====================
//	Client-side implementation of the base
// module
//===============================================

#include "cbase.h"
#include "c_overlord_basemodule.h"
#include "c_overlord_modulelist.h"
#include "hl2mp_gamerules.h"

IMPLEMENT_NETWORKCLASS_ALIASED( OverlordBaseModule, DT_OverlordBaseModule )


BEGIN_NETWORK_TABLE( C_OverlordBaseModule, DT_OverlordBaseModule )
	RecvPropInt( RECVINFO( m_iCost ) ),
	RecvPropFloat( RECVINFO( m_flLength ) ),
	RecvPropBool( RECVINFO(m_bEnabled ) ),
	RecvPropBool(RECVINFO(m_bActivated) ),
	RecvPropFloat(RECVINFO(m_flCooldown) ),
	RecvPropFloat(RECVINFO(m_flDeactTime) ),
	RecvPropString( RECVINFO(m_ModuleName) ),
	RecvPropString( RECVINFO(m_ModuleDescription) ),
	RecvPropInt(RECVINFO(m_spawnflags)),
	RecvPropEHandle(RECVINFO(m_GlowEntity)),

END_RECV_TABLE()

void C_OverlordBaseModule::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if(m_spawnflags & SF_DO_NOT_DRAW)
		return;

	if(!IsGlowRegistered())
		RegisterGlow(GetIndicatorColor(), 0.15f);

	if(m_GlowEntity && !m_GlowEntity->IsGlowRegistered())
	{
		m_GlowEntity->RegisterGlow(GetIndicatorColor(), 0.15f);
	}
}

int C_OverlordBaseModule::DrawModel(int flags)
{
	COverlordData * pData = GET_OVERLORD_DATA;
	if(pData && (pData->GetOverlord() == C_BasePlayer::GetLocalPlayer()) && pData->IsInVehicle())
	{
			if(!(m_spawnflags & SF_DO_NOT_DRAW))
			{
				Color clr = GetIndicatorColor();

				SetGlowColor(clr);
				if(m_GlowEntity)
					m_GlowEntity->SetGlowColor(clr);

				return BaseClass::DrawModel(flags);
			}
	}

	if(m_GlowEntity && m_GlowEntity->IsGlowRegistered())
		m_GlowEntity->DeregisterGlow();

	return 0;
}


Color C_OverlordBaseModule::GetIndicatorColor() const
{
	GET_OVERLORD_DATA_RETURN(pData, Color(0, 0, 0));
	Color bgColor(0, 0, 0, BACKGROUND_ALPHA);
	if(!IsEnabled())
	{
		bgColor.SetColor(0, 0, 0, BACKGROUND_ALPHA);
	}
	else if(IsActivated() && ShouldNeverDeactivate())
	{
		bgColor.SetColor(150, 0, 0, BACKGROUND_ALPHA);
	}
	else if(IsActivated())
	{
		bgColor.SetColor(125, 70, 0, BACKGROUND_ALPHA);
	}
	else if(IsCoolingDown())
	{
		// Calculate color for the cooldown
		int blue = 255 - (GetRemainingCooldown() / GetCooldownLength()) * 255;


		blue = clamp(blue, 20, 255);

	

		bgColor.SetColor(0, 0, blue, BACKGROUND_ALPHA);
	}
	else if(GetModuleCost() > pData->GetPower())
	{
		bgColor.SetColor(120, 0, 0, BACKGROUND_ALPHA);
	}
	else
	{
		//if(pModule->GetModuleCost() <= 0 && pModule->GetDuration() <= 0 && pModule->GetCooldownLength() <= 0)
		//	bgColor.SetColor(0, 0, 0, BACKGROUND_ALPHA);
		//else
			bgColor.SetColor(0, 125, 0, BACKGROUND_ALPHA);
	}

	return bgColor;
}