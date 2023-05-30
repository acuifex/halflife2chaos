//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "iclientmode.h"
#include "c_chaos.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar chaos_effect_interval("chaos_effect_interval", "30", FCVAR_REPLICATED, "Time between each effect.");

using namespace vgui;

class CHUDChaos : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDChaos, vgui::Panel );
public:
	CHUDChaos( const char *pElementName );
	void Init( void );
	virtual void OnThink();
	virtual void Paint();
	virtual bool ShouldDraw( void );
	
	virtual void ApplySchemeSettings( IScheme *scheme );
private:
	float	m_barFraction;
};

DECLARE_HUDELEMENT( CHUDChaos );

CHUDChaos::CHUDChaos( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HUDChaos" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
}

void CHUDChaos::Init( void )
{
	m_barFraction = 0.0f;
}

void CHUDChaos::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled( false );
    SetSize( ScreenWidth(), ScreenHeight() );
}

bool CHUDChaos::ShouldDraw()
{
	return true;
}

void CHUDChaos::OnThink()
{
	BaseClass::OnThink();

	m_barFraction = Clamp( 1.0f - g_CC.m_flNextEffectRem / chaos_effect_interval.GetFloat(), 0.0f, 1.0f );
}


ConVar chaos_bar_r("chaos_bar_r", "255");
ConVar chaos_bar_g("chaos_bar_g", "220");
ConVar chaos_bar_b("chaos_bar_b", "0");
ConVar chaos_bar_a("chaos_bar_a", "255");

void CHUDChaos::Paint()
{
	Color barColor(chaos_bar_r.GetInt(),chaos_bar_g.GetInt(),chaos_bar_b.GetInt(),chaos_bar_a.GetInt());
	gHUD.DrawProgressBar( 0, 0, GetWide(), 10, m_barFraction, barColor, CHud::HUDPB_HORIZONTAL_INV );
}
