//==============Overlord mod=====================
//	Sentry gun's barrel
//	
//===============================================

#include "baseentity.h"
#include "iscorer.h"

#ifndef H_OV_BARREL
#define H_OV_BARREL

#ifdef _WIN32
#pragma once
#endif

class COverlordBarrel : public CBaseEntity, public IScorer
{
public:
	DECLARE_CLASS(COverlordBarrel, CBaseEntity);
	DECLARE_DATADESC();


	COverlordBarrel();
	virtual ~COverlordBarrel();

	virtual void Spawn();

	virtual void Precache();

	virtual void Fire(const Vector &forward);
	virtual void FireToLocation(const Vector &location);

	int GetDamage() const { return m_iDamage; };
	float GetNextAttack() const { return m_flNextAttack; };

	virtual bool CanFire() const;

	// Give credit to the Ov!
	virtual CBasePlayer * GetScorer();
	virtual CBasePlayer * GetAssistant() { return NULL; };

	virtual void InputFire(inputdata_t &inputData);
protected:
	void SetNextFire(float flnext) { m_flNextAttack = flnext; };
	void UpdateNextFire() { m_flNextAttack = gpGlobals->curtime + m_flIntermission; };
private:
	virtual void DoEffects(const Vector &forward);
	float m_flNextAttack;
	float m_flIntermission;
	float m_flDistance;
	
	int	  m_iDamage;
	int	  m_iGenericDamage;

	char * m_iszSoundName;

};


#endif