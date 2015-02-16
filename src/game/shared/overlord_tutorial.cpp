//==============Overlord mod=====================
//	Tutorial entity
//===============================================

#ifdef CLIENT_DLL
#define COverlordTutorial C_OverlordTutorial
#endif

class COverlordTutorial : public CBaseEntity
{
public:
	DECLARE_CLASS( COverlordTutorial, CBaseEntity);

	COverlordTutorial();
	~COverlordTutorial();
};

LINK_ENTITY_TO_CLASS(ov_tutorial, COverlordTutorial);

COverlordTutorial::COverlordTutorial()
{
}

COverlordTutorial::~COverlordTutorial()
{
}

