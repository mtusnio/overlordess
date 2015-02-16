//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef H_FRAG_GRENADE
#define H_FRAG_GRENADE

#ifdef CLIENT_DLL
#define CWeaponFrag C_WeaponFrag
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

class CBaseGrenade;

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFrag: public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponFrag, CBaseHL2MPCombatWeapon );
public:

	typedef CBaseGrenade * (*GrenadeCreate)( const Vector &, const QAngle &, const Vector &, const AngularImpulse &, CBaseEntity *, float, bool);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponFrag();
	virtual ~CWeaponFrag() { };

	virtual void	Precache( void );
	virtual void	PrimaryAttack( void );
	virtual void	SecondaryAttack( void );
	virtual void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	virtual void	ItemPostFrame( void );

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	
	virtual bool	Reload( void );

#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	virtual void	ThrowGrenade( CBasePlayer *pPlayer );
	bool	IsPrimed( bool ) { return ( m_AttackPaused != 0 );	}

	virtual bool IsClassWeapon() const { return false; };
#ifndef CLIENT_DLL
protected:

	// Declare in constructor
	GrenadeCreate m_GrenadeCreate;
#endif
private:

	virtual void	RollGrenade( CBasePlayer *pPlayer );
	virtual void	LobGrenade( CBasePlayer *pPlayer );
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	virtual void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CNetworkVar( bool,	m_bRedraw );	//Draw the weapon again after throwing a grenade
	
	CNetworkVar( int,	m_AttackPaused );
	CNetworkVar( bool,	m_fDrawbackFinished );

	CWeaponFrag( const CWeaponFrag & );

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
};


#endif