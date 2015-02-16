//==============Overlord mod=====================
//	Client-side implementation of the base
// module
//===============================================

#ifndef C_H_OV_BASEMODULE
#define C_H_OV_BASEMODULE

#define SF_DO_NOT_DRAW 1024

class C_OverlordModuleList;

class C_OverlordBaseModule : public C_BaseAnimating 
{
public:
	DECLARE_CLASS(C_OverlordBaseModule, C_BaseAnimating);

	DECLARE_CLIENTCLASS();

	C_OverlordBaseModule() { };
	virtual ~C_OverlordBaseModule() { };

	virtual void OnDataChanged(DataUpdateType_t updateType);

	virtual int DrawModel(int flags);

	const char * GetModuleName() const { return m_ModuleName; };
	const char * GetModuleDescription() const { return m_ModuleDescription; };
	int   GetModuleCost() const { return m_iCost; };
	float GetDuration() const { return (m_flLength >= 0.0f) ? m_flLength : 0; };
	bool  IsEnabled() const { return m_bEnabled; };
	bool  IsActivated() const { return m_bActivated; };
	bool  IsCoolingDown() const { return (GetDeactivationTime() != 0) &&
		((GetCooldownLength() + GetDeactivationTime()) > gpGlobals->curtime) && IsEnabled() && !IsActivated(); };
	float GetCooldownLength() const { return m_flCooldown; };
	float GetDeactivationTime() const { return m_flDeactTime; };
	float GetRemainingCooldown() const { return IsCoolingDown() ? (GetCooldownLength() + GetDeactivationTime()) - gpGlobals->curtime : 0.0f; };
	bool  ShouldNeverDeactivate() const { return m_flLength == 0.0f; };
	virtual bool CanBeSelected() { return (m_spawnflags & SF_DO_NOT_DRAW) == 0; };

	C_BaseEntity * GetGlowEntity() const { return m_GlowEntity; };

	virtual bool   ShouldGlow() const { return true; };
	virtual Color  GetGlowColour() const { return GetIndicatorColor(); };

	Color GetIndicatorColor() const;
private:
	int m_iCost;

	float m_flLength;
	float m_flCooldown;
	float m_flDeactTime;

	bool m_bActivated;
	bool m_bEnabled;
	char m_ModuleName[BUTT_LENGTH];
	char m_ModuleDescription[DESC_LENGTH];

	EHANDLE m_GlowEntity;

	int m_spawnflags;
};


#endif