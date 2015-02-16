//==============Overlord mod=====================
//	Brush which emits particles inside of it
//===============================================

#ifndef H_OV_PARTICLEBRUSH
#define H_OV_PARTICLEBRUSH

#define SF_CLIENTSIDE 2

class COverlordParticleBrush : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordParticleBrush, CBaseEntity);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	COverlordParticleBrush();
	virtual ~COverlordParticleBrush();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

	virtual int UpdateTransmitState();
private:
	CNetworkVar(string_t, m_ParticleSystem);
	CNetworkVar(float, m_minTime);
	CNetworkVar(float, m_maxTime);
};

#endif