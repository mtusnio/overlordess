//==============Overlord mod=====================
//	Module entity
//===============================================

#include "overlord_basemodule.h"

class COverlordLogicModule : public COverlordBaseModule
{
public:
	DECLARE_CLASS(COverlordLogicModule, COverlordBaseModule);
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();

	COverlordLogicModule();
	virtual ~COverlordLogicModule();

	//virtual void InputActivate(inputdata_t &inputData);
	//virtual void Deactivate();
protected:
	virtual void RunModule() { };

};