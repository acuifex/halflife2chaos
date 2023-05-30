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
#include "vgui/ILocalize.h"
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
	vgui::HFont m_hFont;
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
	m_hFont = g_hFontTrebuchet24;
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

ConVar chaos_text_x("chaos_text_x", "0.05");
ConVar chaos_text_y("chaos_text_y", "0");

ConVar chaos_text_spacing("chaos_text_spacing", "1");

ConVar chaos_text_r("chaos_text_r", "255");
ConVar chaos_text_g("chaos_text_g", "220");
ConVar chaos_text_b("chaos_text_b", "0");
ConVar chaos_text_a("chaos_text_a", "255");
ConVar chaos_textfade_r("chaos_textfade_r", "255");
ConVar chaos_textfade_g("chaos_textfade_g", "48");
ConVar chaos_textfade_b("chaos_textfade_b", "0");
ConVar chaos_textfade_a("chaos_textfade_a", "255");

void CHUDChaos::Paint()
{
	int width, height;
	GetSize(width, height);

	Color barColor(chaos_bar_r.GetInt(),chaos_bar_g.GetInt(),chaos_bar_b.GetInt(),chaos_bar_a.GetInt());
	gHUD.DrawProgressBar( 0, 0, width, 10, m_barFraction, barColor, CHud::HUDPB_HORIZONTAL_INV );

	// effect text stuff bellow. all of it could go into its own panel but eh
	wchar_t textBuf[ 256 ];
	Color textColor(chaos_text_r.GetInt(),chaos_text_g.GetInt(),chaos_text_b.GetInt(),chaos_text_a.GetInt());

	surface()->DrawSetTextFont( m_hFont );
	int fontHeight = surface()->GetFontTall(m_hFont);
	for (int i = 0; i < g_CC.m_activeEffects.Count(); ++i)
	{
		float scale = g_CC.m_activeEffects[i].GetTimeLeftFraction();

		// lmao the ammount of letters that this function has
		V_snwprintf(textBuf, ARRAYSIZE( textBuf ), L"%s (%.2f)", // TODO: display seconds left and omit if it's single fire
			g_CC.m_activeEffects[i].m_strName, scale);

		Color textColorFaded(chaos_textfade_r.GetInt(),chaos_textfade_g.GetInt(),chaos_textfade_b.GetInt(),chaos_textfade_a.GetInt());
		for (int i = 0; i < 4; ++i)
		{
			textColorFaded[i] = textColorFaded[i] + scale * (textColor[i] - textColorFaded[i]);
		}

		surface()->DrawSetTextColor( textColorFaded );
		surface()->DrawSetTextPos( 
			width * chaos_text_x.GetFloat(), 
			(height * chaos_text_y.GetFloat()) + (i * fontHeight * chaos_text_spacing.GetFloat())
		);
		surface()->DrawPrintText( textBuf, wcslen(textBuf) );
	}
}