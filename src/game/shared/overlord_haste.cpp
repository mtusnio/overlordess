//==============Overlord mod=====================
//	Overlord's weapon, speeds him up
//	
//===============================================

#include "cbase.h"
#include "weapon_hl2mpbasebasebludgeon.h"
#include "materialsystem/imaterialsystem.h"
#ifndef CLIENT_DLL
#include "hl2mp_gamerules.h"
#else
#include "iviewrender.h"
#include "ClientEffectPrecacheSystem.h"
#include "view.h"
#endif

#ifdef CLIENT_DLL
#define COverlordHaste C_OverlordHaste
#endif

#ifndef CLIENT_DLL
extern ConVar hl2_normspeed;
#else
#define OVERLAY_PATH "models/effects/portalfunnel_sheet.vmt"
#define SCALE_DIVISION 1.5f
#endif

ConVar eo_haste_speed("eo_haste_speed", "600", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_haste_damage("eo_haste_damage", "15", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_haste_default_damage("eo_haste_default_damage", "5", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_haste_drain("eo_haste_drain", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar eo_haste_damage_scale("eo_haste_damage_scale", "0.6", FCVAR_REPLICATED | FCVAR_CHEAT);

#define HASTE_SPEED eo_haste_speed.GetFloat()
#define HASTE_DAMAGE eo_haste_damage.GetFloat()
#define HASTE_DEFAULT_DAMAGE eo_haste_default_damage.GetFloat()
#define HASTE_DISABLE_SOUND "ambient.whoosh_large_incoming1"
#define HASTE_ENABLE_SOUND "ambient.whoosh_large_incoming4"
#define HASTE_POWER_DRAIN eo_haste_drain.GetInt()

#define HASTE_TRAIL_PATH "sprites/heatwave"

#ifdef CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectHaste)
CLIENTEFFECT_MATERIAL(HASTE_TRAIL_PATH)
CLIENTEFFECT_REGISTER_END()
#endif

// We do not inherit from CBasePsionic, as we have no
// base class for Overlord's weapon, this is not needed
class COverlordHaste : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS(COverlordHaste, CBaseHL2MPBludgeonWeapon);
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();
	DECLARE_NETWORKCLASS(); 

	COverlordHaste();
	virtual ~COverlordHaste();

	virtual void Precache();
#ifndef CLIENT_DLL
	virtual void ItemPostFrame();
#else
	virtual int  DrawModel(int flags);
#endif

	virtual void SecondaryAttack();

	virtual void Drop(const Vector &vecVelocity);
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
#ifdef CLIENT_DLL
	
#endif

	bool IsHasted() const { return m_bHasted.Get(); };

	virtual	float	GetFireRate( void )								{	return	0.45f;	}
	virtual float	GetRange( void )								{	return IsHasted() ? 64.0f : 52.0f;	}
	virtual	float	GetDamageForActivity( Activity hitActivity );

	virtual bool IsOverlordWeapon() { return true; };

	virtual const char * GetPrimaryMode() const { return "Attack"; }; 
	virtual const char * GetSecondaryMode() const { return "Enable haste"; }; 
	virtual const char * GetCostLabel() const { return "0/0"; };
private:
	void SwitchMode();
	void Haste(const bool bHaste);

#ifndef CLIENT_DLL
	float CalculateHasteDamage() const;
#endif

#ifdef CLIENT_DLL
	void DrawTrail() const;
	void DrawOverlay(bool bDraw) const;
	void DrawHasteHalo(IMaterial * pMaterial, const Vector & source, float scale, const float * color) const;

#endif

	CNetworkVar(bool, m_bHasted);
	
#ifndef CLIENT_DLL
	int m_TimesHurt;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(OverlordHaste, DT_OverlordHaste)

BEGIN_NETWORK_TABLE(COverlordHaste, DT_OverlordHaste)
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bHasted ) ),
#else
	SendPropBool( SENDINFO( m_bHasted ) ),
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(COverlordHaste)

	DEFINE_PRED_FIELD(m_bHasted, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),

END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_haste, COverlordHaste);
PRECACHE_WEAPON_REGISTER(weapon_haste);

acttable_t COverlordHaste::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};


IMPLEMENT_ACTTABLE(COverlordHaste);

COverlordHaste::COverlordHaste()
{
#ifndef CLIENT_DLL
	m_TimesHurt = 0;
#endif
	m_bHasted = false;
}

COverlordHaste::~COverlordHaste()
{
	Haste(false);
#ifdef CLIENT_DLL
	DrawOverlay(false);
#endif
}

void COverlordHaste::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound(HASTE_ENABLE_SOUND);
	PrecacheScriptSound(HASTE_DISABLE_SOUND);
}
#ifndef CLIENT_DLL
void COverlordHaste::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	if(m_bHasted && GetOwner() && GetOwner()->GetDamageCount() > m_TimesHurt)
	{
		m_TimesHurt = GetOwner()->GetDamageCount();
		float damage = CalculateHasteDamage();
		GetOwner()->TakeHealth(-damage, DMG_GENERIC);
	}

	// If we are in camera, unhaste
	if(GetOverlordData()->IsInVehicle())
		Haste(false);
}
#endif	

#ifdef CLIENT_DLL
// Draws the screen overlay
void COverlordHaste::DrawOverlay(bool bDraw) const
{
	if(bDraw)
	{
		IMaterial *pMaterial = materials->FindMaterial(OVERLAY_PATH, TEXTURE_GROUP_CLIENT_EFFECTS, true);
		view->SetScreenOverlayMaterial(pMaterial);
	}
	else
	{
		view->SetScreenOverlayMaterial(NULL);
	}
}

// Draws the trail behind hasted overlord
void COverlordHaste::DrawTrail() const
{
	IMaterial * pMaterial = materials->FindMaterial(HASTE_TRAIL_PATH, NULL, false);
	if(pMaterial && GetOwner())
	{
		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->Bind(pMaterial);

		Vector vMoveDir = GetOwner()->GetAbsVelocity();
		if(vMoveDir.Length() <= 1.0f)
			return;

		VectorNormalize(vMoveDir);
		vMoveDir.Negate();

		float alpha;
		float color[3];
		
		for ( int i = 0; i < 20; i++ )
		{
			alpha = RemapValClamped( i, 3, 9, 0.25f, 0.05f );
			alpha += RandomFloat(0.0f, 7.5f);
			color[0] = color[1] = color[2] = alpha;
			for(int j = 0; j < 3; j++)
				color[j] += RandomFloat(0.0f, 30.0f);
			
			// This hack allows us to give the player an illusion
			// of blurr
			Vector pos = WorldSpaceCenter();

			pos = pos + vMoveDir * 18;
			pos.x += RandomFloat(0.0f, 10.0f);
			pos.y += RandomFloat(0.0f, 10.0f);
			pos.z += RandomFloat(0.0f, 10.0f);
			DrawHasteHalo( pMaterial, pos, 35.0f, color );
		}
	}
}

int COverlordHaste::DrawModel(int flags)
{
	if(m_bHasted)
		DrawTrail();

	return BaseClass::DrawModel(flags);
}
#endif

void COverlordHaste::SecondaryAttack()
{
	if(m_flNextSecondaryAttack > gpGlobals->curtime)
		return;

	SwitchMode();

	m_flNextSecondaryAttack = gpGlobals->curtime + 5.0f;
}

void COverlordHaste::Drop(const Vector &vecVelocity)
{
	Haste(false);
#ifndef CLIENT_DLL
	UTIL_Remove(this);
#endif
}

bool COverlordHaste::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if(IsHasted() && m_flNextSecondaryAttack > gpGlobals->curtime)
		return false;

	Haste(false);
	return BaseClass::Holster(pSwitchingTo);
}

#ifndef CLIENT_DLL
// Returns damage that should be added to the damage count when the haste
// is enabled
float COverlordHaste::CalculateHasteDamage() const
{
	const CBasePlayer * pPlayer = ToBasePlayer(GetOwner());

	if(!pPlayer)
		return 0.0f;

	float damage = pPlayer->GetLastDamageAmount();

	if(damage <= 0)
		return 0.0f;

	// Calculate the damage, uses custom damage table (does it? ;))
	damage *= eo_haste_damage_scale.GetFloat();

	return damage;
}
#endif

void COverlordHaste::SwitchMode()
{
	Haste(!m_bHasted);
}

void COverlordHaste::Haste(const bool bHaste)
{
	if(bHaste == m_bHasted)
		return;

	m_bHasted = bHaste;

	CHL2MP_Player * pPlayer = ToHL2MPPlayer(GetOwner());

	if(!pPlayer)
		return;

	if(bHaste)
	{
#ifndef CLIENT_DLL
		pPlayer->EnableSprint(false);
		m_TimesHurt = pPlayer->GetDamageCount();
		pPlayer->AddSlowdown(-HASTE_SPEED, -1.0f, this);
#else
		DrawOverlay(true);
#endif
		EmitSound(HASTE_ENABLE_SOUND);
	}
	else if (!bHaste)
	{
#ifndef CLIENT_DLL
		pPlayer->EnableSprint(true);
		m_TimesHurt = 0;
		pPlayer->RemoveSlowdown(this);
#else
		DrawOverlay(false);
#endif
		EmitSound(HASTE_DISABLE_SOUND);
	}
}

float COverlordHaste::GetDamageForActivity( Activity hitActivity )
{
	if(m_bHasted)
		return HASTE_DAMAGE;
	else
		return HASTE_DEFAULT_DAMAGE;
}

#ifdef CLIENT_DLL
// Taken from DrawHalo, changed to draw a rectangle
void COverlordHaste::DrawHasteHalo(IMaterial * pMaterial, const Vector & source, float scale, const float * color) const
{
	if(!GetOwner())
		return;

	CBasePlayer * pPlayer = ToBasePlayer(GetOwner());
	if(!pPlayer)
		return;
	
	Vector forw, right, up;
	forw = pPlayer->GetAbsVelocity();
	
	pPlayer->EyeVectors(NULL, &right, &up);

	Vector		point, screen;
	
	CMatRenderContextPtr pRenderContext( materials );
	IMesh* pMesh = pRenderContext->GetDynamicMesh( );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	// Transform source into screen space
	ScreenTransform( source, screen );

	meshBuilder.Color3fv (color);
	meshBuilder.TexCoord2f (0.6f, 0, 1);
	VectorMA (source, -scale, CurrentViewUp(), point);
	VectorMA (point, -scale/SCALE_DIVISION, CurrentViewRight(), point);
	meshBuilder.Position3fv (point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv (color);
	meshBuilder.TexCoord2f (0.7f, 0, 0);
	VectorMA (source, scale, CurrentViewUp(), point);
	VectorMA (point, -scale/SCALE_DIVISION, CurrentViewRight(), point);
	meshBuilder.Position3fv (point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv (color);
	meshBuilder.TexCoord2f (0.5f, 0.6f, 0);
	VectorMA (source, scale, CurrentViewUp(), point);
	VectorMA (point, scale/SCALE_DIVISION, CurrentViewRight(), point);
	meshBuilder.Position3fv (point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv (color);
	meshBuilder.TexCoord2f (0.75f, 0.6f, 1);
	VectorMA (source, -scale, CurrentViewUp(), point);
	VectorMA (point, scale/SCALE_DIVISION, CurrentViewRight(), point);
	meshBuilder.Position3fv (point.Base());
	meshBuilder.AdvanceVertex();
	
	meshBuilder.End();
	pMesh->Draw();
}
#endif