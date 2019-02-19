#include "VisualizeSounds.h"

/* Declarations */
soundesp::player_hurt_event player_hurt_listener;
soundesp::player_footstep_event player_footstep_listener;

/* Sound esp */
soundesp g_soundesp;

/* Sound logs */
std::vector<sound_info> sound_logs;

/* Sound esp draw */
void soundesp::draw_circle(Color color, Vector position)
{
	BeamInfo_t beam_info;
	beam_info.m_nType = TE_BEAMRINGPOINT;
	beam_info.m_pszModelName = "sprites/purplelaser1.vmt";
	beam_info.m_nModelIndex = g_MdlInfo->GetModelIndex("sprites/purplelaser1.vmt");
	//beam_info.m_pszHaloName = "sprites/purplelaser1.vmt";
	beam_info.m_nHaloIndex = -1;
	beam_info.m_flHaloScale = 5;
	beam_info.m_flLife = .50f;
	beam_info.m_flWidth = 10.f;
	beam_info.m_flFadeLength = 1.0f;
	beam_info.m_flAmplitude = 0.f;
	beam_info.m_flRed = color.r();
	beam_info.m_flGreen = color.g();
	beam_info.m_flBlue = color.b();
	beam_info.m_flBrightness = color.a();
	beam_info.m_flSpeed = 0.f;
	beam_info.m_nStartFrame = 0.f;
	beam_info.m_flFrameRate = 60.f;
	beam_info.m_nSegments = -1;
	//beam_info.m_bRenderable = true;
	beam_info.m_nFlags = FBEAM_FADEOUT;
	beam_info.m_vecCenter = position + Vector(0, 0, 5);
	beam_info.m_flStartRadius = 20.f;
	beam_info.m_flEndRadius = 640.f;

	auto beam = g_pViewRenderBeams->CreateBeamRingPoint(beam_info);

	if (beam)
		g_pViewRenderBeams->DrawBeam(beam);
}

/* Listeners */
void soundesp::listener()
{
	g_pEventManager->AddListener(&player_footstep_listener, "player_footstep", false);

	g_pEventManager->AddListener(&player_hurt_listener, "player_hurt", false);
}

/* Footstep */
void soundesp::player_footstep_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Check if we have the sound esp enabled */
	if (!g_Settings.Visualizations.bSound)
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Local player */
	auto localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	/* Footstep */
	if (strstr(p_event->GetName(), "player_footstep"))
	{
		/* Get the walker */
		auto walker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("userid")));

		/* Return if walker is nullptr */
		if (walker == nullptr)
			return;

		/* Return if walker is dormant */
		if (walker->IsDormant())
			return;

		/* Clock */
		static int timer;

		timer += 1;

		if (timer > 1)
			timer = 0;

		if (walker->GetTeam() != localplayer->GetTeam())
		{
			if (walker && timer < 1)
			{
				sound_logs.push_back(sound_info(walker->GetAbsOrigin(), g_pGlobalVars->curtime, p_event->GetInt("userid")));
			}
		}
	}
}

/* Player hurt */
void soundesp::player_hurt_event::FireGameEvent(IGameEvent * p_event)
{
	/* If we are connected */
	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;

	/* Check if we have the sound esp enabled */
	if (!g_Settings.Visualizations.bSound)
		return;

	/* Return if not event */
	if (!p_event)
		return;

	/* Local player */
	auto localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	/* Get the attacker */
	auto attacker = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("attacker")));

	/* Get the victim */
	auto victim = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("userid")));


	/* Clock */
	static int timer;

	timer += 1;

	if (timer > 2)
		timer = 0;

	/* Check if we are attacker */
	if (attacker == localplayer)
	{
		if (timer < 1)
			sound_logs.push_back(sound_info(victim->GetAbsOrigin(), g_pGlobalVars->curtime, p_event->GetInt("userid")));
	}
}

/* Draw */
void soundesp::draw()
{
	/* If footstep esp is enabled */
	if (!g_Settings.Visualizations.bSound)
		return;

	/* If we are connected */
	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;

	//CUtlVector<SndInfo_t> sndList;
	//sndList.RemoveAll();
	//g_pEngineSound->GetActiveSounds(sndList);

	/*for (int i = 0; i < sndList.Count(); i++)
	{
		SndInfo_t sndInfo = sndList.Element(i);

		if (sndInfo.m_nSoundSource)
		{
			if (sndInfo.m_nChannel == 4)
			{
				if (sndInfo.m_bUpdatePositions)
				{
					C_BaseEntity* target = (g_pEntityList->GetClientEntity(sndList[i].m_nSoundSource));

					if (target && target->GetTeam() != g::pLocalEntity->GetTeam())
					{
						if (!sndInfo.m_pOrigin)
							continue;

						g_soundesp.draw_circle(g_Settings.Visualizations.cSound, *sndInfo.m_pOrigin);
					}
				}
			}
		}
	}*/

	for (unsigned int i = 0; i < sound_logs.size(); i++)
	{
		/* Draw the beam */
		g_soundesp.draw_circle(g_Settings.Visualizations.cSound, sound_logs[i].position);

		sound_logs.erase(sound_logs.begin() + i);
	}
}