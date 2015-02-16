//==============Overlord mod=====================
//	Sentrygun controlled by the overlord.
//	Client-side
//===============================================

#ifndef C_H_OV_REMOTESENTRY
#define C_H_OV_REMOTESENTRY

#include "overlord_camera.h"

#define AMMO_BUTTON 0

class C_OverlordSentry : public C_OverlordCamera
{
public:
	DECLARE_CLASS(C_OverlordSentry, C_OverlordCamera);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_OverlordSentry();
	virtual ~C_OverlordSentry();

	virtual int				 DrawModel(int flags) { return 0; };

	virtual bool			 ShouldBlockControls() const { return true; };
private:
};
#endif