#pragma once
#include "..\Utils\GlobalVars.h"
#include "..\Settings\Settings.h"

class Misc
{
public:
	void OnCreateMove()
	{
		this->pCmd = g::pCmd;
		this->pLocal = g::pLocalEntity;

		/* Bunny hop */
		if (g_Settings.Miscellaneous.bBhopEnabled)
			this->BunnyHop();

		/* Air strafe */
		if (g_Settings.Miscellaneous.bAutoStrafer)
			this->AirStrafe();

		/* Chat spammer*/
		if (g_Settings.Miscellaneous.bChatSpammer)
			this->ChatSpammer();

		/* Radio spammer*/
		if (g_Settings.Miscellaneous.bRadioSpammer)
			this->RadioSpammer();

		/* Clan tag spammer */
		if (g_Settings.Miscellaneous.bClanTagSpammer)
			this->ClanTagSpammer();

		/* Reveal competitive ranks */
		if (g_Settings.Miscellaneous.bRevealCompetitiveRanks)
			this->RevealCompetitiveRanks();

		/* Infinite duck */
		if (g_Settings.Miscellaneous.bInfiniteDuck)
			pCmd->buttons |= IN_BULLRUSH;

		/* Weapon switch spammer */
		if (g_Settings.Miscellaneous.bWeaponSwitchSpammer)
			g_pEngine->ExecuteClientCmd("lastinv");
	};
private:
	CUserCmd*     pCmd;
	C_BaseEntity* pLocal;

	__forceinline static float NormalizeAngle(float angle) {
		if (angle > 180.f || angle < -180.f) {
			auto epic = roundf(fabsf(angle / 360.f));

			if (angle < 0.f)
				angle = angle + 360.f * epic;
			else
				angle = angle - 360.f * epic;
		}
		return angle;
	}

	void AirStrafe() {

		static float circle_yaw = 0.f,
			old_yaw = 0.f;

		if (!g_Settings.Miscellaneous.bAutoStrafer)
			return;

		if (this->pLocal->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
			return;

		if (!(this->pLocal->GetFlags() & FL_ONGROUND))
		{
			float yaw_delta = NormalizeAngle(pCmd->viewangles.y - old_yaw);

			circle_yaw = old_yaw = pCmd->viewangles.y;

			if (yaw_delta > 0.f)
				pCmd->sidemove = -450.f;
			else if (yaw_delta < 0.f)
				pCmd->sidemove = 450.f;
		}
	}

	void BunnyHop() const
	{
		if (this->pLocal->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
			return;

		static bool bLastJumped = false;
		static bool bShouldFake = false;

		if (!bLastJumped && bShouldFake)
		{
			bShouldFake = false;
			pCmd->buttons |= IN_JUMP;
		}
		else if (pCmd->buttons & IN_JUMP)
		{
			if (pLocal->GetFlags() & FL_ONGROUND)
				bShouldFake = bLastJumped = true;
			else
			{
				pCmd->buttons &= ~IN_JUMP;
				bLastJumped = false;
			}
		}
		else
			bShouldFake = bLastJumped = false;
	}

	void ChatSpammer() const
	{
		static DWORD Spammed = 0;
		if (GetTickCount() - Spammed > 800)
		{
			Spammed = GetTickCount();
			g_pEngine->ExecuteClientCmd("say Get good. Get yerba.");
		}
	}

	void RadioSpammer() const
	{
		std::vector<std::string> RadioSpam = { "coverme" , "takepoint" , "holdpos", "regroup", "followme",
			"takingfire", "go", "fallback", "sticktog",
			"getinpos", "stormfront", "report", "roger",
			"enemyspot", "needbackup", "sectorclear",
			"inposition", "reportingin", "getout",
			"negative", "enemydown" };

		static DWORD Spammed = 0;
		if (GetTickCount() - Spammed > 800)
		{
			Spammed = GetTickCount();
			std::string msg = RadioSpam[rand() % RadioSpam.size()];
			g_pEngine->ExecuteClientCmd(msg.c_str());
		}
	}

	void RevealCompetitiveRanks() const
	{
		using ServerRankRevealAll = char(__cdecl*)(int*);

		static auto fnServerRankRevealAll = Utils::FindSignature("client_panorama.dll", "55 8B EC 8B 0D ? ? ? ? 85 C9 75 28 A1 ? ? ? ? 68 ? ? ? ? 8B 08 8B 01 FF 50 04 85 C0 74 0B 8B C8 E8 ? ? ? ? 8B C8 EB 02 33 C9 89 0D ? ? ? ? 8B 45 08");

		if (fnServerRankRevealAll) {
			int v[3] = { 0,0,0 };

			reinterpret_cast<ServerRankRevealAll>(fnServerRankRevealAll)(v);
		}
	}

	void SetClanTag(const char* tag, const char* name)
	{
		static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utils::FindSignature("engine.dll", "53 56 57 8B DA 8B F9 FF 15")));
		pSetClanTag(tag, name);
	}

	void Marquee(std::string& ClanTag)
	{
		std::string temp = ClanTag;
		ClanTag.erase(0, 1);
		ClanTag += temp[0];
	}

	void ClanTagSpammer()
	{
		static std::string cur_clantag = " yerba ";
		static float oldTime;

		static size_t LastTime = 0;
		if (GetTickCount() > LastTime)
		{
			Marquee(cur_clantag);
			SetClanTag(cur_clantag.c_str(), "yerba");
			LastTime = GetTickCount() + 400;
		}
	}
};

extern Misc g_Misc;