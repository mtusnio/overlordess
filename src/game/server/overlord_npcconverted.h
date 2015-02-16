//==============Overlord mod=====================
//	Converted NPC
//===============================================

#include "ai_baseactor.h"

class COverlordNPCConverted : public CAI_BaseActor
{
public:
	DECLARE_CLASS(COverlordNPCConverted, CAI_BaseActor);
	DECLARE_DATADESC();

	COverlordNPCConverted();
	virtual ~COverlordNPCConverted();

	virtual void Precache();
	virtual void Spawn();

	virtual Class_T Classify() { return CLASS_CONVERTED; };
private:
	const char * m_WeaponName;
};