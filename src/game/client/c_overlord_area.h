//==============Overlord mod=====================
//	Client-side class of the area brush	
//
//===============================================

#ifndef C_H_OV_AREA
#define C_H_OV_AREA

#ifdef _WIN32
#pragma once
#endif

class C_OverlordArea : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordArea, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_OverlordArea();
	virtual ~C_OverlordArea();

	const char * GetAreaName() const { return m_iszAreaName; };
private:
	char m_iszAreaName[32];
};


#endif