//==============Overlord mod=====================
// Healing crate
//===============================================

#ifndef H_OV_HEALCRATE
#define H_OV_HEALCRATE

class COverlordHealCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordHealCrate, CBaseAnimating);
	DECLARE_DATADESC();
	
	COverlordHealCrate();
	virtual ~COverlordHealCrate();

	virtual void Precache();
	virtual void Spawn();

	virtual void HealThink();

private:
	int m_MaxHealth;

	int m_Health;
	int m_MedkitAmmo;
};



#endif