//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
//
// hud_redraw.cpp
//
#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "itextmessage.h"
#include "vgui_basepanel.h"
#include "hud_crosshair.h"
#include <vgui/ISurface.h>
#include "overlord_consolemanager.h"
#include "overlord_hintsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//For progress bar orientations
const int CHud::HUDPB_HORIZONTAL = 0;
const int CHud::HUDPB_VERTICAL = 1;
const int CHud::HUDPB_HORIZONTAL_INV = 2;

// Called when a ConVar changes value
static void FovChanged_Callback( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	ConVarRef var( pConVar );
	if ( engine->IsInGame() )
	{
		engine->ServerCmd( VarArgs( "fov %f\n", var.GetFloat() ) );
	}
}

static ConVar fov_watcher( "_fov", "0", 0, "Automates fov command to server.", FovChanged_Callback );

//-----------------------------------------------------------------------------
// Purpose: Think
//-----------------------------------------------------------------------------
void CHud::Think(void)
{
	// Determine the visibility of all hud elements
	for ( int i = 0; i < m_HudList.Size(); i++ )
	{
		// Visible?
		bool visible = m_HudList[i]->ShouldDraw();

		m_HudList[i]->SetActive( visible );

		// If it's a vgui panel, hide/show as appropriate
		vgui::Panel *pPanel = dynamic_cast<vgui::Panel*>(m_HudList[i]);
		if ( pPanel && pPanel->IsVisible() != visible )
		{
			pPanel->SetVisible( visible );
		}
		else if ( !pPanel )
		{
			// All HUD elements should now derive from vgui!!!
			Assert( 0 );
		}

		if ( visible )
		{
			m_HudList[i]->ProcessInput();
		}
	}

	// Let the active weapon at the keybits
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->HandleInput();
	}

	if ( ( m_flScreenShotTime > 0 ) && ( m_flScreenShotTime < gpGlobals->curtime ) )
	{
		if ( !IsX360() )
		{
			engine->ClientCmd( "screenshot" );
		}

		m_flScreenShotTime = -1;
	}

	// Handle the Overlord console manager and the hint system
	g_ConsoleManager.Run();
	g_HintManager.Run();
}

//-----------------------------------------------------------------------------
// Purpose:  The percentage passed in is expected and clamped to 0.0f to 1.0f
// Input  : x - 
//			y - 
//			width - 
//			height - 
//			percentage - 
//			clr - 
//			type - 
//-----------------------------------------------------------------------------
void CHud::DrawProgressBar( int x, int y, int width, int height, float percentage, Color& clr, unsigned char type )
{
	//Clamp our percentage
	percentage = min( 1.0f, percentage );
	percentage = max( 0.0f, percentage );

	Color lowColor = clr;
	lowColor[ 0 ] /= 2;
	lowColor[ 1 ] /= 2;
	lowColor[ 2 ] /= 2;

	//Draw a vertical progress bar
	if ( type == HUDPB_VERTICAL )
	{
		int	barOfs = height * percentage;

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x, y, x + width, y + barOfs );

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x, y + barOfs, x + width, y + height );
	}
	else if ( type == HUDPB_HORIZONTAL )
	{
		int	barOfs = width * percentage;

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x, y, x + barOfs, y +  height );

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x + barOfs, y, x + width, y + height );
	}
	else if ( type == HUDPB_HORIZONTAL_INV )
	{
		int	barOfs = width * percentage;

		surface()->DrawSetColor( clr );
		surface()->DrawFilledRect( x, y, x + barOfs, y + height );

		surface()->DrawSetColor( lowColor );
		surface()->DrawFilledRect( x + barOfs, y, x + width, y +  height );
	}
}

void CHud::DrawProgressBarWithOutline(int xpos, int ypos, int outlinedist,
											 int width, int height, float percent, Color full, Color empty, 
											 Color outline /* = Color(255, 255, 255, 255) */,
											 unsigned char type /* = HUDPB_HORIZONTAL */)
{
	percent = clamp(percent, 0.0f, 1.0f);

	int barOfs = (float)width * percent;
	if(type == HUDPB_HORIZONTAL )
	{
		int startX = xpos;
		int startY = ypos;
		int fullEndX = startX + barOfs;
		int fullEndY = startY + height;
		int emptyEndX = startX + width;
		int emptyEndY = fullEndY;	
			

		surface()->DrawSetColor( full );
		surface()->DrawFilledRect( startX, startY, fullEndX, fullEndY);

		surface()->DrawSetColor( empty );
		surface()->DrawFilledRect( fullEndX, startY, emptyEndX, emptyEndY );
	}
	else if(type == HUDPB_VERTICAL)
	{
		int startX = xpos + outlinedist;
		int startY = ypos + height - outlinedist;
		int fullEndX = startX + width - 2*outlinedist;
		int fullEndY = startY - barOfs;
		int emptyEndX = startX;
		int emptyEndY = startY - height + 2*outlinedist;
			
		surface()->DrawSetColor( full );
		surface()->DrawFilledRect( startX, fullEndY, fullEndX, startY);

		surface()->DrawSetColor( empty );
		surface()->DrawFilledRect( emptyEndX, emptyEndY, fullEndX, fullEndY );
	}
	else if(type == HUDPB_HORIZONTAL_INV)
	{
		int startX = xpos + width - 2*outlinedist;
		int startY = ypos + outlinedist;
		int fullEndX = startX - barOfs;
		int fullEndY = startY + height - 2*outlinedist;
		int emptyEndX = xpos + outlinedist;
		int emptyEndY = startY;	
			
		surface()->DrawSetColor( full );
		surface()->DrawFilledRect( fullEndX, startY, startX, fullEndY );

		surface()->DrawSetColor( empty );
		surface()->DrawFilledRect( emptyEndX, emptyEndY, fullEndX, fullEndY );
	}
	else
	{
		Warning("Unknown bar type!\n");
		return;
	}
	
	// Draw the outline
	if(outlinedist > 0)
	{
		surface()->DrawSetColor( outline );
		surface()->DrawOutlinedRect(xpos, ypos, xpos + width , ypos + height);
	}
	
}

//-----------------------------------------------------------------------------
// Purpose:  The percentage passed in is expected and clamped to 0.0f to 1.0f
// Input  : x - 
//			y - 
//			*icon - 
//			percentage - 
//			clr - 
//			type - 
//-----------------------------------------------------------------------------
void CHud::DrawIconProgressBar( int x, int y, CHudTexture *icon, CHudTexture *icon2, float percentage, Color& clr, int type, int wide, int high )
{
	if ( icon == NULL )
		return;

	//Clamp our percentage
	percentage = min( 1.0f, percentage );
	percentage = max( 0.0f, percentage );

	int	pheight = high;
	int	pwidth  = wide;

	int height = icon->Height();
	int width = icon->Width();

	//Draw a vertical progress bar
	if ( type == HUDPB_VERTICAL )
	{	
		int	barOfs = height * percentage;
	
		icon2->DrawSelfCropped( 
			x, y,  // Pos
			0, 0, width, barOfs, // Cropped subrect
			pwidth, (pheight*percentage), clr );

		icon->DrawSelfCropped( 
			x, y + (pheight*percentage),
			0, barOfs, width, height - barOfs, // Cropped subrect
			pwidth, pheight - (pheight*percentage), clr );
	}
	else if ( type == HUDPB_HORIZONTAL )
	{
		int	barOfs = width * percentage;

		icon2->DrawSelfCropped( 
			x, y,  // Pos
			0, 0, barOfs, height, // Cropped subrect
			clr );

		icon->DrawSelfCropped( 
			x + barOfs, y, 
			barOfs, 0, width - barOfs, height, // Cropped subrect
			clr );
	}
}


