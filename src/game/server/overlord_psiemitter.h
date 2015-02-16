//==============Overlord mod=====================
//	Psiemitter trap
//	
//===============================================

#ifndef H_OV_PSIEMITTER
#define H_OV_PSIEMITTER

#ifdef _WIN32
#pragma once
#endif

#include "overlord_basemodule.h"

class COverlordPsiEmitter : public COverlordBaseModule
{
public:
	DECLARE_CLASS(COverlordPsiEmitter, COverlordBaseModule);
	COverlordPsiEmitter();
	virtual ~COverlordPsiEmitter();

	virtual void Think();

private:
	virtual void RunModule();
};

#endif
