//==============Overlord mod=====================
//	Overlord's teleport weapon
//	
//===============================================

#ifndef H_OV_TELEPORTWEAPON
#define H_OV_TELEPORTWEAPON

#ifdef CLIENT_DLL
#define COverlordTeleport C_OverlordTeleport
#endif

class COverlordTeleport : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(COverlordTeleport, CBaseHL2MPCombatWeapon);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();

	COverlordTeleport();
	virtual ~COverlordTeleport();

	virtual void Precache();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual void ItemPostFrame();

	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	Vector GetCameraOrigin() { return ((m_Target.Get() != NULL) ? m_Target.Get()->RealEyePosition() : vec3_origin); };
	QAngle GetCameraAngles() { return ((m_Target.Get() != NULL) ? m_Target.Get()->EyeAngles() : vec3_angle); };
	bool   GetState() const { return m_bState.Get(); };

	virtual bool IsOverlordWeapon() const { return true; };
	virtual bool HasAnyAmmo() { return true; };
	virtual bool CanBeSelected() { return true; };

	virtual const char * GetPrimaryMode() const { return "Teleport to a spot"; }; 
	virtual const char * GetSecondaryMode() const { return "Select a player"; }; 
	virtual const char * GetCostLabel() const;
private:

	void Revert();
	void Jump();
	bool Teleport(Vector vTeleport = vec3_origin);

	CNetworkVar(CHandle<CBasePlayer>, m_Target);
	CNetworkVar(bool, m_bState);
	CNetworkVar(float, m_flJumpTime);
};

#endif