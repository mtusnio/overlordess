//==============Overlord mod=====================
//	Proxy class for the data
//===============================================

#ifndef H_OV_DATA_PROXY
#define H_OV_DATA_PROXY

class COverlordData;

class COverlordDataProxy : public CBaseEntity
{
	friend class COverlordData;
public:
	DECLARE_CLASS(COverlordDataProxy, CBaseEntity);
	DECLARE_DATADESC();

	COverlordDataProxy();
	virtual ~COverlordDataProxy();

	virtual void Activate();


private:

};



#endif