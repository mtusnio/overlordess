//==============Overlord mod=====================
//	Manager class for the Overlord's in-console
//	view and abilities
//===============================================

#ifndef C_H_OV_CONSOLE_MANAGER
#define C_H_OV_CONSOLE_MANAGER

#include "overlord_dynamictraps.h"
#include "gameeventlistener.h"


class COverlordData;
class C_BasePlayer;
class C_OverlordTrap;
class COverlordCameraMenu;
class C_OverlordBaseModule;

struct dlight_t;


enum AxisLock
{
	NONE = -1,
	Z_AXIS,
	Y_AXIS,
};

#define MAX_CONTROL_GROUPS 10

struct ControlGroupInfo_t
{
	enum
	{
		TYPE_NONE = 0,
		TYPE_TRAP,
		TYPE_MODULE,
	};

	unsigned short m_EntityType;
	CHandle<C_BaseEntity> m_hEntity;

	bool operator==(const ControlGroupInfo_t & rhs) const
	{
		return (m_EntityType == rhs.m_EntityType) && (m_hEntity == rhs.m_hEntity);
	}

};

typedef CUtlVector<ControlGroupInfo_t> ControlGroup_t;

class COverlordConsoleManager : public CGameEventListener
{
public:
	COverlordConsoleManager();
	virtual ~COverlordConsoleManager();

	virtual void FireGameEvent( IGameEvent *event );

	void Precache();

	void Run();

	bool IsInited() const { return m_bInit; };

	C_BasePlayer * GetSelectedPlayer() const { return ToBasePlayer(m_Selected.Get()); };
	bool IsPlayerSelected() const { return (GetSelectedPlayer() != NULL); };

	C_OverlordTrap * GetSelectedTrap() const { return COverlordTrap::EntityToTrap(m_Selected.Get()); };
	bool IsTrapSelected() const { return (GetSelectedTrap() != NULL); };

	Vector GetSelectedLocation() const;
	Vector GetPlaneNormal() const;
	
	bool IsInBuildMode() const { return m_BuildMode != BL_NONE; };
	Build_t GetBuildMode() const { return m_BuildMode; };

	bool IsPowerupPanelOpen() const { return m_bPowerupPanel; };
	void SetPowerupPanelOpen(bool bOpen);

	// Prepares the entity for passing it to the Build
	void BuildGhost(const char * type) { BuildGhost(type, GetSelectedLocation()); };
	void BuildGhost(const char * type, const Vector & location);
	C_OverlordTrap * GetGhost() const { return m_Building.Get(); };

	// Builds ghosted entity
	void Build() { Build(GetSelectedLocation()); };
	void Build(const Vector & location);
	//void BuildFromScratch(int index);

	void CancelBuildMode();
	void ExitBuildMode();

	C_OverlordTrap * GetHoverTrap() const { return m_HoverTrap; };

	void EmitSound(const char * sound) const;
	bool IsInFreeroam() const { return m_bFreeroam; };
	bool IsRoaming() const;
	bool IsInMouseRotation() const;


	bool IsZLocked() const;
	bool IsYLocked() const;

	void RotateTrap(int x, int y);
	void MoveTrap();

	bool ShouldEnterFreeroam() const;

	C_OverlordBaseModule * GetHoverStaticTrap() const { return m_StaticTrap; };
	QAngle GetBuildingAngle(Vector normal) const;

	void				   HandleAllNumericKeys();
	void				   HandleNumericKey(short key, bool bControlPressed);

	void				   AddToControlGroup(C_BaseEntity * pEnt, int group, unsigned short type, bool bSound = true);
	void				   RemoveFromControlGroup(C_BaseEntity * pEnt, int group);
	void				   WipeControlGroup(int group) { if(m_ControlGroup[group].Count() > 0) m_ControlGroup[group].Purge(); };
	bool				   IsInControlGroup(int group, C_BaseEntity * pEnt) const;
	const ControlGroup_t & GetControlGroup(int group) const { return m_ControlGroup[group]; };

	bool					ShouldOverrideForNormal(const Vector & normal) const;
private:
	void Init();
	void DeInit();

	void RegisterGlowEntity(C_BaseEntity * pEnt, Color clr);
	void DeregisterGlowEntity();

	void HandleManagerHints();

	void EnterFreeroam(const Vector & pos = vec3_origin, const QAngle & angle = vec3_angle);

	void OutsideBuildModeThink();
	void BuildModeThink();

	void DoMouseButtonChecks();

	void HandleLeftPressed(void);
	void HandleMiddlePressed(void);
	void HandleRightPressed(void);

	bool IsNonPanelMouseClick(void) const;
	inline bool CanSelectPlayer(const C_BasePlayer * pPlayer) const;

	bool CanBuildTrap() const;
	bool CanBuildOnEntity(C_BaseEntity * pEnt) const;
	bool CanBuildOnSurface(int flags) const;
	bool CanBuildOnSurface() const { return CanBuildOnSurface(m_LastSurfFlags); };

	C_BaseEntity * SelectEntity(Vector * position = NULL, trace_t * tra = NULL);

	void HandleColours();

	bool IsSelected() const { return (m_Selected.Get() != NULL); };

	void SelectPlayer(C_BasePlayer * pPlayer);
	void DeselectPlayer() { if(IsPlayerSelected()) DeselectEntity(); };

	void DeselectEntity();

	void SelectDynamicTrap(C_OverlordTrap * pTrap);

	void EnterBuildMode(bool bShowTrapMenu = true);

	void FireHint(const char * hint) const;

	void DoCameraJump();
	void DoCameraJump(const trace_t & tra);

	void ClickOnTrap(C_OverlordTrap * pTrap, bool bRightMouseButton = false);

	void CloseAllAuxiliaryPanels();
	// Also returns a trace
	C_BaseEntity * GetEntityUnderCursor(trace_t * tra = NULL);

	void		   HandleStaticTrapHover();

	// Overrides DEFAULT placement angles for this surface
	inline void OverrideAngles(bool bOverride, Vector normal = vec3_origin, QAngle angle = vec3_angle);

	ControlGroup_t & GetControlGroup(int group) { return m_ControlGroup[group]; };

	CHandle<C_BaseEntity> m_Selected;

	bool m_bPowerupPanel;

	// Last entity the trap was built on
	CHandle<C_BaseEntity> m_LastEntity;
	unsigned short m_LastSurfFlags;

	// Last spot. In global worldspace if no parent specified, in local with a parent
	Vector m_vSelection;

	
	// Our trap in ghost form
	CHandle<C_OverlordTrap> m_Building;
	// Trap we are or not hovering over
	CHandle<C_OverlordTrap> m_HoverTrap;
	// Static trap we're hovering over
	CHandle<C_OverlordBaseModule> m_StaticTrap;

	EHANDLE m_GlowEntity;

	// Last entity type built
	int m_LastIndex;

	//bool m_BuildMode;
	int     m_GroupControl;
	Build_t m_BuildMode;
	bool m_bInit;
	Vector m_LastNormal;

	int m_RadiusTexture;
//	float m_flPlayerUpdate;
	dlight_t * m_pDynamicLight;

	IMaterial * m_BeamMat;

	float m_flFreezeBuild;

	// Freeroam mode boolean
	bool m_bFreeroam;
	//int m_FreeroamX;
	//int m_FreeroamY;
	float m_flInputDelay;

	ControlGroup_t m_ControlGroup[MAX_CONTROL_GROUPS];

	// Angle overrides for ghost
	bool m_bOverrideAngle;
	QAngle m_angOverride;
	// Normal for which to override angles
	Vector m_NormalOverride;
	AxisLock m_AxisLock;

	float m_flRightClickHoldDelay;
};

inline bool COverlordConsoleManager::CanSelectPlayer(const CBasePlayer * pPlayer) const
{
	if(!pPlayer)
		return false;

	if(!pPlayer->IsRebel())
		return false;

	if(pPlayer->IsInvisible())
		return false;

	if(!pPlayer->IsAlive())
		return false;

	return true;
}

inline void COverlordConsoleManager::OverrideAngles(bool bOverride, Vector normal, QAngle angle)
{
	m_bOverrideAngle = bOverride;
	m_NormalOverride = normal;
	m_angOverride = angle;
}

extern COverlordConsoleManager g_ConsoleManager;

#endif