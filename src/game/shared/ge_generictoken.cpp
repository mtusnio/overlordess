//==============Overlord mod=====================
//	Generic token for glow effect
//  taken from developer wiki
//  http://developer-new.valvesoftware.com/wiki/L4D_Glow_Effect
//===============================================
#include "cbase.h"
#include "ge_generictoken.h"

IMPLEMENT_NETWORKCLASS_ALIASED(GenericToken, DT_GenericToken);

BEGIN_NETWORK_TABLE( CGenericToken, DT_GenericToken )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO(m_bEnableGlow) ),
	RecvPropInt( RECVINFO(m_GlowColor), 0, RecvProxy_IntToColor32 ),
#else
	SendPropBool( SENDINFO(m_bEnableGlow) ),
	SendPropInt( SENDINFO(m_GlowColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt ),
#endif
END_NETWORK_TABLE()
 
CGenericToken::CGenericToken( void )
{
#ifndef CLIENT_DLL
	color32 col32 = { 255, 255, 255, 100 };
	m_GlowColor.Set( col32 );
#else
	m_bClientGlow = false;
	m_pEntGlowEffect = (CEntGlowEffect*)g_pScreenSpaceEffects->GetScreenSpaceEffect("ge_entglow");
#endif
 
	m_bEnableGlow = false;
}

CGenericToken::~CGenericToken()
{
}
 
 
#ifndef CLIENT_DLL
void CGenericToken::SetGlow( bool state, Color glowColor /*=Color(255,255,255)*/ )
{
	m_bEnableGlow = state;
	color32 col32 = { glowColor.r(), glowColor.g(), glowColor.b(), glowColor.a() };
 
	m_GlowColor.Set( col32 );
	m_bEnableGlow.Set( state );
}
#else
void CGenericToken::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );
 
	color32 col = m_GlowColor.Get();
	// Did we change glow states?
	if ( m_bClientGlow != m_bEnableGlow )
	{
		if ( m_bEnableGlow )
		{
			// Register us with the effect
			m_pEntGlowEffect->RegisterEnt( this, Color(col.r, col.g, col.b, col.a) );
		}
		else
		{
			// Stop glowing
			m_pEntGlowEffect->DeregisterEnt( this );
		}
 
		m_bClientGlow = m_bEnableGlow;
	}
	else
	{
		// Maybe we changed color? Set it anyway (not a costly function at all)
		m_pEntGlowEffect->SetEntColor( this, Color(col.r, col.g, col.b, col.a) );
	}
}
#endif