//==============Overlord mod=====================
//	Player spawner
//	
//===============================================\

#ifndef C_H_OV_SPAWNER
#define C_H_OV_SPAWNER

#include "GameEventListener.h"

class C_OverlordSpawner : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_OverlordSpawner, C_BaseAnimating);
	DECLARE_DATADESC();
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_OverlordSpawner();
	virtual ~C_OverlordSpawner();

	int GetUses() const;

private:

	bool m_bAlreadyMarked;
	//int m_iUses;
};

#endif