//==============Overlord mod=====================
//	Base variables for the turrets, has basic 
//	rotation functions
//===============================================

#ifndef H_OV_BASETURRET
#define H_OV_BASETURRET

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
#define COverlordBaseTurret C_OverlordBaseTurret
#endif


#define SF_WORLD_YAW   512
#define SF_WORLD_PITCH 256

#define MAX_BARRELS 8

#define DEFINE_BARRELS() 	DEFINE_KEYFIELD(m_iszBarrelName[0], FIELD_STRING, "BarrelName1"), \
	DEFINE_KEYFIELD(m_iszBarrelName[1], FIELD_STRING, "BarrelName2"), \
	DEFINE_KEYFIELD(m_iszBarrelName[2], FIELD_STRING, "BarrelName3"), \
	DEFINE_KEYFIELD(m_iszBarrelName[3], FIELD_STRING, "BarrelName4"), \
	DEFINE_KEYFIELD(m_iszBarrelName[4], FIELD_STRING, "BarrelName5"), \
	DEFINE_KEYFIELD(m_iszBarrelName[5], FIELD_STRING, "BarrelName6"), \
	DEFINE_KEYFIELD(m_iszBarrelName[6], FIELD_STRING, "BarrelName7"), \
	DEFINE_KEYFIELD(m_iszBarrelName[7], FIELD_STRING, "BarrelName8")

#define DEFINE_TURRET() 	DEFINE_KEYFIELD(m_iYaw, FIELD_FLOAT, "MaxYaw"), \
	DEFINE_KEYFIELD(m_iPitch, FIELD_FLOAT, "MaxPitch"), \
	DEFINE_KEYFIELD(m_flMoveSpeed, FIELD_FLOAT, "MoveSpeed"), \
	DEFINE_KEYFIELD(m_flDist, FIELD_FLOAT, "DistanceFromCamera")

#define PITCH_AXIS (!MovesOnRelativePitch() ? CreateAxis(Vector(0, 1, 0)) : Vector(0, 1, 0))
#define YAW_AXIS   (!MovesOnRelativeYaw() ? CreateAxis(Vector(0, 0, 1)) : Vector(0, 0, 1))

// Put this in activate function
#define CREATE_TURRET() CreateBaseTurret()

class COverlordBarrel;

class COverlordBaseTurret : public CBaseAnimating
{
public:
	DECLARE_CLASS_NOBASE(COverlordBaseTurret);
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
#ifdef CLIENT_DLL
	DECLARE_PREDICTABLE();
#endif

	COverlordBaseTurret();
	virtual ~COverlordBaseTurret() { };

	virtual void ResetTurret();

	virtual float GetYawSpeed() const { return GetMoveSpeed(); };
	virtual float GetPitchSpeed() const { return GetMoveSpeed(); };

	virtual float GetMoveSpeed() const { return m_flMoveSpeed; };

	virtual int		GetMaxYaw() const { return m_iYaw; };
	virtual int		GetMaxPitch() const { return m_iPitch; };

	virtual bool    MovesOnRelativePitch() const { return m_bRelPitch; };
	virtual bool	MovesOnRelativeYaw() const   { return m_bRelYaw; };

	virtual bool    IsWithinBounds(const QAngle & angle) const;

	inline Vector GetShootVector() const 
	{
		if(m_flDist == 0.0f)
			return GetLocalOrigin();

		// Add to origin
		Vector add;
		AngleVectors(GetLocalAngles(), &add);
		VectorNormalize(add);

		add *= m_flDist;

		return (GetLocalOrigin() + add);
	}
protected:
	inline void CreateBaseTurret();

	virtual void Fire() { };
	virtual void ResolveNames();
	
	virtual void RotateLeft();
	virtual void RotateRight();

	virtual void MoveUp();
	virtual void MoveDown();

	CNetworkVar(float, m_flMoveSpeed);

	CNetworkVar(float, m_iPitch);
	CNetworkVar(float, m_iYaw);
	CNetworkVar(float, m_flDist);
	
	// This is only server stuff, no idea what's doing here though...
#ifndef CLIENT_DLL
	char * m_iszBarrelName[MAX_BARRELS];

	CHandle<COverlordBarrel> m_pBarrel[MAX_BARRELS];
#endif

	CNetworkVar(bool, m_bRelYaw);
	CNetworkVar(bool, m_bRelPitch);

private:
	enum MoveType_t
	{
		BASETURRET_LEFT = 0,
		BASETURRET_RIGHT,
		BASETURRET_UP,
		BASETURRET_DOWN,
	};

	void		  SetMoveSpeed(float speed) { m_flMoveSpeed = speed; }; 

	inline void   RotateTurret(MoveType_t move, float speed, Vector axis);

	// Creates the maximum and minimum coordinates
	void CreateBoundries();

	// Apply any neccessary changes and set the angle
	inline void FinalizeTheAngle(QAngle & angle);
	void RotateAngle(QAngle & angle, float flAngle, const Vector & axis) const;

	// Creates a relative axis to the global one specified
	inline Vector  CreateAxis(const Vector & axis) const;

	CNetworkVar(QAngle, m_aMaximum);
	CNetworkVar(QAngle, m_aMinimum);
	CNetworkVar(QAngle, m_aInit);

};

inline void COverlordBaseTurret::CreateBaseTurret()
{
#ifndef CLIENT_DLL
	ResolveNames();
	
	m_aInit = GetLocalAngles();

	m_bRelYaw = !HasSpawnFlags(SF_WORLD_YAW);
	m_bRelPitch = !HasSpawnFlags(SF_WORLD_PITCH);

	CreateBoundries();
#endif
}

inline void COverlordBaseTurret::FinalizeTheAngle(QAngle & angle)
{

}

inline void COverlordBaseTurret::RotateTurret(MoveType_t move, float speed, Vector axis)
{
	if(speed == 0)
	{
		Warning("RotateTurret passed with speed 0\n");
		return;
	}

	QAngle angle;
	
	RotateAngle(angle, speed, axis);

	// Don't move the angle into boundries if this is right
	/*if(!((move == BASETURRET_LEFT || move == BASETURRET_RIGHT) && ((GetMaxYaw() < 0) || (GetMaxYaw() >= 360))))
	{
		MoveIntoBoundries(angle);
	}

	if(!((move == BASETURRET_UP || move == BASETURRET_DOWN) && ((GetMaxPitch() < 0) || (GetMaxPitch() >= 360))))
	{
		MoveIntoBoundries(angle);
	}*/

	if((move == BASETURRET_LEFT || move == BASETURRET_RIGHT) && (GetMaxYaw() >= 360 || GetMaxYaw() < 0))
		SetLocalAngles(angle);
	else if((move == BASETURRET_UP || move == BASETURRET_DOWN) && (GetMaxPitch() >= 360 || GetMaxPitch() < 0))
		SetLocalAngles(angle);
	else if(IsWithinBounds(angle))
		SetLocalAngles(angle);
}

inline Vector COverlordBaseTurret::CreateAxis(const Vector & axis) const
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