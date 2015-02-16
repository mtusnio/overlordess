//==============Overlord mod=====================
//	Displays charge, ammo etc. for the Overlord
//	and rebels
//===============================================

#ifndef H_OV_HINT
#define H_OV_HINT

#ifdef _WIN32
#pragma once
#endif

#include "hud.h"
#include "vgui_controls/Label.h"
#include "hudelement.h"

#define DISPLAY_SIZE LABEL_SIZE

/*#define MODE_POS_X 10
#define MODE_POS_Y 4

#define TARG_POS_X 10
#define TARG_POS_Y 20*/

#define TEXT_POS_X 8
#define TEXT_POS_Y 4

#define WIDTH 150
#define HINT_LENGTH 256

// Whether we should show weapon hints
// #define SHOW_WEAPON_HINTS

namespace vgui
{
	class RichText;
}

// Our list of possible hints
enum EventType_t
{
	UNKNOWN = -1,
	DEFAULT, // Default also means first!
	CUSTOM, // Custom also means last!
};

class COverlordHint : public vgui::Panel, public CHudElement
{
public:
	DECLARE_CLASS(COverlordHint, vgui::Panel);
	
	COverlordHint(const char * pElementName);
	virtual ~COverlordHint();

	virtual void Init();
	virtual void Reset();
	virtual bool ShouldDraw();

	virtual void OnThink();

	virtual void StartListening();

	virtual void FireGameEvent(IGameEvent *event);

protected:

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
#ifdef SHOW_WEAPON_HINTS
	bool WasWeaponFound() const { return m_bWeaponFound; };
#endif
	// Contains current displayed event as well as next one, along with time of
	// the next update
	struct HintEvent
	{
		EventType_t current;
		EventType_t next;

		float nextUpdate;

		void JumpForward() { current = next; next = DEFAULT; };
	};

	virtual void UpdateDisplay(EventType_t type = DEFAULT, bool bUpdateNow = false);

	//char m_Display[HINT_LENGTH];
	HintEvent m_Hint;
	
	// Custom display, specified through FireGameEvent
	char m_Custom[HINT_LENGTH];

#ifdef SHOW_WEAPON_HINTS
	bool m_bWeaponFound;
#endif

	vgui::RichText * m_RichText;
};

#endif