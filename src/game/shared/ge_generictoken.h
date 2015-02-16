//==============Overlord mod=====================
//	Generic token for glow effect
//  taken from developer wiki
//  http://developer-new.valvesoftware.com/wiki/L4D_Glow_Effect
//===============================================

#ifndef H_OV_GENERICTOKEN
#define H_OV_GENERICTOKEN

#ifdef CLIENT_DLL
#include "ge_screeneffects.h"

#define CGenericToken C_GenericToken
#endif

class CGenericToken
{
public:
	DECLARE_CLASS_NOBASE(CGenericToken, CBaseEntity);
	DECLARE_NETWORKCLASS();

	CGenericToken();
	virtual ~CGenericToken();

#ifdef GAME_DLL
	// Function call to set this entitie's glow
	virtual void SetGlow( bool state, Color glowColor = Color(255,255,255) );
#else
	// This is called after we receive and process a network data packet
	virtual void PostDataUpdate( DataUpdateType_t updateType );
#endif
 
private:
#ifdef CLIENT_DLL
	CEntGlowEffect *m_pEntGlowEffect;
	bool m_bClientGlow;
#endif
	CNetworkVar( bool, m_bEnableGlow );
	CNetworkVar( color32, m_GlowColor );
};


#endif