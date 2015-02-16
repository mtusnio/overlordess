//==============Overlord mod=====================
// Effect for the minimap
//===============================================

 #include "cbase.h" 
 #include "te_effect_dispatch.h" 
  
 void MakeSphere(const Vector &location, float flScale, bool overlord, float lifetime) 
 {
        CEffectData data; 
 
        data.m_vOrigin = location; 
        data.m_flScale = flScale; 
		data.m_bOverlord = overlord;
		data.m_flLifeTime = lifetime;
  
        DispatchEffect( "Sphere", data ); 
} 