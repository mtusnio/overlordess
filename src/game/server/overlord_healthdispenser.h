//==============Overlord mod=====================
// Healing machine
//===============================================

#ifndef H_OV_HEALTHDISPENSER
#define H_OV_HEALTHDISPENSER


class COverlordDispenser : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordDispenser, CBaseAnimating);
	DECLARE_DATADESC();
	
	COverlordDispenser();
	virtual ~COverlordDispenser();

	virtual void Precache();
	virtual void Spawn();
};


#endif
