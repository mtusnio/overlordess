//==============Overlord mod=====================
//	Player spawner
//	
//===============================================

#ifndef H_OV_SPAWNER
#define H_OV_SPAWNER

class CSprite;

class COverlordSpawner : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordSpawner, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();

	COverlordSpawner();
	virtual ~COverlordSpawner();

	virtual void Precache();

	virtual void Spawn();
	virtual void Activate();

	virtual void Think();

	int GetUses() const;

	virtual bool CanSpawn() const;
	virtual CBaseEntity * GetObjectOnTop() const;
	virtual bool SpawnPlayer(CBasePlayer * pPlayer = NULL);

	static COverlordSpawner * GetLastUsed() { return COverlordSpawner::m_LastUsed; };
	void					  SetAsLastUsed();
	void					  DelayedSpawn(float flDelay);

	void					  InputMarkSpawner(inputdata_t & inputdata);

	void					  Mark();
	void					  Unmark();
	
	Vector					  GetSpawnSpot() const;

private:
	virtual bool PushPlayer();

	CHandle<CBasePlayer> m_pRespawn;

	CHandle<CSprite> m_hGlow;

	CNetworkVar(bool,  m_bAlreadyMarked);
	static CHandle<COverlordSpawner> m_LastUsed;
	//CNetworkVar(int, m_iUses);
	float m_flNextSpawn;
	float m_flNextDelayed;
	bool  m_bDelayedSpawn;
	bool  m_bSetAsLast;
	int   m_iQueue;
	int m_AutomarkDistance;
	
};


#endif