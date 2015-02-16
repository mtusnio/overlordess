//==============Overlord mod=====================
//	Client-side declaration of the overlord's
//	console
//===============================================

#ifndef C_OVCONSOLE_H
#define C_OVCONSOLE_H

#ifdef _WIN32
#pragma once
#endif

//#include "c_baseentity.h"

class C_OverlordConsole : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordConsole, C_BaseEntity);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_OverlordConsole();
	~C_OverlordConsole();

	bool IsEnabled() const { return m_bEnabled; };
	bool ShouldDraw() const { return true; };
	const char * GetAreaName() const { return &m_iszArea[0]; };

	virtual const char *            GetTargetTextFormat() const { if(IsEnabled()) return "#Console_Enabled"; else return "#Console_Disabled"; };
	virtual const char *            GetTargetText() const { return NULL; };
private:
	char m_iszArea[MAX_AREA_NAME_SIZE];
	bool m_bEnabled;
};


#endif