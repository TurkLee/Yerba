#include "Logs.h"

/* Declarations */
Logs::player_hurt_event player_hurt_listener;

/* Logs */
Logs g_logs;

typedef void(__cdecl* MsgFn)(const char* msg, va_list);
inline void ConMsg(const char* msg, ...)
{
	if (msg == nullptr)
		return; /* If no string was passed, or it was null then don't do anything */
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg"); /* This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there) */
	char buffer[989];
	va_list list; /* Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments */
	va_start(list, msg);
	vsprintf_s(buffer, msg, list);
	va_end(list);
	fn(buffer, list); /* Calls the function, we got the address above. */
}

/* Listeners */
void Logs::listener()
{
	/* Return if we don't have enabled */
	if (!g_Settings.Miscellaneous.bEventLogs)
		return;

	g_pEventManager->AddListener(&player_hurt_listener, "player_hurt", false);
}

/* Getting player info */
PlayerInfo_s GetInfo(int Index) {
	PlayerInfo_s Info;
	g_pEngine->GetPlayerInfo(Index, &Info);
	return Info;
}

/* Hitgroup */
auto HitgroupToString = [](int hitgroup) -> char*
{
	if (hitgroup == 1)
		return "head";
	else if (hitgroup == 2)
		return "chest";
	else if (hitgroup == 3)
		return "stomach";
	else if (hitgroup == 4)
		return "left arm";
	else if (hitgroup == 5)
		return "right arm";
	else if (hitgroup == 6)
		return "left leg";
	else if (hitgroup == 7)
		return "right leg";
	else if (hitgroup == 8)
		return "body";
};

/* Player hurt */
void Logs::player_hurt_event::FireGameEvent(IGameEvent * p_event)
{
	/* Check if we are in game */
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	/* Player hurt */
	if (strstr(p_event->GetName(), "player_hurt"))
	{
		/* Local player */
		auto localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

		/* Get the attacker */
		auto attacker = p_event->GetInt("attacker");

		/* Get the attacker for checking */
		auto attackercheck = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("attacker")));

		/* Used for checking if we are getting damaged */
		auto shooter = g_pEntityList->GetClientEntity(g_pEngine->GetPlayerForUserID(p_event->GetInt("userid")));

		/* Get the guy who we are shooting */
		auto dead = p_event->GetInt("userid");

		int hitgroup = p_event->GetInt("hitgroup");

		if (attackercheck == localplayer)
		{
			if (attacker || dead)
			{
				if (!g_Settings.Miscellaneous.bEventLogs[0])
					return;

				char* szHitgroup = HitgroupToString(hitgroup);
				PlayerInfo_s killed_info = GetInfo(g_pEngine->GetPlayerForUserID(dead));
				PlayerInfo_s killer_info = GetInfo(g_pEngine->GetPlayerForUserID(attacker));

				g_pCvar->console_printf("[");
				g_pCvar->console_color_printf(g_Settings.Miscellaneous.cMenuColor, "yerba");
				g_pCvar->console_printf("] ");

				/// Hit Patrick in the head for 100 dmg (0 health remaining)
				ConMsg("Hit %s in the %s for %s dmg (%s health remaining)\n",
					killed_info.szName, szHitgroup,  p_event->GetString("dmg_health"), p_event->GetString("health"));
			}
		}

		if (attacker || dead)
		{
			if (!g_Settings.Miscellaneous.bEventLogs[1])
				return;

			char* szHitgroup = HitgroupToString(hitgroup);
			PlayerInfo_s killed_info = GetInfo(g_pEngine->GetPlayerForUserID(dead));
			PlayerInfo_s killer_info = GetInfo(g_pEngine->GetPlayerForUserID(attacker));

			if (shooter == localplayer)
			{
				g_pCvar->console_printf("[");
				g_pCvar->console_color_printf(g_Settings.Miscellaneous.cMenuColor, "yerba");
				g_pCvar->console_printf("] ");

				/// Harmed by Patrick in the head for 100 dmg
				ConMsg("Harmed by %s in the %s for %s dmg\n",
					killer_info.szName, szHitgroup, p_event->GetString("dmg_health"));
			}
		}
	}
}