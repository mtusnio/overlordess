//==============Overlord mod=====================
//	Brush which emits particles inside of it
//===============================================

#ifndef C_H_OV_PARTICLEBRUSH
#define C_H_OV_PARTICLEBRUSH

#define SF_CLIENTSIDE 2

class C_OverlordParticleBrush : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordParticleBrush, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_OverlordParticleBrush();
	virtual ~C_OverlordParticleBrush();

	virtual void Spawn();
	virtual void PostDataUpdate( DataUpdateType_t updateType );

	virtual void ClientThink();
private:
	bool m_bInitialised;

	int m_spawnflags;
	char m_ParticleSystem[128];
	float m_minTime;
	float m_maxTime;
};

#endif