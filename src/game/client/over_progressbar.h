//==============Overlord mod=====================
//	Progres bar class
//===============================================

#ifndef H_OV_PROGRESSBAR
#define H_OV_PROGRESSBAR

#include <vgui_controls/Panel.h>

namespace vgui
{
	class COverlordProgressBar : public vgui::Panel
	{
	public:
		DECLARE_CLASS_SIMPLE(COverlordProgressBar, vgui::Panel);

		COverlordProgressBar(vgui::Panel * parent, const char * name);
		COverlordProgressBar(vgui::Panel * parent, const char * name, int outlineSpace, int type = CHud::HUDPB_HORIZONTAL);
		COverlordProgressBar(vgui::Panel * parent, const char * name, CHudTexture * iconFull, CHudTexture * iconEmpty, 
			int type = CHud::HUDPB_HORIZONTAL);

		virtual ~COverlordProgressBar();

		virtual void ApplySettings(KeyValues *inResourceData);

		virtual void Paint();

		virtual void SetColors(Color full, Color empty, Color outline) { SetFullColor(full); SetEmptyColor(empty); SetOutlineColor(outline); };

		virtual void SetFullColor(Color clr) { m_FullColor = clr; }
		virtual void SetEmptyColor(Color clr) { m_EmptyColor = clr; };
		virtual void SetOutlineColor(Color clr) { m_OutlineColor = clr; };

		virtual Color GetFullColor() const { return m_FullColor; };
		virtual Color GetEmptyColor() const { return m_EmptyColor; };
		virtual Color GetOutlineColor() const { return m_OutlineColor; };

		virtual void SetPercentage(float flPercent) { m_flPercentage = flPercent; };

		virtual float GetPercentage() const { return m_flPercentage; };

		
		virtual void SetSpacing(int spacing) { m_Spacing = spacing; };

		virtual void SetType(int type) { m_BarType = type; };
		virtual int GetType() const { return m_BarType; };
	private:
		void Init();

		Color m_FullColor;
		Color m_EmptyColor;
		Color m_OutlineColor;
		CHudTexture * m_IconFull;
		CHudTexture * m_IconEmpty;
		int m_BarType;
		int m_Spacing;
		float m_flPercentage;
	};
};


#endif