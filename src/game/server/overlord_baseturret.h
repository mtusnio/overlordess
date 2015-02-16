//==============Overlord mod=====================
//	Base variables for the turrets, has basic 
//	rotation functions
//===============================================

#ifndef H_OV_BASETURRET
#define H_OV_BASETURRET

#ifdef _WIN32
#pragma once
#endif


#define SF_RELATIVE_YAW   512
#define SF_RELATIVE_PITCH 1024

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

#define PITCH_AXIS (MovesOnRelativePitch() ? CreateAxis(Vector(0, 1, 0)) : Vector(0, 1, 0))
#define YAW_AXIS   (MovesOnRelativeYaw() ? CreateAxis(Vector(0, 0, 1)) : Vector(0, 0, 1))

#define CREATE_TURRET() CreateBaseTurret(this)

class COverlordBarrel;

class COverlordBaseTurret
{
public:
	DECLARE_CLASS_NOBASE(COverlordBaseTurret);

	COverlordBaseTurret() { m_pRotate = NULL; };
	virtual ~COverlordBaseTurret() { };

	virtual void ResetTurret();

	virtual float GetYawSpeed() const { return GetMoveSpeed(); };
	virtual float GetPitchSpeed() const { return GetMoveSpeed(); };

	float		  GetMoveSpeed() const { return m_flMoveSpeed; };

	int			  GetMaxYaw() const { return m_iYaw; };
	int			  GetMaxPitch() const { return m_iPitch; };
	bool		  MovesOnRelativePitch() const { return m_pOwner->HasSpawnFlags(SF_RELATIVE_PITCH); };
	bool		  MovesOnRelativeYaw() const   { return m_pOwner->HasSpawnFlags(SF_RELATIVE_YAW); };

	bool		 IsWithinBounds(const QAngle & angle) const;

	inline Vector GetShootVector() const 
	{
		if(m_flDist == 0.0f)
			return m_pRotate->GetLocalOrigin();

		// Add to origin
		Vector add;
		AngleVectors(m_pRotate->GetLocalAngles(), &add);
		VectorNormalize(add);

		add *= m_flDist;

		return (m_pRotate->GetLocalOrigin() + add);
	}
protected:
	inline void CreateBaseTurret(CBaseEntity * pOwner);

	virtual void Fire() { };
	virtual void ResolveNames();
	
	virtual void RotateLeft();
	virtual void RotateRight();

	virtual void MoveUp();
	virtual void MoveDown();

	float m_flMoveSpeed;

	float m_iPitch;
	float m_iYaw;
	float m_flDist;
	
	char * m_iszBarrelName[MAX_BARRELS];
	char * m_iszAngleParent;

	CHandle<COverlordBarrel> m_pBarrel[MAX_BARRELS];

	bool m_bRelYaw;
	bool m_bRelPitch;

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

	// Rotate around this entity, either our angle parent or this entity
	EHANDLE m_pRotate;

	// Entity inheriting this one
	EHANDLE m_pOwner;

	QAngle m_aMaximum;
	QAngle m_aMinimum;
	QAngle m_aInit;

};

inline void COverlordBaseTurret::CreateBaseTurret(CBaseEntity * pOwner)
{
	ResolveNames();

	if(!m_pRotate)
		m_pRotate = pOwner;

	m_pOwner = pOwner;
	
	m_aInit = m_pRotate->GetLocalAngles();
	CreateBoundries();
}

inline void COverlordBaseTurret::FinalizeTheAngle(QAngle & angle)
{

}

inline void COverlordBaseTurret::RotateTurret(MoveType_t move, float speed, Vector axis)
{
	if(speed == 0)
		return;

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
		m_pRotate->SetAbsAngles(angle);
	else if((move == BASETURRET_UP || move == BASETURRET_DOWN) && (GetMaxPitch() >= 360 || GetMaxPitch() < 0))
		m_pRotate->SetAbsAngles(angle);
	else if(IsWithinBounds(angle))
		m_pRotate->SetAbsAngles(angle);
}

inline Vector COverlordBaseTurret::CreateAxis(const Vector & axis) const
{
	// Rotation axis
	Vector rotaxis;

	// Adjust it to the local space of the camera
	VectorIRotate(axis, m_pRotate->EntityToWorldTransform(), rotaxis);
	
	//m_pOwner->EntityToWorldSpace(axis, &rotaxis);
	//rotaxis -= m_pOwner->GetAbsOrigin();

	//VectorNormalize(rotaxis);

	return rotaxis;
}


#endif