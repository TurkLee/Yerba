#include "GrenadeRange.h"

/* Declarations */
GrenadeRange::inferno_startburn_event     inferno_startburn_listener;
GrenadeRange::inferno_expire_event        inferno_expire_listener;
GrenadeRange::round_end_event             round_end_listener;

/* Molotov logs */
std::vector<molotov_info> molotov_logs;

/* Grenade range */
GrenadeRange g_grenaderange;

/* Listeners */
void GrenadeRange::listener()
{
	g_pEventManager->AddListener(&inferno_startburn_listener, "inferno_startburn", false);

	g_pEventManager->AddListener(&inferno_expire_listener, "inferno_expire", false);

	g_pEventManager->AddListener(&round_end_listener, "round_end", false);
}

void GrenadeRange::draw()
{
	if (!g_Settings.Visualizations.bProjectiles)
		return;

	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	this->molotov();
}

/* Inferno startburn */
void GrenadeRange::inferno_startburn_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Molotov grenade */
	if (strstr(p_event->GetName(), "inferno_startburn"))
	{
		Vector position(p_event->GetFloat("x"), p_event->GetFloat("y"), p_event->GetFloat("z"));

		Utils::Log("%, %, %", position.x, position.y, position.z);

		molotov_logs.emplace_back(molotov_info(position, true, p_event->GetInt("userid")));

		Utils::Log("inferno_startburn");
	}
}

/* Inferno expire */
void GrenadeRange::inferno_expire_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Molotov grenade */
	if (strstr(p_event->GetName(), "inferno_expire"))
	{
		//molotov_logs.emplace_back(molotov_info(position, false, p_event->GetInt("entityid")));

		for (int i = 0; i < molotov_logs.size(); i++)
			molotov_logs.erase(molotov_logs.begin() + i);

		Utils::Log("inferno_expire");
	}
}

/* Round end */
void GrenadeRange::round_end_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Round end */
	if (strstr(p_event->GetName(), "round_end"))
	{
		//molotov_logs.emplace_back(molotov_info(position, false, p_event->GetInt("entityid")));

		//for (int i = 0; i < molotov_logs.size(); i++)
		//	molotov_logs.erase(molotov_logs.end());

		Utils::Log("round_end");
	}
}

void GrenadeRange::molotov()
{
	for (int i = 0; i < molotov_logs.size(); i++)
	{
		TraceCircle(molotov_logs[i].position);

		Vector screen;
		const std::string to_render = "fire"; //hdr->szName;
		if (Utils::WorldToScreen(molotov_logs[i].position, screen))
		{
			int iProjectile = g_Settings.Visualizations.cProjectiles.alpha;

			g_Render.String(screen.x - 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x - 0, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x + 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);

			g_Render.String(screen.x - 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x - 0, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x + 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);

			g_Render.String(screen.x - 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x - 0, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);
			g_Render.String(screen.x + 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iProjectile), g_Fonts.pFontTahoma8.get(), to_render);

			g_Render.String(screen.x, screen.y, CD3DFONT_CENTERED_X, g_Settings.Visualizations.cProjectiles, g_Fonts.pFontTahoma8.get(), to_render);
		}
	}
}

void GrenadeRange::TraceCircle(Vector position)
{
	Vector prev_scr_pos{ -1, -1, -1 };
	Vector scr_pos;

	float step = M_PI * 2.0 / 150;

	float rad = 100.f;

	Vector origin = position;

	static float hue_offset = 0;
	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z + 2);
		Vector tracepos(origin.x, origin.y, origin.z + 2);

		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;

		ray.Init(tracepos, pos);

		g_pEngineTrace->TraceRay(ray, MASK_SPLITAREAPORTAL, &filter, &trace);

		if (Utils::WorldToScreen(trace.endpos, scr_pos))
		{
			if (prev_scr_pos != Vector{ -1, -1, -1 })
			{
				int hue = RAD2DEG(rotation) + hue_offset;

				Color color = Color::FromHSB(1, hue / 360.f, 1);

				g_Render.Line(prev_scr_pos.x, prev_scr_pos.y, scr_pos.x, scr_pos.y, color);
			}
			prev_scr_pos = scr_pos;
		}
	}
	hue_offset -= 0.5;
}