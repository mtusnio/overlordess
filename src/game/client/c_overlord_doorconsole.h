//==============Overlord mod=====================
//	Door console
//	
//===============================================

#ifndef H_OV_DOOR_CONSOLE
#define H_OV_DOOR_CONSOLE


#include "c_baseanimating.h"

class C_OverlordDoorLock : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_OverlordDoorLock, C_BaseAnimating);
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();
	DECLARE_CLIENTCLASS();

	C_OverlordDoorLock();
	virtual ~C_OverlordDoorLock();

	virtual void Spawn();

	virtual const char *            GetTargetText() const;
	virtual const char *            GetTargetTextFormat() const { return "#TARGET_TEXT_FORMAT"; };

	virtual void					ClientThink();
private:
	float m_flHackTime;
	float m_flHacked;
	bool m_bLocked;

	char m_HackedString[64];
	char m_HackingString[64];
};
#endif