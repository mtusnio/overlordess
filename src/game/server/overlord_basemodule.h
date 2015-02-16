//==============Overlord mod=====================
//	Base class for all modules
//	
//===============================================

#ifndef C_OV_BASEMODULE
#define C_OV_BASEMODULE


#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

// Inheriting from CBaseAnimating lets us use the camera model

#define SF_START_MODULE_DISABLED 1
#define SF_DO_NOT_DRAW 1024

class COverlordBaseModule : public CBaseAnimating
{
public:
	DECLARE_CLASS(COverlordBaseModule, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	COverlordBaseModule();
	virtual ~COverlordBaseModule();

	virtual void Spawn();
	virtual void Activate();

	virtual void Precache();

	virtual void Think(); // Always override this, SetNextThink is omitted in this one
							  // therefore you must specify your own. 
	
	virtual void InputActivate(inputdata_t &inputData); 
	virtual void InputDeactivate(inputdata_t &inputData); 
	virtual void InputDisable(inputdata_t &inputData);
	virtual void InputEnable(inputdata_t &inputData);
		
	virtual void Deactivate();

	virtual bool  CanActivate() const;

	bool  IsActivated() const { return m_bActivated; };
	bool  IsEnabled() const { return m_bEnabled.Get(); };
	virtual bool  ShouldDeactivate() const { return ((m_flDeactTime.Get() <= gpGlobals->curtime) 
		&& IsActivated() && (GetDuration() > 0.0f)); };

	float RemainingCooldownTime() const { return ((m_flDeactTime.Get() + m_flCooldown) - gpGlobals->curtime); };
	bool  IsCoolingDown() const 
	{
		return ((m_flDeactTime.Get() != 0) && RemainingCooldownTime() > 0.0f 
			&& IsEnabled() && !IsActivated()); 
	};

	virtual bool CanBeSelected() { return (m_spawnflags & SF_DO_NOT_DRAW) != 0; };

	const char * GetModuleName() { return STRING(m_ModuleName.Get()); };
	const char * GetModuleDescription() { return STRING(m_ModuleDescription.Get()); };
	int   GetModuleCost() const { return m_iCost.Get(); };
	float GetDuration() const { return (m_flLength.Get() >= 0.0f) ? m_flLength.Get() : 0; };
	bool  ShouldNeverDeactivate() const { return m_flLength.Get() == 0.0f; };

	void SetCost(int cost) { m_iCost = cost; };

	
	virtual int UpdateTransmitState( void)
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

protected:

	virtual void  DenyActivation();
	virtual void  RunModule() { }; // A pseudo-think function for the module, runs only when 
								   // it's activated
	virtual void  FireOnActivated() { m_OnActivated.FireOutput(NULL, this); };
	virtual void  FireOnDeactivated() { m_OnDeactivated.FireOutput(NULL, this); };

	void		  SetActivated(bool bActivated) { m_bActivated = bActivated; };

private:
	CNetworkVar(string_t, m_ModuleName);
	CNetworkVar(string_t, m_ModuleDescription);
	CNetworkVar(int, m_iCost);
	CNetworkVar(float, m_flLength);
	CNetworkVar(bool, m_bEnabled);
	CNetworkVar(bool, m_bActivated);
	CNetworkVar(float, m_flCooldown);
	CNetworkVar(float, m_flDeactTime);
	CNetworkHandle(CBaseEntity, m_GlowEntity);

	const char * m_szGlowEntityName;
	//bool	m_bEnabled;
	//float	m_flLength;
	//int		m_iCost;
	//char *	m_iszModuleName;
	//bool	m_bActivated;
	//float   m_flDeactTime;
	//float	m_flCooldown;

	COutputEvent m_OnActivated;
	COutputEvent m_OnDeactivated;
};

#endif