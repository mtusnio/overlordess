//==============Overlord mod=====================
//	Psiemitter trap
//	
//===============================================


#include "cbase.h"
#include "overlord_psiemitter.h"

#define EMITTER_THINK 0.1f

COverlordPsiEmitter::COverlordPsiEmitter()
{
}

COverlordPsiEmitter::~COverlordPsiEmitter()
{
}

void COverlordPsiEmitter::Think()
{
	BaseClass::Think();

	if(IsActivated())
		RunModule();

	SetNextThink(gpGlobals->curtime + EMITTER_THINK);
}

void COverlordPsiEmitter::RunModule()
{
}