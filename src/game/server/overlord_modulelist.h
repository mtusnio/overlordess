//==============Overlord mod=====================
//	Contains list of all modules, allows switching
//	between different lists
//===============================================

#ifndef H_OV_MODULE_LIST
#define H_OV_MODULE_LIST

class COverlordBaseModule;

class COverlordModuleList : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordModuleList, CBaseEntity);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	COverlordModuleList();
	virtual ~COverlordModuleList();

	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	virtual void Activate();

	virtual void FireOnModule(int module);
	COverlordBaseModule * GetModule(int i) const { return m_Module[i]; };
private:
	CNetworkArray(CHandle<COverlordBaseModule>, m_Module, MAX_BUTTONS);

	const char * m_iszButtons[MAX_BUTTONS];
};


#endif