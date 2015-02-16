//==============Overlord mod=====================
// Interface for weapon selection
// panels
//===============================================

#ifndef C_H_OV_WEAPONSELECTION
#define C_H_OV_WEAPONSELECTION

class IWeaponSelection
{
public:
	virtual void HandleCommand(const char * cmd) = 0;

	static IWeaponSelection * GetCurrentSelection();
};




#endif