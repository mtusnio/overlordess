//==============Overlord mod=====================
//	Module entity
//===============================================

#ifndef C_H_OV_LOGMODULE
#define C_H_OV_LOGMODULE

#include "c_overlord_basemodule.h"

#define COverlordLogicModule C_OverlordLogicModule

class C_OverlordModuleList;

class C_OverlordLogicModule : public C_OverlordBaseModule
{
public:
	DECLARE_CLASS(C_OverlordLogicModule, C_OverlordBaseModule);
	DECLARE_NETWORKCLASS();

	C_OverlordLogicModule();
	virtual ~C_OverlordLogicModule();

	virtual bool ShouldGlow() const { return true; };
	//virtual Color GetGlowColour() const { return Color(184, 134, 11); };
};




#endif