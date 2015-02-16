//==============Overlord mod=====================
//	Power generator
//	
//===============================================

#ifndef H_OV_GENERATOR
#define H_OV_GENERATOR

#include "baseentity.h"

#ifdef _WIN32
#pragma once
#endif

class COverlordGenerator : public CBaseEntity
{
public:
	DECLARE_CLASS(COverlordGenerator, CBaseEntity);

	COverlordGenerator() { };
	virtual ~COverlordGenerator() { };
};



#endif