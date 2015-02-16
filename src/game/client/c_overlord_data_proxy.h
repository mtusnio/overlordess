//==============Overlord mod=====================
//	Client data proxy
//===============================================

#ifndef C_H_OV_DATAPROXY
#define C_H_OV_DATAPROXY

class COverlordData;

class C_OverlordDataProxy : public C_BaseEntity
{
	friend class COverlordData;
public:
	DECLARE_CLASS(C_OverlordDataProxy, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_OverlordDataProxy();
	virtual ~C_OverlordDataProxy();

private:
	
	// Keep them as CNetworkVars here too for shared code
	CNetworkVar(float, m_flSecondsPerHealth);
	CNetworkVar(float, m_flSpawnMultiplier);
	CNetworkVar(int, m_iStartHealth);
	CNetworkVar(int, m_iHealthCap);
	CNetworkVar(int, m_iMaxTraps);
};

#endif