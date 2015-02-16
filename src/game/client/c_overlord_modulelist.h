//==============Overlord mod=====================
//	Client-side implementation of the module
//	list
//===============================================

#ifndef C_H_OV_MODULELIST
#define C_H_OV_MODULELIST

class C_OverlordBaseModule;

class C_OverlordModuleList : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_OverlordModuleList, C_BaseEntity);
	DECLARE_CLIENTCLASS()

	C_OverlordModuleList() { };
	virtual ~C_OverlordModuleList() { };

	C_OverlordBaseModule * GetModule(int i) const { return m_Module[i]; };

	int GetMaxModules() const { return MAX_BUTTONS; };

	// Returns its index, -1 if isn't
	int IsEntityOnList(const C_BaseEntity * pEnt) const;
	
	void AddGlows();
	void UpdateGlows();
	void RemoveGlows();
private:
	// We need to put it manually, for some reason 
	// MAX_BUTTONS doesn't show up as defined
	CHandle<C_OverlordBaseModule> m_Module[MAX_BUTTONS];
};



#endif