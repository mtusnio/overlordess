//==============Overlord mod=====================
//	Automatic turret
//	
//===============================================

#ifndef H_OV_TURRET
#define H_OV_TURRET

#ifdef _WIN32
#pragma once
#endif

#include "overlord_basemodule.h"

// Just to clarify what we are using and why
typedef bool LAST_MOVE;
//#define LAST_MOVE bool
#define LAST_MOVE_LEFT true
#define LAST_MOVE_RIGHT false

#define SF_CONCENTRATE_BARRELS 2
#define SF_ALWAYS_FIRE 4

#define MAX_BARRELS 8

class COverlordBarrel;

class COverlordTurret : public COverlordBaseModule
{
public:
	DECLARE_CLASS(COverlordTurret, COverlordBaseModule);
	DECLARE_DATADESC();

	COverlordTurret();
	~COverlordTurret();
	
	virtual void Precache();
	virtual void Spawn();
	virtual void Activate();
	virtual void Think();

	CBasePlayer * GetTarget(void) const { return m_pTarget.Get(); };

	LAST_MOVE	GetLastYawMove(void) const { return m_LastMove; };
	//virtual void InputActivate(inputdata_t &inputData);

	virtual float GetYawSpeed() const;
	virtual float GetPitchSpeed() const;

	virtual float GetMoveSpeed() const { return m_flMoveSpeed; };

	inline Vector GetShootVector() const 
	{
		if(m_flDist == 0.0f)
			return GetAbsOrigin();

		// Add to origin
		Vector add;
		AngleVectors(GetAbsAngles(), &add);
		VectorNormalize(add);

		add *= m_flDist;

		return (GetAbsOrigin() + add);
	}

	virtual bool MovesOnRelativePitch();
	virtual bool MovesOnRelativeYaw();

	virtual float GetMaxYaw() const { return m_iYaw; };
	virtual float GetMaxPitch() const { return m_iPitch; };
private:
	enum MoveType_t
	{
		BASETURRET_LEFT = 0,
		BASETURRET_RIGHT,
		BASETURRET_UP,
		BASETURRET_DOWN,
	};

	virtual void ResolveNames();

	virtual void Fire();

	virtual void Deactivate();
	virtual void RunModule();
	virtual void AlignTurret();

	virtual void RotateLeft();
	virtual void RotateRight();
	virtual void MoveUp();
	virtual void MoveDown();

	virtual void CreateBoundries();

	virtual void ResetTurret()
	{
		SetAbsAngles(m_aInit);
	}
	virtual void RotateTurret(MoveType_t move, float speed, Vector axis);
	virtual void RotateAngle(QAngle & angle, float flAngle, const Vector & axis) const;

	virtual bool IsWithinBounds(const QAngle & angle) const;

	float         GetDistance(CBasePlayer * pPlayer) const 
	{ 
		Vector pos = GetHeadPosition(pPlayer) - GetAbsOrigin();
		return pos.Length();
	}

	Vector		  GetHeadPosition(CBasePlayer * pPlayer) const
	{
		QAngle nullangle;
		Vector headPos;
		int iBone = pPlayer->LookupBone("ValveBiped.Bip01_Head1");

		if(iBone >= 0)
		{
			pPlayer->GetBonePosition(iBone, headPos, nullangle);
		}
		else
		{
			headPos = pPlayer->RealEyePosition();
		}

		return headPos;
	}
	virtual bool  CanTarget(CBasePlayer * pTarget) const;
	void		  SetTarget(CBasePlayer * target)
	{
		if(m_pTarget != target)
		{
			m_flShootDelay = 0.0f;
		}
		m_pTarget = target; 
	}; 

	virtual bool  ShouldFire(const trace_t & trace);

	inline Vector  CreateAxis(const Vector & axis) const;

	// No need for further performance loses
	FORCEINLINE void   GetTurretTargetAngles(QAngle & turret, QAngle & target) const;

	// Handles make sure that the game doesn't crash when the player quits
	CHandle<CBasePlayer> m_pTarget;
	QAngle m_aInit;

	float m_flRadius;
	float m_flAlign;
	float m_flTrackDelay;

	float m_flShootDelay;

	LAST_MOVE m_LastMove;

	// Rotation vars
	float m_flMoveSpeed;
	float m_iYaw;
	float m_iPitch;
	float m_flDist;

	QAngle m_aMaximum;
	QAngle m_aMinimum;

	// Barrels
	const char * m_iszBarrelName[MAX_BARRELS];
	CHandle<COverlordBarrel> m_pBarrel[MAX_BARRELS];
};


FORCEINLINE void COverlordTurret::GetTurretTargetAngles(QAngle & turret, QAngle & target) const
{
	QAngle nullangle;
	Vector headPos;
	GetTarget()->GetBonePosition(GetTarget()->LookupBone("ValveBiped.Bip01_Head1"), headPos, nullangle);
	Vector pos = headPos - GetAbsOrigin();

	// Trace vector
	Vector trace;

	// Forward vector
	Vector forw;
	AngleVectors(GetAbsAngles(), &forw);

	// Get the position we point to
	VectorMA(GetAbsOrigin(), (GetTarget()->GetAbsOrigin() - GetAbsOrigin()).Length(), forw, trace);

	// Move it into the (0, 0, 0) coords
	trace -= GetAbsOrigin(); 
	// Now, vectors into the angles
	VectorAngles(trace, turret);

	VectorAngles(pos, target);
}

inline Vector COverlordTurret::CreateAxis(const Vector & axis) const
{
	// Rotation axis
	Vector rotaxis;

	// Adjust it to the local space of the camera
	VectorIRotate(axis, EntityToWorldTransform(), rotaxis);
	
	//m_pOwner->EntityToWorldSpace(axis, &rotaxis);
	//rotaxis -= m_pOwner->GetAbsOrigin();

	//VectorNormalize(rotaxis);

	return rotaxis;
}




#endif

