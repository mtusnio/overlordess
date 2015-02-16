//==============Overlord mod=====================
//	Dynamic traps baseclass
//===============================================


#ifndef H_OV_DYNAMICTRAPS
#define H_OV_DYNAMICTRAPS

#ifndef CLIENT_DLL
#include "iscorer.h"
#else
#include "gameeventlistener.h"
#endif

#ifdef CLIENT_DLL
#define COverlordTrap C_OverlordTrap
#endif

#define NUM_OF_TRAPS 16
#define TARGET_BEAM "sprites/selectionbeam"
#define EXPLOSION_PARTICLE "ExplosionSmoke"

#define LINK_TRAP_TO_ENTITY(localName, className, buttonName, trapCost, trapDuration, description, category) LINK_ENTITY_TO_CLASS(localName, className); \
	class C##localName##FooTrap																								   \
	{																														   \
	public:																											           \
		C##localName##FooTrap()																								   \
		{																													   \
			if(COverlordTrap::STrapRecord::usedIndex < NUM_OF_TRAPS)														   \
			{																												   \
			COverlordTrap::GetWriteableRecord(COverlordTrap::STrapRecord::usedIndex).SetRecord(#buttonName, #localName, #className, trapCost, trapDuration, ##description##, ##category##); \
				COverlordTrap::STrapRecord::usedIndex++;																	   \
			}																												   \
			else																											   \
			{																												   \
				Warning("No trap added to records, traps exceed the NUM_OF_TRAPS variable\n");								   \
			}																												   \
		}																													   \
	};																														   \
	C##localName##FooTrap g_##localName##TrapFactory;


extern ConVar eo_trapsneverdecay;
extern ConVar eo_trapsdistance;
extern ConVar eo_traps_initial_dormant_range;
extern ConVar eo_traps_repair_delay;
extern ConVar eo_traps_remark_time;

//#ifdef CLIENT_DLL
struct error_t
{
	error_t() { canBuild = true; Q_strncpy(desc, "\0", ARRAYSIZE(desc)); };
	error_t(bool bBuild, const char * error)
	{
		canBuild = bBuild;
		Q_strncpy(desc, error, ARRAYSIZE(desc));
	}
	bool canBuild;
	char desc[256];
};

#ifdef CLIENT_DLL
class CNewParticleEffect;
#else
struct LagRecordTrap
{
public:
	LagRecordTrap()
	{
		m_fFlags = 0;
		m_vecOrigin.Init();
		m_vecAngles.Init();
		m_vecMins.Init();
		m_vecMaxs.Init();
		m_flSimulationTime = -1;
	/*	m_masterSequence = 0;
		m_masterCycle = 0;*/
	}

	LagRecordTrap( const LagRecordTrap& src )
	{
		m_fFlags = src.m_fFlags;
		m_vecOrigin = src.m_vecOrigin;
		m_vecAngles = src.m_vecAngles;
		m_vecMins = src.m_vecMins;
		m_vecMaxs = src.m_vecMaxs;
		m_flSimulationTime = src.m_flSimulationTime;
		/*for( int layerIndex = 0; layerIndex < MAX_LAYER_RECORDS; ++layerIndex )
		{
			m_layerRecords[layerIndex] = src.m_layerRecords[layerIndex];
		}
		m_masterSequence = src.m_masterSequence;
		m_masterCycle = src.m_masterCycle;*/
	}

	// Did player die this frame
	int						m_fFlags;

	// Player position, orientation and bbox
	Vector					m_vecOrigin;
	QAngle					m_vecAngles;
	Vector					m_vecMins;
	Vector					m_vecMaxs;

	float					m_flSimulationTime;	
	
	// Player animation details, so we can get the legs in the right spot.
	/*LayerRecord				m_layerRecords[MAX_LAYER_RECORDS];
	int						m_masterSequence;
	float					m_masterCycle;*/
};
#endif
//#endif


class CBeam;

#ifndef CLIENT_DLL
class COverlordTrap : public CBaseAnimating, public IScorer
#else
class COverlordTrap : public C_BaseAnimating, public CGameEventListener
#endif
{
public:
	DECLARE_CLASS(COverlordTrap, CBaseAnimating);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	// Performs fast conversion, better than dynamic_cast
	static COverlordTrap * EntityToTrap(CBaseEntity * pEnt);

	COverlordTrap();
	virtual ~COverlordTrap();

	virtual void Precache(); 

#ifndef CLIENT_DLL
	// Always send, needed especially for the Ov
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}
#endif

	virtual void Spawn();

	int GetTrapCost(void) const;

	float GetTrapDefaultLifetime(void) const;
	float GetTrapLifetime(void) const;
	
	const char * GetTrapName(void) const;

	float GetTrapSpawnTime(void) const { return m_flSpawnTime; };
	const char * GetTrapModel() const { return m_szTrapModel; };

	virtual void DecayTrap();
	virtual void InitializeTrap() { };

	virtual error_t CanBuild();

	virtual float GetRemainingLifetime() const { return ((GetTrapSpawnTime() + GetTrapLifetime()) - gpGlobals->curtime); };
	
	//==================================================================================================
	// Trap settings
	virtual bool  IsTrapDormant() const { return m_bIsDormant.Get(); };
	virtual bool  IsInInitialDormant() const { return m_bInitialDormant.Get(); };
	virtual float GetInitialDormantRange() const { return eo_traps_initial_dormant_range.GetFloat(); };
	virtual bool  UseLOSForInitialDormant() const { return true; };
	virtual bool  Use2DForInitialDormant() const { return false; };
	virtual bool  CanBeDormant() const { return true; };
	virtual float  GetNormalMultiplier() const { return 10; };
	
	// Arc beams
	virtual float  GetArcDegrees() const { return 0.0f; };
	virtual float  GetPitchDeegres() const { return 0.0f; };

	// Radius methods
	virtual Vector GetRadiusOrigin() { return GetEmitterPosition(); };
	virtual float GetRadiusRange() { return 0.0f; };

	// Red beam's range, nothing more.
	virtual float  BeamRange() const { return 0.0f; };

	virtual bool  CanRestoreDuration() const { return true; };
	virtual bool  CanRestoreHealth() const { return true; };

	virtual bool  ShouldGib() const { return true; };

#ifndef CLIENT_DLL
	virtual bool		ShouldUseDefenses() const { return m_bUseDefenses && (m_TrapDefenseRef <= 0); };

	virtual bool		CanDisableTrap() const { return true; };

	virtual bool		CanMarkTrap() const { return !IsTrapDormant() && IsTrapEnabled(); };

	virtual bool		ShouldFireZaps() const { return m_TrapDefenseRef <= 0; };
#endif
	//==================================================================================================

	virtual void  SetTrapDormant(bool bDormant);
	virtual float GetDormantDuration() const { return gpGlobals->curtime - m_flSetDormant.Get(); };
	virtual float GetDormantTime() const { return m_flSetDormant.Get(); };
	virtual int	  GetTrapIndex() const;

	virtual int	  GetHealth() const { return m_iHealth; };
	virtual int	  GetMaxHealth() const { return m_iMaxHealth; };

	virtual bool IsDecaying() const { return m_bDecaying; };

	virtual QAngle  NormalToAngle(const Vector & normal) const
	{
		QAngle angle;
		VectorAngles(normal, angle);
		return angle;
	}

	virtual void  ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName = NULL );

	virtual bool  IsTrapEnabled() const { return !m_bDisabled.Get(); };

	virtual void		DisableTrap(float flTime);
	virtual void		EnableTrap();
#ifdef CLIENT_DLL
	virtual void DrawBeam();
	virtual void DrawBeam(Vector start, Vector end, color32 color, float flDieTime = 0.00001f);
	void		 DrawBeam(Vector start, Vector end, Color color, float flDieTime = 0.00001f) 
	{
		color32 clr;
		clr.r = color.r();
		clr.g = color.g();
		clr.b = color.b();
		clr.a = color.a();

		DrawBeam(start, end, clr, flDieTime);
	}

	virtual void ClientThink();
	// This is the one you can override
	virtual void GhostThink() { };
	virtual void TrapThink() { }

	virtual void SpawnClientEntity() { Precache(); SetModel(m_szTrapModel);};

	// If we use some different system to give range etc. clues
	virtual bool   ShouldUseBeam() const { return true; };
	virtual Vector GetBeamOrigin() { return GetEmitterPosition(); };


	virtual Vector GetBeamVector() const;
	bool			IsGhost() { return !IsServerEntity() && !m_bOverrideGhost; };
	void			SetGhost(bool bGhost) { m_bOverrideGhost = !bGhost; };
	// NULL for no hint
	virtual const char *    GetHint() { return NULL; };

	//virtual bool  ShouldUseLocalRotationAxes() const { return false; };

	virtual void ReceiveMessage(int classID, bf_read &msg);
	virtual void FireGameEvent(IGameEvent * event);

	// Trap panel variables
	virtual float GetPanelDistanceMultiplier() const { return 1.0f; };
	virtual float GetPanelZDistance() const { return 0.0f; };
	virtual float GetPanelYDistance() const { return 0.0f; };
	virtual QAngle GetPanelAngle() const;

	void		   DoGlowBlink(Color clr, float flBlinkTime = 0.09f);
#else
	virtual void Think();

	virtual int  ObjectCaps();

	// Trap run fuction
	virtual void RunTrap() { };
	virtual void SetNextRun(float nextRun) { m_flNextRun = nextRun; };
	virtual float GetNextRun() const { return m_flNextRun; };

	// Make the trap disappear slowly
	virtual bool ShouldDecay() const;
	virtual void DecayThink();

	float		 GetLastRun() { return GetLastThink(); };

	// Give credit where it's due :)
	virtual CBasePlayer * GetScorer();
	virtual CBasePlayer * GetAssistant() { return NULL; };

	void SetExplodable(float magnitude, float radius, float force = 0.0f, bool bParticle = false)
	{
		m_bExploding = true;
		m_flExplosionMagnitude = magnitude;
		m_flExplosionRadius = radius;
		m_flExplosionForce = force;
		m_bEmitParticles = bParticle;
	}
	void SetExplodable(bool bExplode)
	{
		m_bExploding = bExplode;
	}
	

	// This function frees the trap from any bounds with other entities,
	// used when you want to make it move on its own
	virtual void		ReleaseTrap();

	virtual void		Event_Killed(const CTakeDamageInfo &info);
	virtual void		UpdateOnRemove();
	virtual int			OnTakeDamage(const CTakeDamageInfo &info);

	virtual void		RestoreDuration(bool bPay = true);
	virtual void		RestoreHealth(bool bPay = true);



	virtual void		MarkTrap(float flLength);
	
	virtual bool		IsTrapMarked() const { return m_flMarkEnd != 0.0f; };

	int					GetKills() const { return m_iKills; };
	void				SetKills(int kills) { m_iKills = kills; };
	void				IncrementKills() { m_iKills++; };

	/* Classes support */
	virtual int			GetPoints(PlayerClass_t playerClass) const;

	// Points to give when someone destroys this trap;
	virtual int			GetPointsOnDestroyed() const { return 0; };;

	// Hacker
	void				AddTrapDefenseRef() { m_TrapDefenseRef++; };
	void				RemoveTrapDefenseRef() { m_TrapDefenseRef--; };
	int					GetTrapDefenseRef() const { return m_TrapDefenseRef; };
	CBasePlayer *       GetLastHacker() const { return m_hHacker; };
	void				SetLastHacker(CBasePlayer * pPlayer) { m_hHacker = pPlayer; };

	// Assault
	void				SetLastTimeMarked(float flTime) { m_flLastTimeMarked = flTime; };
	float				GetRemarkTime() const { return (m_flLastTimeMarked == 0.0f) ? 0.0f : (m_flLastTimeMarked + eo_traps_remark_time.GetFloat()); };
#endif
	virtual Vector GetEmitterPosition();

	error_t CreateError(const char * message)
	{
		error_t error;
		error.canBuild = false;
		Q_strncpy(error.desc, message, ARRAYSIZE(error.desc));

		return error;
	}

private:
	CHandle<CBasePlayer> m_hHacker;
	float m_flLastTimeMarked;
	int m_iKills;
	

protected:
	// If center is null will use worldspacecenter
#ifdef CLIENT_DLL
	virtual void DrawRadius(int radius, int width, byte r, byte g, byte b, byte a, const Vector * center = NULL)
	{
		color32 c;
		c.r = r;
		c.g = g;
		c.b = b;
		c.a = a;
		DrawRadius(2*radius, width, c, center);
	}
	virtual void DrawRadius(int radius, int width, color32 color, const Vector * center = NULL); 
	virtual void DrawRadius() { DrawRadius(GetRadiusRange(), 9, 255, 255, 255, 255); };

#endif
	inline QAngle AlignToTop(const Vector & normal) const
	{
		QAngle normalAngle;
		VectorAngles(normal, normalAngle);

		Vector forw, right, up;
		AngleVectors(normalAngle, &forw, &right, &up);

		QAngle final;
		VectorAngles(up, forw, final);

		return final;
	}
	// Designed for combining tracehull and traceline
	//void		 SmartTraceHull(const Vector & start, const Vector & end, const Vector & hulMin, const Vector & hulMax, int mask, 
	//								const IHandleEntity * ignore, int collisiongroup, trace_t & tr) const;

	void		 SetDestroyable(int health);

	bool IsPlayerInRadius(float radius, bool bLineOfSight = true, bool bTwoDimensions = false);

	char m_szTrapModel[128];

#ifndef CLIENT_DLL
	bool m_bUseDefenses;
#endif
private:
#ifndef CLIENT_DLL
	void SelectNextZap();
	void FireZap(CBasePlayer * pPlayer);
#endif
	CNetworkVar(float, m_flSpawnTime);
	// The amount of time you want to add to the lifetime
	CNetworkVar(float, m_flLifeAdd);
	// Last time it went dormant
	CNetworkVar(float, m_flSetDormant);

	CNetworkVar(bool, m_bIsDormant);
	CNetworkVar(bool, m_bInitialDormant);
	CNetworkVar(bool, m_bEmitParticles);
	CNetworkVar(bool, m_bDecaying);
	CNetworkVar(bool, m_bDisabled);
#ifdef CLIENT_DLL
	float m_flBlinkDisable;
	int m_BeamTexture;
	// This beam allows the overlord to see where the trap points
	IMaterial * m_BeamMat;

	// Max health!
	int m_iMaxHealth;
	int m_iHealth;

	bool m_bOverrideGhost;

	CNewParticleEffect * m_DisableEffect;
#else
	float m_flTimeEnable;
	bool m_bExploding;
	bool m_bHasParent;
	bool m_HandledInitialDormant;

	float m_flExplosionRadius;
	float m_flExplosionMagnitude;
	float m_flExplosionForce;

	float m_flNextRun;

	bool m_EventSent;

	int m_NextZap;
	float m_flZapDelay;
	float m_flZapStart;
	Vector m_vZapPoint;

	float m_flMarkEnd;
	int m_TrapDefenseRef;
#endif


//=========================
// Static
//=========================
public:
	struct STrapRecord
	{
		STrapRecord() { SetRecord("", "", "", 0, 0, "", ""); };
		STrapRecord(const char * rhsButtonName, const char * rhsEntName, const char * rhsClassName, int rhsTrapCost, int rhsTrapDuration, const char * rhsDescription,
			const char * rhsCategory)
		{
			SetRecord(rhsButtonName, rhsEntName, rhsClassName, rhsTrapCost, rhsTrapDuration, rhsDescription, rhsCategory);
		};

		void SetRecord(const char * rhsButtonName, const char * rhsEntName, const char * rhsClassName, int rhsTrapCost, int rhsTrapDuration, const char * rhsDescription, 
			const char * rhsCategory)
		{ 
			Q_strncpy(buttonName, rhsButtonName, ARRAYSIZE(buttonName));
			Q_strncpy(entName, rhsEntName, ARRAYSIZE(entName));
			Q_strncpy(className, rhsClassName, ARRAYSIZE(className)); 
			Q_strncpy(description, rhsDescription, ARRAYSIZE(description));
			Q_strncpy(category, rhsCategory, ARRAYSIZE(category));

			trapCost = rhsTrapCost;
			trapDuration = rhsTrapDuration;
		};
		
		char className[48];
		char buttonName[48];
		char entName[48];
		char description[512];
		char category[32];

		int  trapCost;
		int  trapDuration;
		// Says how many indexes have been used, as they are used in order, we can add records
		// without having to supply it with index
		static int usedIndex;
	};
	static const STrapRecord & GetRecord(int num) { return sTrapRecord[num]; };
	static STrapRecord & GetWriteableRecord(int num) { return sTrapRecord[num]; };

	
private:
	static STrapRecord sTrapRecord[NUM_OF_TRAPS];


// Lag compensation stuff!
#ifndef CLIENT_DLL
public:
	CUtlFixedLinkedList<LagRecordTrap> * GetLagTrack() { return m_LagTrack; }
	LagRecordTrap*	GetLagRestoreData() { if ( m_RestoreData != NULL ) return m_RestoreData; else return (m_RestoreData = new LagRecordTrap()); }
	LagRecordTrap*	GetLagChangeData() { if ( m_ChangeData != NULL ) return m_ChangeData; else return (m_ChangeData = new LagRecordTrap()); }
	void		SetLagRestoreData(LagRecordTrap* l) { if ( m_RestoreData != NULL ) delete m_RestoreData; m_RestoreData = l; }
	void		SetLagChangeData(LagRecordTrap* l) { if ( m_ChangeData != NULL ) delete m_ChangeData; m_ChangeData = l; }
	void		FlagForLagCompensation( bool tempValue ) { m_bFlaggedForLagCompensation = tempValue; }
	bool		IsLagFlagged() { return m_bFlaggedForLagCompensation; }
 
private:
	CUtlFixedLinkedList<LagRecordTrap>* m_LagTrack;
	LagRecordTrap*	m_RestoreData;
	LagRecordTrap*	m_ChangeData;
	bool		m_bFlaggedForLagCompensation;
#endif
};

// Trace filter
class CTrapFilter : public CTraceFilterSimple
{
public:
	CTrapFilter( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pEntity, int contentsMask )
	{
		if(COverlordTrap::EntityToTrap(static_cast<CBaseEntity*>(pEntity)))
			return false;

		return CTraceFilterSimple::ShouldHitEntity(pEntity, contentsMask);
	};
};

#endif