#include "HitMarker.h"
#pragma comment(lib, "Winmm.lib")

/* Variables */
int hitmarkerTime = 0;

/* Declarations */
hitmarker::player_hurt_event player_hurt_listener;

/* Hitmarker */
hitmarker g_hitmarker;

/* Functions */
void hitmarker::listener()
{
	g_pEventManager->AddListener(&player_hurt_listener, "player_hurt", false);
}

void hitmarker::run()
{
	if (!g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		return;

	/* Draw hitmarker if enabled */
	if (g_Settings.Visualizations.bHitmarker || g_Settings.Visualizations.bHitmarkerSound)
		this->draw();
}

void hitmarker::player_hurt_event::FireGameEvent(IGameEvent * p_event)
{
	/* Return if not event */
	if (!p_event)
		return;

	/* Local player */
	auto localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	/* Get the attacker */
	auto attacker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("attacker")));

	/* Check if we are attacker */
	if (attacker == localplayer)
	{
		/* Set the hitmarker time to 255 */
		hitmarkerTime = 255;

		if (g_Settings.Visualizations.bHitmarkerSound)
			g_pSurface->play_sound("buttons\\arena_switch_press_02.wav");
	}
}

void hitmarker::draw()
{
	/* Screen size */
	int screenWidth, screenHeight;
	g_pEngine->GetScreenSize(screenWidth, screenHeight);

	/* Screen middle */
	int ScreenWidthMid = screenWidth / 2;
	int ScreenHeightMid = screenHeight / 2;

	/* Draw the hitmarker */
	if (hitmarkerTime > 0)
	{
		/* Set alpha to hitmarker time */
		float alpha = hitmarkerTime;

		/* Screen */
		if (g_Settings.Visualizations.bHitmarker)
		{
			g_Render.Line(ScreenWidthMid - 5, ScreenHeightMid - 5, ScreenWidthMid + 5, ScreenHeightMid + 5, Color(255, 255, 255, alpha));
			g_Render.Line(ScreenWidthMid - 6, ScreenHeightMid - 6, ScreenWidthMid + 6, ScreenHeightMid + 6, Color(255, 255, 255, alpha));
			g_Render.Line(ScreenWidthMid + 5, ScreenHeightMid - 5, ScreenWidthMid - 5, ScreenHeightMid + 5, Color(255, 255, 255, alpha));
			g_Render.Line(ScreenWidthMid + 6, ScreenHeightMid - 6, ScreenWidthMid - 6, ScreenHeightMid + 6, Color(255, 255, 255, alpha));
		}

		/* Hitmarker fade animation */
		hitmarkerTime -= 2;
	}
}