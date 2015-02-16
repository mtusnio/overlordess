//==============Overlord mod=====================
//	Client-side implementation of the module
//	list
//===============================================

#include "cbase.h"
#include "c_overlord_modulelist.h"
#include "c_overlord_basemodule.h"

IMPLEMENT_CLIENTCLASS_DT( C_OverlordModuleList, DT_OverlordModuleList, COverlordModuleList)

	RecvPropArray3( RECVINFO_ARRAY(m_Module),	RecvPropEHandle( RECVINFO(m_Module[0]) ) ),

END_RECV_TABLE()

int C_OverlordModuleList::IsEntityOnList(const C_BaseEntity * pEnt) const
{
	if(!pEnt)
		return -1;

	for(int i = 0; i < GetMaxModules(); i++)
	{
		if(pEnt == GetModule(i))
			return i;
	}

	return -1;
}

void C_OverlordModuleList::AddGlows()
{
	int max = GetMaxModules();
	for(int i = 0; i < max; i++)
	{
		C_OverlordBaseModule * pTrap = GetModule(i);

		if(!pTrap)
			continue;

		if(pTrap->ShouldGlow())
			pTrap->RegisterGlow(pTrap->GetGlowColour());

		CBaseEntity * pGlow = pTrap->GetGlowEntity();

		if(pGlow)
		{
			pGlow->RegisterGlow(pTrap->GetGlowColour());
		}
	}
}

void C_OverlordModuleList::UpdateGlows()
{
	int max = GetMaxModules();
	for(int i = 0; i < max; i++)
	{
		C_OverlordBaseModule * pTrap = GetModule(i);

		if(!pTrap)
			continue;
	
		C_BaseEntity * pEnt = pTrap->GetGlowEntity();

		if(pEnt && pEnt->IsGlowRegistered())
		{
			pEnt->SetGlowColor(pTrap->GetGlowColour());
		}
		if(!pTrap->ShouldGlow() || !pTrap->IsGlowRegistered())
			continue;

		pTrap->SetGlowColor(pTrap->GetGlowColour());
	}
}

void C_OverlordModuleList::RemoveGlows()
{
	int max = GetMaxModules();
	for(int i = 0; i < max; i++)
	{
		C_OverlordBaseModule * pTrap = GetModule(i);

		if(!pTrap)
			continue;

		if(pTrap->ShouldGlow())
			pTrap->DeregisterGlow();

		CBaseEntity * pGlow = pTrap->GetGlowEntity();

		if(pGlow)
		{
			pGlow->DeregisterGlow();
		}
	}
}