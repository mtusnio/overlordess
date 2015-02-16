//==============Overlord mod=====================
//	Path node base class
//===============================================

#ifndef H_OV_PATHNODE
#define H_OV_PATHNODE

#define SF_SPAWN_NODE 1
#define SF_MARK_ON_INPUT 2
#define SF_CAN_ALWAYS_REACH 4
#define SF_CAN_SPAWN 8
#define SF_CANNOT_BLINK 16
#define SF_FIRST_NODE 32

class CSprite;

class COverlordPathNode : public CBaseAnimating
{
public:
	static COverlordPathNode * GetFirstNode() { return m_FirstNode; };
	// Returns last node reached
	static COverlordPathNode * GetLastNode() { return m_LastNode; };
	static COverlordPathNode * GetSpawnNode() { return m_SpawnNode; };
	static COverlordPathNode * GetMostPopulatedNode();

	DECLARE_CLASS(COverlordPathNode, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	COverlordPathNode();
	virtual ~COverlordPathNode();

	virtual void Precache();
	virtual void Spawn();

	virtual void Think();

	virtual COverlordPathNode * GetNextNode() const { return m_NextNode; };
	virtual COverlordPathNode * GetPreviousNode() const { return m_PrevNode; };
	virtual float				GetPathLength() const { return m_PathLength; };
	virtual int  				GetTeleportDistance() const { return m_iSpawnDistance; };
	virtual bool				WasInited() const { return m_bInitialized; };
	virtual bool				IsAltNode() const { return m_bIsAltNode; };
	virtual bool				WasReached() const { return m_bReached; };
	virtual bool				IsSpawnNode() const { return m_spawnflags & SF_SPAWN_NODE; };
	virtual bool				CanReachThisNode() const;
	virtual bool				CanSpawnNode() const { return IsSpawnNode() || (m_spawnflags & SF_CAN_SPAWN) != 0; };
	virtual bool				CanOnlyMarkOnInput() const { return !m_bMarkedByInput && ((m_spawnflags & SF_MARK_ON_INPUT) != 0); };
	virtual bool				CanBlinkToThis() const { return (m_spawnflags & SF_CANNOT_BLINK) == 0; };
	virtual int					GetPlayerCount() const { return m_PlayersPassed.Count(); };
	virtual bool				IsLastNode() const { return GetLastNode() == this; };
	virtual bool				IsFirstNode() const { return GetFirstNode() == this; };

	virtual void				InitializeNode(COverlordPathNode * prev, float length);
	virtual void				InitializeAltNode(COverlordPathNode * prev, float length);

	// pNode specifies the node to remove to
	virtual void				RemoveFromForwardChain(CBasePlayer * pPlayer, COverlordPathNode * pNode = NULL);
	virtual void				RemoveFromChain(CBasePlayer * pPlayer);

	virtual void				RespawnPlayer(CBasePlayer * pPlayer) const;
	virtual void				TeleportPlayer(CBasePlayer * pPlayer) const;
	/* Inputs */
	void						InputSetNextNode( inputdata_t &inputdata);
	void						InputSetAsReached( inputdata_t &inputdata);

	/* Comparison operators. Greater node means that it's further down the chain */
	bool operator>(const COverlordPathNode & rhs) const
	{
		if(this == &rhs)
			return false;

		if(&rhs == GetFirstNode())
			return false;

		const COverlordPathNode * prev = this;
		while(true)
		{
			prev = prev->GetPreviousNode();
			if(!prev)
				return false;

			if(prev == &rhs)
				return true;
		}

		return false;
	}

	bool operator<(COverlordPathNode & rhs) const
	{
		if(this == &rhs)
			return false;

		return rhs > *this;
	}

protected:
	virtual void				SetAsReached(CBasePlayer * pPlayer = NULL);
	virtual void				AddToList(CBasePlayer * pPlayer);
	virtual void				RemoveFromList(CBasePlayer * pPlayer);
	virtual void				DisplayHint();

	static CHandle<COverlordPathNode> m_FirstNode;
	static CHandle<COverlordPathNode> m_LastNode;
	static CHandle<COverlordPathNode> m_SpawnNode;

	bool m_bMarkedByInput;
	bool m_bInitialized;

	const char * m_NextLinkName;
	const char * m_AltLinkName;

	float m_flReachDistance;
	int m_iSpawnDistance;

	const char * m_szHint;

	COutputEvent m_OnFirstPlayerReached;
	COutputEvent m_OnPlayerReached;

	CUtlVector<CHandle<CBasePlayer>> m_PlayersPassed;

	CNetworkHandle(COverlordPathNode, m_NextNode);
	CNetworkHandle(COverlordPathNode, m_PrevNode);
	CNetworkHandle(COverlordPathNode, m_AltNode);
	CNetworkVar(float, m_PathLength);
	CNetworkVar(bool, m_bReached);
	CNetworkVar(bool, m_bIsAltNode);
};

class COverlordPathStart : public COverlordPathNode
{
public:
	DECLARE_CLASS(COverlordPathStart, COverlordPathNode);

	virtual void Activate();

	virtual bool				IsSpawnNode() const { return true;};
	virtual void				RemoveFromForwardChain(CBasePlayer * pPlayer, COverlordPathNode * pNode = NULL) { };
	virtual void				RemoveFromChain(CBasePlayer * pPlayer) { };
};


#endif