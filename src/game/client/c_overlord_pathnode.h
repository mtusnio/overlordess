//==============Overlord mod=====================
//	Path node base class
//===============================================

#ifndef C_H_OV_PATHNODE
#define c_H_OV_PATHNODE

#define SF_SPAWN_NODE 1
#define SF_MARK_ON_INPUT 2
#define SF_CAN_ALWAYS_REACH 4
#define SF_CAN_SPAWN 8
#define SF_CANNOT_BLINK 16
#define SF_FIRST_NODE 32

#ifdef CLIENT_DLL
#define COverlordPathNode C_OverlordPathNode
#endif

class C_OverlordPathNode : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_OverlordPathNode, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	C_OverlordPathNode();
	virtual ~C_OverlordPathNode();

	static C_OverlordPathNode * GetFirstNode();
	static C_OverlordPathNode * GetSpawnNode();
	static C_OverlordPathNode * GetLastNode();

	virtual void OnDataChanged(DataUpdateType_t updateType);

	virtual void ClientThink();
	virtual int DrawModel(int flags);

	virtual C_OverlordPathNode * GetNextNode() const { return m_NextNode; };
	virtual C_OverlordPathNode * GetPreviousNode() const { return m_PrevNode; };

	virtual float				 GetPathLength() const { return m_PathLength; };
	virtual bool				 WasReached() const { return m_bReached; };
	virtual bool				 IsAltNode() const { return m_bIsAltNode; };
	virtual bool				 IsSpawnNode() const { return m_spawnflags & SF_SPAWN_NODE; };
	virtual bool				 CanSpawnNode() const { return (m_spawnflags & SF_CAN_SPAWN) != 0; };

	virtual Color				 GetNodeColor() const;

	virtual bool				 ShouldShowNode() const;
	virtual Color				 GetBeamToNodeColour(C_OverlordPathNode * node) const;
private:
	void						 DrawLine(C_OverlordPathNode * node, Color clr) const;

	IMaterial * m_BeamMat;

	static CHandle<C_OverlordPathNode> m_FirstNode;

	CHandle<C_OverlordPathNode> m_NextNode;
	CHandle<C_OverlordPathNode> m_PrevNode;
	CHandle<C_OverlordPathNode> m_AltNode;

	float m_PathLength;
	bool m_bReached;
	bool m_bIsAltNode;
	int m_spawnflags;
};


#endif