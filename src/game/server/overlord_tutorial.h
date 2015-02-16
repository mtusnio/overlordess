//==============Overlord mod=====================
//	Tutorial entity
//===============================================
#ifndef H_OV_TUTORIAL
#define H_OV_TUTORIAL

#ifdef CLIENT_DLL
#define COverlordTutorial C_OverlordTutorial
#endif

class COverlordTutorial : public CBaseEntity
{
public:
	DECLARE_CLASS( COverlordTutorial, CBaseEntity);
	DECLARE_DATADESC();

	COverlordTutorial();
	~COverlordTutorial();

	virtual void Activate();

	virtual int GetTutorialTeam() const { return m_Team; };
private:
	int m_Team;
	int m_Spawns;
};

#endif