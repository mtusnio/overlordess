//==============Overlord mod=====================
// Effect for the minimap
//===============================================

#include "cbase.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"

void SphereCallback(const CEffectData &data) 
{
	CSmartPtr<CSimpleEmitter> pSphereEmitter;
    pSphereEmitter = CSimpleEmitter::Create("Sphere");
  
    if (pSphereEmitter == NULL)
		return;
  
	Vector  dest = data.m_vOrigin;
	float   scale = data.m_flScale;
	bool	overlord = data.m_bOverlord;
	float   lifetime = data.m_flLifeTime;
  
	pSphereEmitter->SetSortOrigin(dest);	

	PMaterialHandle hMaterial = pSphereEmitter->GetPMaterial("Effects/yellowflare");

	if(!hMaterial)
		DevMsg("Material not found!");

	SimpleParticle * pParticle;

	pParticle = pSphereEmitter->AddSimpleParticle(hMaterial, dest, lifetime );

	if(pParticle == NULL)
		return;

	pParticle->m_uchStartSize = (unsigned char) scale;
    pParticle->m_uchEndSize   = 0;

	if(!overlord)
	{
		pParticle->m_uchColor[0] = 255; 
		pParticle->m_uchColor[1] = 0; 
		pParticle->m_uchColor[2] = 0; 
	}
	else
	{
		pParticle->m_uchColor[0] = 0; 
		pParticle->m_uchColor[1] = 255; 
		pParticle->m_uchColor[2] = 0; 
	}
  
	pParticle->m_uchStartAlpha = 255;
} 
  
DECLARE_CLIENT_EFFECT( "Sphere", SphereCallback ); 