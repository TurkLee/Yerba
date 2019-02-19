#include "Visualizations.h"
ESP g_ESP;

void ESP::Render()
{
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	if (g::pLocalEntity == nullptr)
		return;

	/* Offscreen ESP */
	if (g_Settings.Visualizations.bOffscreenESP)
		OffscreenESP(g_Settings.Visualizations.cOffscreenESP);

	/* Force Crosshair */
	if (g_Settings.Visualizations.bForceCrosshair && g::pLocalEntity->IsAlive() && !g::pLocalEntity->GetScoped())
	{
		ConVar* cross = g_pCvar->FindVar("weapon_debug_spread_show");
		SpoofedConvar* cross_spoofed = new SpoofedConvar(cross);
		cross_spoofed->SetInt(3);
	}

	/* Grenade Prediction */
	if (g_Settings.Visualizations.bGrenadePrediction)
		GrenadePrediction();

	auto calculate_dynamic_box = [](C_BaseEntity* entity, box_data& box) -> bool
	{
		if (!entity)
			return false;

		const auto& tran_frame = entity->get_rgflcoordinateframe();

		const auto min = entity->GetOBBMins();
		const auto max = entity->GetOBBMaxs();

		Vector screen_boxes[8];

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		for (auto i = 0; i <= 7; ++i)
			if (!Utils::WorldToScreen(g_Math.VectorTransform(points[i], tran_frame),
				screen_boxes[i]))
				return false;

		Vector box_array[] =
		{
			screen_boxes[3], /* Front left */
			screen_boxes[5], /* Back right */
			screen_boxes[0], /* Back left */
			screen_boxes[4], /* Front right */
			screen_boxes[2], /* Front right */
			screen_boxes[1], /* Back right */
			screen_boxes[6], /* Back left */
			screen_boxes[7]  /* Front Left */
		};

		auto left = screen_boxes[3].x, bottom = screen_boxes[3].y, right = screen_boxes[3].x, top = screen_boxes[3].y;

		for (auto i = 0; i <= 7; i++)
		{
			if (left > box_array[i].x)
				left = box_array[i].x;

			if (bottom < box_array[i].y)
				bottom = box_array[i].y;

			if (right < box_array[i].x)
				right = box_array[i].x;

			if (top > box_array[i].y)
				top = box_array[i].y;
		}

		box.x = static_cast<int>(left);
		box.y = static_cast<int>(top);
		box.w = static_cast<int>(right - left);
		box.h = static_cast<int>(bottom - top);

		return true;
	};

	auto calculate_box_static = [](C_BaseEntity * entity, box_data& box) -> bool
	{
		Vector top, down, air, s[2];

		Vector adjust = Vector(0, 0, -15) * entity->GetCrouchAmount();

		if (!(entity->GetFlags() & FL_ONGROUND) && (entity->GetMoveType() != MOVETYPE_LADDER))
			air = Vector(0, 0, 10);
		else
			air = Vector(0, 0, 0);

		down = entity->GetAbsOrigin() + air;
		top = down + Vector(0, 0, 72) + adjust;

		if (Utils::WorldToScreen(top, s[1]) && Utils::WorldToScreen(down, s[0]))
		{
			Vector delta = s[1] - s[0];

			box.h = fabsf(delta.y) + 6;
			box.w = box.h / 2 + 5;

			box.x = s[1].x - (box.w / 2) + 2;
			box.y = s[1].y - 1;

			return true;
		}

		return false;
	};

	auto paint_other_esp = [&](C_BaseEntity * entity, ClientClass * client_class)
	{
		if (!entity)
			return;

		box_data box;

		/* Bomb */
		if (g_Settings.Visualizations.bBomb && client_class->ClassID == CC4 || client_class->ClassID == CPlantedC4)
			Bomb(entity);

		/* Projectiles */
		if (g_Settings.Visualizations.bProjectiles && client_class->ClassID == CMolotovProjectile || client_class->ClassID == CSmokeGrenadeProjectile || client_class->ClassID == CDecoyProjectile || client_class->ClassID == CBaseCSGrenadeProjectile)
			Projectiles(entity);

		/* Dynamic Boxes */
		if (!calculate_dynamic_box(entity, box))
			return;

		/* Dropped Weapons */
		if (g_Settings.Visualizations.bItems && ((strstr(client_class->pNetworkName, "Weapon") || client_class->ClassID == CDEagle || client_class->ClassID == CAK47)))
			DroppedWeapons(entity, box);
	};

	auto paint_player_esp = [&](C_BaseEntity * entity)
	{
		if (!entity || !g::pLocalEntity || entity->GetTeam() == g::pLocalEntity->GetTeam())
			return;

		box_data box;

		/* Dynamic box */
		if (g_Settings.Visualizations.bDynamic ? !calculate_dynamic_box(entity, box) : !calculate_box_static(entity, box))
			return;

		/* Dormant fade */
		const int alpha = esp_alpha_fade[entity->EntIndex()] * 255;
		const int inline_alpha = esp_alpha_fade[entity->EntIndex()] * 150;
		const int outline_alpha = alpha * 0.9f;

		/* Bounding box */
		if (g_Settings.Visualizations.bBox && entity->GetTeam() != g::pLocalEntity->GetTeam())
			Box(box, g_Settings.Visualizations.cBox, g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255);

		/* Name */
		if (g_Settings.Visualizations.bName && entity->GetTeam() != g::pLocalEntity->GetTeam())
			Name(box, entity, g_Settings.Visualizations.cName, g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255);

		/* Health bar */
		if (g_Settings.Visualizations.bHealth && entity->GetTeam() != g::pLocalEntity->GetTeam())
			Health(box, entity, Color::Black(), g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255, g_Settings.Visualizations.bDormant ? inline_alpha : 150);

		/* Ammo bar */
		if (g_Settings.Visualizations.bWeaponAmmo && entity->GetTeam() != g::pLocalEntity->GetTeam())
			Ammo(box, entity, g_Settings.Visualizations.cWeaponAmmo, g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255, g_Settings.Visualizations.bDormant ? inline_alpha : 150);

		/* Weapons */
		if ((g_Settings.Visualizations.bWeaponIcon || g_Settings.Visualizations.bWeaponName) && entity->GetTeam() != g::pLocalEntity->GetTeam())
			WeaponName(box, entity, Color::Black(), g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255);

		/* Skeleton */
		if (g_Settings.Visualizations.bSkeleton && entity->GetTeam() != g::pLocalEntity->GetTeam())
			Skeleton(entity, g_Settings.Visualizations.cSkeleton, g_Settings.Visualizations.bDormant ? alpha : 255);

		/* Flags */
		if (entity->GetTeam() != g::pLocalEntity->GetTeam())
			Flags(box, entity, g_Settings.Visualizations.bDormant ? alpha : 255, g_Settings.Visualizations.bDormant ? outline_alpha : 255);
	};

	for (auto i = 0; i < g_pEntityList->GetHighestEntityIndex(); i++)
	{
		auto entity = g_pEntityList->GetClientEntity(i);

		if (!entity || !g::pLocalEntity)
			continue;

		const auto class_id = entity->GetClientClass();

		/* Paint miscellaneous esp */
		paint_other_esp(entity, class_id);

		/* Filter player out */
		if (!entity->IsAlive()
			|| entity == g::pLocalEntity
			|| class_id->ClassID != CCSPlayer)
			continue;

		const auto in = (1.f / 0.2f) * g_pGlobalVars->frametime; /* Fade in */
		const auto out = (1.f / 7.f) * g_pGlobalVars->frametime; /* Fade out ( 7 = time) */

		if (!entity->IsDormant())
		{
			if (esp_alpha_fade[entity->EntIndex()] < 1.f)
				esp_alpha_fade[entity->EntIndex()] += in;
		}
		else
		{
			if (esp_alpha_fade[entity->EntIndex()] > 0.f)
				esp_alpha_fade[entity->EntIndex()] -= out;
		}


		esp_alpha_fade[entity->EntIndex()] = (esp_alpha_fade[entity->EntIndex()] > 1.f
			? 1.f
			: esp_alpha_fade[entity->EntIndex()] < 0.f
			? 0.f
			: esp_alpha_fade[entity->EntIndex()]);

		/* Paint player ESP */
		paint_player_esp(entity);
	}
}

/* ========== Player ESP ========== */

void ESP::Box(box_data box, Color colour, int alpha, int outline_alpha)
{
	g_Render.OutlinedRect(box.x, box.y, box.w, box.h, colour(alpha));
	g_Render.OutlinedRect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(25, 25, 25, outline_alpha));
	g_Render.OutlinedRect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(25, 25, 25, outline_alpha));
}

void ESP::Name(box_data box, C_BaseEntity * entity, Color colour, int alpha, int outline_alpha)
{
	static auto sanitize = [](char * name) -> std::string {
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 20) {
			tmp.erase(20, (tmp.length() - 20));
			tmp.append("...");
		}

		return tmp;
	};

	std::string name = sanitize(entity->GetPlayerInfo().szName);
	std::string bot;

	if (entity->GetPlayerInfo().fakeplayer)
		bot = "[BOT] ";

	g_Render.String(SPoint(box.x + box.w / 2, box.y - 14), CD3DFONT_CENTERED_X, colour(alpha), g_Fonts.pFontESPFont.get(), bot + name);
}

void ESP::Health(box_data box, C_BaseEntity * entity, Color colour, int alpha, int outline_alpha, int inline_alpha)
{
	const auto player_health = entity->GetHealth();
	auto health_colour = Color().hsv_to_rgb(player_health + 30, 1, 1);

	const auto bar_height = player_health * box.h / 100.0f,
		offset = box.h - bar_height;

	g_Render.FilledRect(box.x - 6, box.y - 1, 3, box.h + 2, Color(10, 10, 10, inline_alpha));
	g_Render.FilledRect(box.x - 5, box.y + offset, 2, bar_height + 2, health_colour(alpha));
	g_Render.OutlinedRect(box.x - 6, box.y - 1, 3, box.h + 2, Color(25, 25, 25, outline_alpha));

	if (player_health < 100)
	{
		g_Render.String(SPoint(box.x - 4, box.y + offset - 4), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 5, box.y + offset - 4), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 6, box.y + offset - 4), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));

		g_Render.String(SPoint(box.x - 4, box.y + offset - 3), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 5, box.y + offset - 3), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 6, box.y + offset - 3), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));

		g_Render.String(SPoint(box.x - 4, box.y + offset - 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 5, box.y + offset - 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
		g_Render.String(SPoint(box.x - 6, box.y + offset - 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));

		g_Render.String(SPoint(box.x - 5, box.y + offset - 3), CD3DFONT_CENTERED_X, Color(255, 255, 255, alpha), g_Fonts.pFontESPFont2.get(), std::to_string(entity->GetHealth()));
	}
}

void ESP::Ammo(box_data box, C_BaseEntity * entity, Color colour, int alpha, int outline_alpha, int inline_alpha)
{
	auto weapon = entity->GetActiveWeapon();

	if (!weapon)
		return;

	if (weapon->realWeapon())
		return;

	auto* weapon_data = g_pWeaponSys->GetWpnData(weapon->GetItemDefinitionIndex());

	if (weapon_data == nullptr)
		return;

	auto ammo_calc = std::clamp(weapon->GetAmmo() * box.w / weapon_data->iMaxClip1, 0, box.w);

	g_Render.FilledRect(box.x - 1, box.y + box.h + 3, box.w + 2, 3, Color(10, 10, 10, inline_alpha));
	g_Render.FilledRect(box.x, box.y + box.h + 4, ammo_calc + 1, 2, colour(alpha));
	g_Render.OutlinedRect(box.x - 1, box.y + box.h + 3, box.w + 2, 3, Color(25, 25, 25, outline_alpha));
}

void ESP::WeaponName(box_data box, C_BaseEntity * entity, Color colour, int alpha, int outline_alpha)
{
	auto weapon = entity->GetActiveWeapon();

	if (!weapon || weapon == nullptr)
		return;

	std::vector<std::tuple<std::string, Color, CD3DFont*>> stored_text;

	int y = box.y + box.h + ((g_Settings.Visualizations.bWeaponAmmo) && !weapon->realWeapon() ? 8 : 4);
	int i = 0;

	/* Weapon text */
	if (g_Settings.Visualizations.bWeaponName)
	{
		stored_text.emplace_back(std::tuple<std::string, Color, CD3DFont*>(weapon->getNameLowerCase(), g_Settings.Visualizations.cWeaponName(alpha), g_Fonts.pFontESPFont2.get()));
	}

	/* Weapon icon */
	if (g_Settings.Visualizations.bWeaponIcon)
	{
		stored_text.emplace_back(std::tuple<std::string, Color, CD3DFont*>(weapon->getIcon(), g_Settings.Visualizations.cWeaponIcon(alpha), g_Fonts.pFontIcons.get()));
	}

	/* Draw text */
	for (auto Text : stored_text)
	{
		g_Render.String(SPoint((box.x + box.w / 2) + 1, y + i * (std::get<2>(Text)->iHeight + 1) + 0), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2) + 1, y + i * (std::get<2>(Text)->iHeight + 1) + 1), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2) + 1, y + i * (std::get<2>(Text)->iHeight + 1) + 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));

		g_Render.String(SPoint((box.x + box.w / 2), y + i * (std::get<2>(Text)->iHeight + 1) + 0), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2), y + i * (std::get<2>(Text)->iHeight + 1) + 1), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2), y + i * (std::get<2>(Text)->iHeight + 1) + 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));

		g_Render.String(SPoint((box.x + box.w / 2) - 1, y + i * (std::get<2>(Text)->iHeight + 1) + 0), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2) - 1, y + i * (std::get<2>(Text)->iHeight + 1) + 1), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));
		g_Render.String(SPoint((box.x + box.w / 2) - 1, y + i * (std::get<2>(Text)->iHeight + 1) + 2), CD3DFONT_CENTERED_X, Color(25, 25, 25, outline_alpha), std::get<2>(Text), std::get<0>(Text));

		g_Render.String(SPoint(box.x + box.w / 2, y + i * (std::get<2>(Text)->iHeight + 1) + 1), CD3DFONT_CENTERED_X, std::get<1>(Text), std::get<2>(Text), std::get<0>(Text));
		i++;
	}
}

void ESP::Flags(box_data box, C_BaseEntity * entity, int alpha, int outline_alpha)
{
	std::vector<std::pair<std::string, Color>> info;
	int i = 0;

	/* Bomb */
	if (entity->GetVecOrigin().x == 0 && entity->GetVecOrigin().y == 0 && entity->GetVecOrigin().z == 0)
	{
		if (entity->GetClientClass()->ClassID == CC4)
		{
			info.push_back(std::pair<std::string, Color>("BOMB", Color(255, 20, 20, alpha)));
		}
	}

	if (entity->GetClientClass()->ClassID != CCSPlayer)
		return;

	/* Balance */
	if (g_Settings.Visualizations.bFlags[0])
		info.push_back(std::pair<std::string, Color>("$" + std::to_string(static_cast<int>(entity->GetBalance())), Color(125, 126, 77, alpha)));

	/* Armor */
	if (g_Settings.Visualizations.bFlags[1])
	{
		bool helmet = entity->HasHelmet();
		bool kevlar = entity->GetArmor() > 0;

		std::string text;

		if (helmet)
			text += "H";
		if (kevlar)
			text += "K";

		if (kevlar || helmet)
			info.push_back(std::pair<std::string, Color>(text, Color(255, 255, 255, alpha)));
	}

	/* Zoom */
	if (g_Settings.Visualizations.bFlags[2] && entity->GetScoped())
		info.push_back(std::pair<std::string, Color>("ZOOM", Color(113, 147, 162, alpha)));

	/* Flashed */
	if (g_Settings.Visualizations.bFlags[3] && (entity->SetFlashDuration() > 0))
		info.push_back(std::pair<std::string, Color>("FLASHED", Color(66, 203, 244, alpha)));

	/* Draw text */
	for (auto Text : info)
	{
		g_Render.String(SPoint(box.x + box.w + 4, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1)), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 4, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 1), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 4, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 2), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);

		g_Render.String(SPoint(box.x + box.w + 3, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1)), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 3, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 1), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 3, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 2), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);

		g_Render.String(SPoint(box.x + box.w + 2, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1)), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 2, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 1), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);
		g_Render.String(SPoint(box.x + box.w + 2, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 2), CD3DFONT_NONE, Color(25, 25, 25, outline_alpha), g_Fonts.pFontESPFont2.get(), Text.first);

		g_Render.String(SPoint(box.x + box.w + 3, box.y + i * (g_Fonts.pFontESPFont2->iHeight - 1) - 1), CD3DFONT_NONE, Text.second, g_Fonts.pFontESPFont2.get(), Text.first);
		i++;
	}
}

void ESP::Skeleton(C_BaseEntity * entity, Color colour, int alpha)
{
	studiohdr_t* model = g_MdlInfo->GetStudiomodel2(entity->GetModel());

	if (!model)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int i = 0; i < model->numbones; i++)
	{
		mstudiobone_t* pBone = model->GetBone(i);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = entity->GetBonePos(i);
			vParent = entity->GetBonePos(pBone->parent);

			int iChestBone = 6;
			Vector vBreastBone; /* New reference point for connecting many bones */
			Vector vUpperDirection = entity->GetBonePos(iChestBone + 1) - entity->GetBonePos(iChestBone); /* Direction vector from chest to neck */
			vBreastBone = entity->GetBonePos(iChestBone) + vUpperDirection / 2;
			Vector vDeltaChild = vChild - vBreastBone; /* Used to determine close bones to the reference point */
			Vector vDeltaParent = vParent - vBreastBone;

			if ((vDeltaParent.Size() < 9 && vDeltaChild.Size() < 9))
				vParent = vBreastBone;

			if (i == iChestBone - 1)
				vChild = vBreastBone;

			if (abs(vDeltaChild.z) < 5 && (vDeltaParent.Size() < 5 && vDeltaChild.Size() < 5) || i == iChestBone)
				continue;

			g_pDebugOverlay->ScreenPosition(vParent, sParent);
			g_pDebugOverlay->ScreenPosition(vChild, sChild);


			g_Render.Line(sParent[0], sParent[1], sChild[0], sChild[1], colour(alpha));

		}
	}
}

/* ========== Other ESP ========== */

void ESP::DroppedWeapons(C_BaseEntity * entity, box_data box)
{
	if (!g_Settings.Visualizations.bItems)
		return;

	auto * weapon = reinterpret_cast<C_BaseCombatWeapon*>(entity);

	if (!weapon)
		return;

	if (weapon->IsDormant())
		return;

	auto class_id = entity->GetClientClass()->ClassID;

	if (class_id == CBaseCSGrenadeProjectile || class_id == CMolotovProjectile || class_id == CDecoyProjectile || class_id == CSmokeGrenadeProjectile
		|| class_id == CSensorGrenadeProjectile || class_id == CBreachChargeProjectile)
		return;

	if (weapon && !(entity->GetVecOrigin().x == 0 && entity->GetVecOrigin().y == 0 && entity->GetVecOrigin().z == 0))
	{
		WeaponInfo_t* weapon_data = weapon->GetCSWpnData();

		if (!weapon_data)
			return;

		std::string weaponName = weapon->getNameLowerCase();

		int iItemAlpha = g_Settings.Visualizations.cItems.alpha;

		g_Render.String(SPoint((box.x + box.w / 2) + 1, box.y - 13), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) + 0, box.y - 13), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) - 1, box.y - 13), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);

		g_Render.String(SPoint((box.x + box.w / 2) + 1, box.y - 12), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) + 0, box.y - 12), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) - 1, box.y - 12), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);

		g_Render.String(SPoint((box.x + box.w / 2) + 1, box.y - 11), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) + 0, box.y - 11), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);
		g_Render.String(SPoint((box.x + box.w / 2) - 1, box.y - 11), CD3DFONT_CENTERED_X, Color(25, 25, 25, iItemAlpha), g_Fonts.pFontTahoma8.get(), weaponName);

		g_Render.String(SPoint(box.x + box.w / 2, box.y - 12), CD3DFONT_CENTERED_X, g_Settings.Visualizations.cItems, g_Fonts.pFontTahoma8.get(), weaponName);

		auto ammo_calc = std::clamp(weapon->GetAmmo() * box.w / weapon_data->iMaxClip1, 0, box.w);

		if (!weapon->realWeapon())
		{
			g_Render.FilledRect(box.x - 1, box.y + box.h + 2, box.w + 2, 4, Color(10, 10, 10, 180));
			g_Render.FilledRect(box.x, box.y + box.h + 3, ammo_calc, 2, Color(99, 150, 232, 255));
		}
	}
}

void ESP::Projectiles(C_BaseEntity * entity)
{
	if (!g_Settings.Visualizations.bProjectiles)
		return;

	const model_t* model = entity->GetModel();

	if (!model)
		return;

	if (entity->IsDormant())
		return;

	const studiohdr_t* hdr = g_MdlInfo->GetStudiomodel2(model);

	if (!hdr)
		return;

	const auto client_class = entity->GetClientClass();

	if (!client_class)
		return;

	std::string to_render = "ERROR";

	switch (client_class->ClassID)
	{
	case CMolotovProjectile:
		to_render = "MOLOTOV";
		break;
	case CSmokeGrenadeProjectile:
		to_render = "SMOKE";
		break;
	case CDecoyProjectile:
		to_render = "DECOY";
		break;
	case CBaseCSGrenadeProjectile:
		if (hdr->szName[16] == 's')
			to_render = "FLASHBANG";
		else
			to_render = "HE GRENADE";
		break;
	default:
		break;
	}

	Vector screen;

	if (Utils::WorldToScreen(entity->GetAbsOrigin(), screen))
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

		if (client_class->ClassID == CSmokeGrenadeProjectile)
		{
			auto time = g_pGlobalVars->intervalPerTick * (g_pGlobalVars->tickcount - entity->GetGrenadeSmokeTickBegin());

			if (!(18 - time < 0)) /// time > 18
			{
				SIZE size;
				g_Fonts.pFontTahoma8->GetTextExtent(to_render.c_str(), &size);
				int calcz = std::clamp((18 - time) *  float(size.cx) / 18, 0.f, float(size.cx));

				g_Render.FilledRect(screen.x - 3 - size.cy, screen.y + size.cy + 1, size.cx + 1, 4, Color(10, 10, 10, 180));
				g_Render.FilledRect(screen.x - 2 - size.cy, screen.y + size.cy + 2, calcz, 2, Color(66, 244, 146, 220));
			}
		}
	}
}

void ESP::Bomb(C_BaseEntity * entity)
{
	static auto sanitize = [](char * name) -> std::string {
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 20) {
			tmp.erase(20, (tmp.length() - 20));
			tmp.append("...");
		}

		return tmp;
	};

	if (!g_Settings.Visualizations.bBomb)
		return;

	auto * Bomb = reinterpret_cast<C_BaseCombatWeapon*>(entity);

	if (!Bomb)
		return;

	if (Bomb->IsDormant())
		return;

	auto class_id = entity->GetClientClass()->ClassID;

	if (!class_id == CC4 || !class_id == CPlantedC4)
		return;

	Vector screen;

	if (Utils::WorldToScreen(entity->GetAbsOrigin(), screen))
	{
		if (!(entity->GetVecOrigin().x == 0 && entity->GetVecOrigin().y == 0 && entity->GetVecOrigin().z == 0))
		{
			int iBomb = g_Settings.Visualizations.cBomb.alpha;

			if (class_id == CPlantedC4)
			{
				float blow = entity->GetC4Blow();

				blow -= g_pGlobalVars->intervalPerTick * g::pLocalEntity->GetTickBase();

				if (blow <= 0)
					blow = 0;

				Utils::Log("%", blow);

				std::string to_render = "BOMB: " + std::to_string(blow);

				float blowRatio = blow * 100 / 40;

				SIZE size;
				g_Fonts.pFontTahoma8->GetTextExtent(to_render.c_str(), &size);
				int calcz = std::clamp(78.f * blowRatio / 100.f, 0.f, 78.f);

				int bombRed = 255 - (blow * 2.55 + 30),
					bombGreen = bombRed * 2.55;

				g_Render.FilledRect(screen.x - 40, screen.y - 4, 80, 18, Color(10, 10, 10, 180));
				g_Render.FilledRect(screen.x - 39, screen.y - 3, calcz, 16, Color(bombRed, bombGreen, 10, 255));

				g_Render.String(screen.x - 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x - 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x - 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x, screen.y, CD3DFONT_CENTERED_X, g_Settings.Visualizations.cBomb, g_Fonts.pFontTahoma8.get(), to_render);
			}
			else
			{
				std::string to_render = "BOMB";

				g_Render.String(screen.x - 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y + 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x - 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y + 0, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x - 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x - 0, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);
				g_Render.String(screen.x + 1, screen.y - 1, CD3DFONT_CENTERED_X, Color(25, 25, 25, iBomb), g_Fonts.pFontTahoma8.get(), to_render);

				g_Render.String(screen.x, screen.y, CD3DFONT_CENTERED_X, g_Settings.Visualizations.cBomb, g_Fonts.pFontTahoma8.get(), to_render);

			}
		}
	}
}

void ESP::OffscreenESP(Color colour)
{
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	int screen_width, screen_height;
	g_pEngine->GetScreenSize(screen_width, screen_height);
	static bool pulse = true;
	static int  alpha = 150;
	const auto screen_center_x = screen_width / 2;
	const auto screen_center_y = screen_height / 2;
	const auto radius = 45;
	const auto size = 26;

	if (g::pLocalEntity == nullptr)
		return;

	if (!g::pLocalEntity->IsAlive())
		return;

	if (pulse)
	{
		alpha += 1;
		if (alpha == 255) pulse = false;
	}
	else
	{
		alpha -= 1;
		if (alpha == 150) pulse = true;
	}

	const auto screenTransform = [](Vector &origin, Vector &screen) -> bool
	{
		static std::uintptr_t pViewMatrix;
		if (!pViewMatrix)
		{
			pViewMatrix = static_cast<std::uintptr_t>(Utils::FindSignature("client_panorama.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));
			pViewMatrix += 3;
			pViewMatrix = *reinterpret_cast<std::uintptr_t*>(pViewMatrix);
			pViewMatrix += 176;
		}

		const VMatrix& w2sMatrix = *reinterpret_cast<VMatrix*>(pViewMatrix);
		screen.x = w2sMatrix.m[0][0] * origin.x + w2sMatrix.m[0][1] * origin.y + w2sMatrix.m[0][2] * origin.z + w2sMatrix.m[0][3];
		screen.y = w2sMatrix.m[1][0] * origin.x + w2sMatrix.m[1][1] * origin.y + w2sMatrix.m[1][2] * origin.z + w2sMatrix.m[1][3];
		screen.z = 0.0f;

		float w = w2sMatrix.m[3][0] * origin.x + w2sMatrix.m[3][1] * origin.y + w2sMatrix.m[3][2] * origin.z + w2sMatrix.m[3][3];

		if (w < 0.001f)
		{
			screen.x *= 100000;
			screen.y *= 100000;
			return true;
		}

		float invw = 1.f / w;
		screen.x *= invw;
		screen.y *= invw;

		return false;
	};

	for (auto i = 1; i < g_pEntityList->GetHighestEntityIndex(); i++)
	{
		auto* entity = g_pEntityList->GetClientEntity(i);

		if (!entity)
			return;

		if (!entity->GetClientClass()->ClassID == CCSPlayer)
			continue;

		if (entity->IsDormant())
			continue;

		if (!g::pLocalEntity)
			continue;

		if (entity->GetTeam() == g::pLocalEntity->GetTeam())
			continue;

		Vector screen_point;
		screenTransform(entity->GetVecOrigin(), screen_point);

		if (!screenTransform(entity->GetVecOrigin(), screen_point))
			continue;

		if (screen_point.x < 0 || screen_point.y < 0 || screen_point.x > screen_width || screen_point.y > screen_height)
		{
			auto angle = Vector();
			g_Math.VectorAngles_2(
				Vector(screen_center_x - screen_point.x, screen_center_y - screen_point.y, 0), angle);

			const auto angle_yaw_rad = g_Math.deg_2_rad(angle.y + 180.f);

			const auto new_point_x = screen_center_x +
				radius * 8 * cosf(angle_yaw_rad);
			const auto new_point_y = screen_center_y +
				radius * 8 * sinf(angle_yaw_rad);

			std::array<Vector2D, 3> points{
				Vector2D(new_point_x - size / 2, new_point_y - size / 2),
				Vector2D(new_point_x + size / 4, new_point_y),
				Vector2D(new_point_x - size / 2, new_point_y + size / 2)
			};

			g_Math.RotateTriangle(points, angle.y + 180.f);
			g_Render.TriangleFilled(SPoint(points.at(0).x, points.at(0).y), SPoint(points.at(1).x, points.at(1).y), SPoint(points.at(2).x, points.at(2).y), colour(alpha));
			g_Render.Triangle(SPoint(points.at(0).x, points.at(0).y), SPoint(points.at(1).x, points.at(1).y), SPoint(points.at(2).x, points.at(2).y), colour(alpha));
		}
	}
}

void ESP::GrenadePrediction()
{
	/* Check if we are connected */
	if (!g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		return;

	/* Check if we are alive */
	if (!g::pLocalEntity->IsAlive())
		return;

	/* If the grenade prediction isn't enabled, return */
	if (!g_Settings.Visualizations.bGrenadePrediction)
		return;

	/* Pointer to the local player */
	auto p_local = g::pLocalEntity;

	/* If the local player is null, don't do anything */
	if (!p_local)
		return;

	/* Pointer to the local player active weapon */
	auto weapon = p_local->GetActiveWeapon();

	/* If we enable the grenade prediction */
	if (g_Settings.Visualizations.bGrenadePrediction) 
	{
		const float TIMEALIVE = 5.f;
		const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.4f;

		float fStep = 0.1f;
		float fGravity = 800.0f / 8.f;

		Vector vPos;
		Vector vStart;
		Vector vThrow;
		Vector vThrow2;
		int iCollisions = 0;

		Vector vViewAngles;
		g_pEngine->GetViewAngles(vViewAngles);

		vThrow[0] = vViewAngles[0];
		vThrow[1] = vViewAngles[1];
		vThrow[2] = vViewAngles[2];

		if (vThrow[0] < 0)
			vThrow[0] = -10 + vThrow[0] * ((90 - 10) / 90.0);
		else
			vThrow[0] = -10 + vThrow[0] * ((90 + 10) / 90.0);

		float fVel = (90 - vThrow[0]) * 4;
		if (fVel > 500)
			fVel = 500;

		g_Math.AngleVectors(vThrow, &vThrow2);

		Vector vEye = p_local->GetEyePosition();
		vStart[0] = vEye[0] + vThrow2[0] * 16;
		vStart[1] = vEye[1] + vThrow2[1] * 16;
		vStart[2] = vEye[2] + vThrow2[2] * 16;

		vThrow2[0] = (vThrow2[0] * fVel) + p_local->GetVelocity()[0];
		vThrow2[1] = (vThrow2[1] * fVel) + p_local->GetVelocity()[1];
		vThrow2[2] = (vThrow2[2] * fVel) + p_local->GetVelocity()[2];

		for (float fTime = 0.0f; fTime < TIMEALIVE; fTime += fStep) {

			vPos = vStart + vThrow2 * fStep;

			Ray_t ray;
			trace_t tr;

			CTraceFilter loc;
			loc.pSkip = p_local;

			ray.Init(vStart, vPos);
			g_pEngineTrace->TraceRay(ray, MASK_SOLID, &loc, &tr);

			if (tr.DidHit()) {
				vThrow2 = tr.plane.normal * -2.0f *DotProduct(vThrow2, tr.plane.normal) + vThrow2;
				vThrow2 *= GRENADE_COEFFICIENT_OF_RESTITUTION;

				iCollisions++;

				if (iCollisions > 2)
					break;

				vPos = vStart + vThrow2 * tr.fraction*fStep;

				fTime += (fStep * (1 - tr.fraction));
			}

			Vector vOutStart, vOutEnd;

			/* Only draw if we are holding our mouse left button */
			if (g_pInputSystem->IsButtonDown(MOUSE_LEFT))
			{
				if (weapon->isGrenade())
				{
					if (Utils::WorldToScreen(vStart, vOutStart), Utils::WorldToScreen(vPos, vOutEnd)) 
					{
						g_Render.Line(vOutStart.x - 0.5, vOutStart.y - 0.5, vOutEnd.x - 0.5, vOutEnd.y - 0.5, Color(57, 57, 57));
						g_Render.Line(vOutStart.x, vOutStart.y, vOutEnd.x, vOutEnd.y, g_Settings.Visualizations.cGrenadePred);
					}

					vStart = vPos;
					vThrow2.z -= fGravity * tr.fraction*fStep;
				}
			}
		}
	}
}

void ESP::TraceCircle(Vector position, Color color)
{
	Vector prev_scr_pos{ -1, -1, -1 };
	Vector scr_pos;

	float step = M_PI * 2.0 / 150;

	float rad = 150.f;

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

		g_pEngineTrace->TraceRay(ray, MASK_ALL, &filter, &trace);

		if (Utils::WorldToScreen(trace.endpos, scr_pos))
		{
			if (prev_scr_pos != Vector{ -1, -1, -1 })
			{
				int hue = RAD2DEG(rotation) + hue_offset;

				g_Render.Line(prev_scr_pos.x, prev_scr_pos.y, scr_pos.x, scr_pos.y, color);
			}
			prev_scr_pos = scr_pos;
		}
	}
	hue_offset -= 0.5;
}