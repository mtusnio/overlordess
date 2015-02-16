//==============Overlord mod=====================
//	Dynamic traps defense systems
//===============================================

#ifndef C_H_OV_DEFENSES
#define C_H_OV_DEFENSES

class COverlordTrap;

enum DefenseType
{
	// Keep random first!
	RANDOM = 0,
	SMOKE,
	GAS,
	VORTEX,
	PROJECTILE,
	//SPIKES,
	REVERSE_VORTEX,
	GRENADES,
	RANDOMTRAP,
	SLOWDOWN,

	MAX_TYPES,
};

// Trap is optional, required only for some defenses
void CreateTrapDefense(const Vector & pos, const QAngle & angle,  COverlordTrap * pTrap, const DefenseType type = RANDOM);

#endif
