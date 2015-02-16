//==============Overlord mod=====================
//	Dynamic traps baseclass
//===============================================

#ifndef C_H_OV_DYNAMICTRAPS
#define C_H_OV_DYNAMICTRAPS

class C_OverlordTrap : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordTrap, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_OverlordTrap();
	~C_OverlordTrap();
};

#endif