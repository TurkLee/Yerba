#include "BulletTracer.h"

/* Declarations */
bullettracer::player_hurt_event player_hurt_listener;
bullettracer::bullet_impact_event bullet_impact_listener;

/* Bulletracer */
bullettracer g_bullettracer;

/* Trace logs */
std::vector<trace_info> trace_logs;

/* Color used for the tracers */
Color color;

/* Bullet tracer draw */
void bullettracer::draw_beam(Vector src, Vector end, Color color)
{
	BeamInfo_t info;
	info.m_nType = TE_BEAMPOINTS;
	info.m_pszModelName = "sprites/purplelaser1.vmt";
	//info.m_pszHaloName = -1;
	info.m_nHaloIndex = -1;
	info.m_flHaloScale = 0.f;
	info.m_flLife = g_Settings.Visualizations.iTracerTime;
	info.m_flWidth = 4;
	info.m_flEndWidth = 6;
	info.m_flFadeLength = 0;
	info.m_flAmplitude = 0.f;
	info.m_flBrightness = color.a();
	info.m_flSpeed = 1.f;
	info.m_nStartFrame = 0;
	info.m_flFrameRate = 0;
	info.m_flRed = color.r();
	info.m_flGreen = color.g();
	info.m_flBlue = color.b();
	info.m_nSegments = -1;
	info.m_bRenderable = true;
	info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE;
	info.m_vecStart = src - Vector(0, 0, 5); //Vector(0, 0, 5);
	info.m_vecEnd = end - Vector(0, 0, 5);

	Beam_t* beam = g_pViewRenderBeams->CreateBeamPoints(info);

	if (beam)
		g_pViewRenderBeams->DrawBeam(beam);
}

/* Listeners */
void bullettracer::listener()
{
	g_pEventManager->AddListener(&bullet_impact_listener, "bullet_impact", false);

	g_pEventManager->AddListener(&player_hurt_listener, "player_hurt", false);
}

/* Bullet impact */
void bullettracer::bullet_impact_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Return if localplayer is null */
	if (g::pLocalEntity == nullptr)
		return;

	/* Check if we have the bullet tracers enabled */
	if (!g_Settings.Visualizations.bBulletTracer)
		return;

	/* Bullet impact */
	if (strstr(p_event->GetName(), "bullet_impact"))
	{
		Vector position(p_event->GetFloat("x"), p_event->GetFloat("y"), p_event->GetFloat("z"));

		/* Get the shooter */
		auto shooter = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("userid")));

		if (shooter == nullptr)
			return;

		if (shooter->IsDormant())
			return;

		if (shooter)
			trace_logs.push_back(trace_info(shooter->GetEyePosition(), position, g_pGlobalVars->curtime, p_event->GetInt("userid")));

		if (shooter == g::pLocalEntity)
			color = g_Settings.Visualizations.cLocalTracer;
		else if (shooter->GetTeam() != g::pLocalEntity->GetTeam())
			color = g_Settings.Visualizations.cEnemyTracer;
		else
			color = Color(0, 0, 0, 0);
	}
}

/* Player hurt */
void bullettracer::player_hurt_event::FireGameEvent(IGameEvent * p_event)
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
		color = g_Settings.Visualizations.cLocalHurt;
	}
}

/* Draw */
void bullettracer::draw()
{
	/* Check if we have the bullet tracers enabled */
	if (!g_Settings.Visualizations.bBulletTracer)
		return;

	/* If we are connected */
	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;

	for (unsigned int i = 0; i < trace_logs.size(); i++)
	{
		/* Draw the beam */
		g_bullettracer.draw_beam(trace_logs[i].start, trace_logs[i].position, color);

		trace_logs.erase(trace_logs.begin() + i);
	}
}