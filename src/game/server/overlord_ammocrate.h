//==============Overlord mod=====================
// Ammo-refilling crate
//===============================================

#ifndef H_OV_AMMOCRATE
#define H_OV_AMMOCRATE

#define SF_NO_AMMO 2
//#define SF_NO_HEALTH 1

class COverlordAmmoCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordAmmoCrate, CBaseAnimating);
	DECLARE_DATADESC();
	
	COverlordAmmoCrate();
	virtual ~COverlordAmmoCrate();

	virtual void Precache();
	virtual void Spawn();

	virtual void AmmoThink();

	virtual bool ShouldOpenForPlayer(CBasePlayer * pPlayer);
private:
	struct CooldownRecord
	{
		CHandle<CBasePlayer> m_Player;
		float m_flEndTime;
	};

	CooldownRecord m_Records[MAX_PLAYERS];
	float m_flCloseTime;
	float m_flOpenDelay;
};



#endif