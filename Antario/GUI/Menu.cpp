#include "GUI.h"
#include "..\Settings\Settings.h"
#include "..\Hooks\Hooks.h"

void Detach() { g_Settings.bCheatActive = false; }

ui::MenuMain nMenu;

void MenuMain::Initialize()
{
	auto mainWindow = std::make_shared<Window>("yerba", SSize(550, 420), g_Fonts.pFontTahoma8, g_Fonts.pFontTahoma10);
	{
		/// Aimbot
		auto Tab1 = std::make_shared<Tab>("H", 2, mainWindow);
		{
			auto Aimbot = Tab1->AddSection("Aimbot", 1.5f);
			{

			}

			auto Other = Tab1->AddSection("Other", 1.5f);
			{
				Other->AddCheckBox("Backtrack", &g_Settings.Legit.bBacktrack);
			}

		} mainWindow->AddChild(Tab1);

		/// Visualizations
		auto Tab2 = std::make_shared<Tab>("D", 2, mainWindow);
		{
			auto PlayerESP = Tab2->AddSection("Player ESP", .9f);
			{
				PlayerESP->AddColor("Box color", &g_Settings.Visualizations.cBox);
				PlayerESP->AddCheckBox("Bounding box", &g_Settings.Visualizations.bBox);
				PlayerESP->AddCheckBox("Dynamic box", &g_Settings.Visualizations.bDynamic);
				PlayerESP->AddColor("Name color", &g_Settings.Visualizations.cName);
				PlayerESP->AddCheckBox("Name", &g_Settings.Visualizations.bName);
				PlayerESP->AddCheckBox("Health bar", &g_Settings.Visualizations.bHealth);
				PlayerESP->AddText("Flags");
				PlayerESP->AddMulti("Flags", g_Settings.Visualizations.bFlags, std::vector<std::string>{
					"Balance",
					"Armor",
					"Zoom",
					"Flashed"
					//"Bomb"
				});
				PlayerESP->AddColor("Weapon icon color", &g_Settings.Visualizations.cWeaponIcon);
				PlayerESP->AddCheckBox("Weapon icon", &g_Settings.Visualizations.bWeaponIcon);
				PlayerESP->AddColor("Weapon text color", &g_Settings.Visualizations.cWeaponName);
				PlayerESP->AddCheckBox("Weapon text", &g_Settings.Visualizations.bWeaponName);
				PlayerESP->AddColor("Ammo bar color", &g_Settings.Visualizations.cWeaponAmmo);
				PlayerESP->AddCheckBox("Ammo bar", &g_Settings.Visualizations.bWeaponAmmo);
				PlayerESP->AddCheckBox("Dormant", &g_Settings.Visualizations.bDormant);
				PlayerESP->AddColor("Out of FOV arrow color", &g_Settings.Visualizations.cOffscreenESP);
				PlayerESP->AddCheckBox("Out of FOV arrow", &g_Settings.Visualizations.bOffscreenESP);
				PlayerESP->AddColor("Glow color", &g_Settings.Visualizations.cGlow);
				PlayerESP->AddCheckBox("Glow", &g_Settings.Visualizations.bGlow);
				PlayerESP->AddColor("Visualize sounds color", &g_Settings.Visualizations.cSound);
				PlayerESP->AddCheckBox("Visualize sounds", &g_Settings.Visualizations.bSound);
				//PlayerESP->AddColor("Skeleton color", &g_Settings.Visualizations.cSkeleton);
				//PlayerESP->AddCheckBox("Skeleton", &g_Settings.Visualizations.bSkeleton);
			}

			auto ColoredModels = Tab2->AddSection("Colored models", .6f);
			{
				ColoredModels->AddText("Player material");
				ColoredModels->AddCombo("Player material", &g_Settings.Visualizations.iMaterial, std::vector<std::string>{
					"Flat",
					"Colored",
					"Metallic"
				});
				ColoredModels->AddColor("Player color", &g_Settings.Visualizations.cChams);
				ColoredModels->AddCheckBox("Player", &g_Settings.Visualizations.bChams);
				ColoredModels->AddColor("Player behind wall color", &g_Settings.Visualizations.cChamsZ);
				ColoredModels->AddCheckBox("Player behind wall", &g_Settings.Visualizations.bChamsZ);
				//ColoredModels->AddColor("Backtrack", &g_Settings.Visualizations.cBackTrack);
				//ColoredModels->AddCheckBox("Backtrack", &g_Settings.Visualizations.bBackTrackChams);
				ColoredModels->AddText("Local player material");
				ColoredModels->AddCombo("Local player material", &g_Settings.Visualizations.iMaterialLocal, std::vector<std::string>{
					"Flat",
					"Colored",
					"Metallic"
				});
				ColoredModels->AddColor("Local player color", &g_Settings.Visualizations.cLocalChams);
				ColoredModels->AddCheckBox("Local player", &g_Settings.Visualizations.bLocalChams);

				ColoredModels->AddText("Self transparency");
				ColoredModels->AddSlider("Self transparency", "%", &g_Settings.Visualizations.iTransparency, 0, 100);
				ColoredModels->AddCheckBox("Blend when scoped", &g_Settings.Visualizations.bBlend);
				ColoredModels->AddCheckBox("Remove chams when scoped", &g_Settings.Visualizations.bChamsWhileScoped);
			}

			auto Removals = Tab2->AddSection("Removals", .44f);
			{
				Removals->AddText("Flashbang effect alpha");
				Removals->AddSlider("Flashbang effect alpha", "", &g_Settings.Visualizations.iFlashbangEffectAlpha, 0, 255);
				Removals->AddCheckBox("Remove smoke grenades", &g_Settings.Visualizations.bRemoveSmoke);
				Removals->AddCheckBox("Remove visual recoil", &g_Settings.Visualizations.bRemoveRecoil);
				Removals->AddCheckBox("Remove scope overlay", &g_Settings.Visualizations.bRemoveScope);
				Removals->AddCheckBox("Disable post processing", &g_Settings.Visualizations.bDisablePostProcessing);
			}

			auto World = Tab2->AddSection("World", .39f);
			{
				World->AddColor("Weapons color", &g_Settings.Visualizations.cItems);
				World->AddCheckBox("Weapons", &g_Settings.Visualizations.bItems);
				World->AddColor("Grenades color", &g_Settings.Visualizations.cProjectiles);
				World->AddCheckBox("Grenades", &g_Settings.Visualizations.bProjectiles);
				World->AddColor("Bomb color", &g_Settings.Visualizations.cBomb);
				World->AddCheckBox("Bomb", &g_Settings.Visualizations.bBomb);
				World->AddCheckBox("Night mode", &g_Settings.Visualizations.bNightmode);
				World->AddCheckBox("Full bright", &g_Settings.Visualizations.bFullBright);
			}

			auto Other = Tab2->AddSection("Other", .673f);
			{
				Other->AddCheckBox("Bullet tracers", &g_Settings.Visualizations.bBulletTracer);
				Other->AddColor("Local bullet color", &g_Settings.Visualizations.cLocalTracer);
				Other->AddText("Bullet tracer miss");
				Other->AddColor("Local hurt color", &g_Settings.Visualizations.cLocalHurt);
				Other->AddText("Bullet tracer hit");
				Other->AddColor("Enemy color", &g_Settings.Visualizations.cEnemyTracer);
				Other->AddText("Enemy bullet tracer");
				Other->AddText("Bullet tracers time");
				Other->AddSlider("Bullet tracers time", "s", &g_Settings.Visualizations.iTracerTime, 1, 10);
				Other->AddCheckBox("Crosshair", &g_Settings.Visualizations.bForceCrosshair);
				Other->AddColor("Grenade trajectory", &g_Settings.Visualizations.cGrenadePred);
				Other->AddCheckBox("Grenade trajectory", &g_Settings.Visualizations.bGrenadePrediction);
				Other->AddCheckBox("Hit marker", &g_Settings.Visualizations.bHitmarker);
				Other->AddCheckBox("Hit marker sound", &g_Settings.Visualizations.bHitmarkerSound);
				Other->AddText("Force third person (alive)");
				Other->AddKeyBind("Third person", &g_Settings.Visualizations.btThirdperonKeybind, &g_Settings.Visualizations.iThirdpersonKeybind);
				Other->AddCheckBox("Force third person (dead)", &g_Settings.Visualizations.bSpecThirdperson);
				Other->AddText("Force third person FOV");
				Other->AddSlider("Force third person FOV", "", &g_Settings.Visualizations.iThirdpersonFOV, 0, 50);
			}
		} mainWindow->AddChild(Tab2);

		/// Miscellaneous
		auto Tab3 = std::make_shared<Tab>("G", 2, mainWindow);
		{
			auto Miscellaneous = Tab3->AddSection("Miscellaneous", 1.5f);
			{
				Miscellaneous->AddText("Override FOV");
				Miscellaneous->AddSlider("Override FOV", "", &g_Settings.Miscellaneous.iOverrideFOV, 0, 50);
				Miscellaneous->AddText("Force aspect ratio");
				Miscellaneous->AddSlider("Force aspect ratio", "", &g_Settings.Miscellaneous.iForceAspectRatio, 0, 10);
				Miscellaneous->AddCheckBox("Bunny hop", &g_Settings.Miscellaneous.bBhopEnabled);
				Miscellaneous->AddCheckBox("Air strafe", &g_Settings.Miscellaneous.bAutoStrafer);
				Miscellaneous->AddCheckBox("Infinite duck", &g_Settings.Miscellaneous.bInfiniteDuck);
				Miscellaneous->AddCheckBox("Chat spammer", &g_Settings.Miscellaneous.bChatSpammer);
				Miscellaneous->AddCheckBox("Radio spammer", &g_Settings.Miscellaneous.bRadioSpammer);
				Miscellaneous->AddCheckBox("Clan tag spammer", &g_Settings.Miscellaneous.bClanTagSpammer);
				Miscellaneous->AddCheckBox("Weapon switch spammer", &g_Settings.Miscellaneous.bWeaponSwitchSpammer);
				Miscellaneous->AddCheckBox("Reveal competitive ranks", &g_Settings.Miscellaneous.bRevealCompetitiveRanks);
				Miscellaneous->AddText("Event logs");
				Miscellaneous->AddMulti("Event logs", g_Settings.Miscellaneous.bEventLogs, std::vector<std::string>{
					"Damage",
					"Harmed"
				});
			}

			auto Settings = Tab3->AddSection("Settings", 1.5f);
			{
				Settings->AddColor("Menu color", &g_Settings.Miscellaneous.cMenuColor);
				Settings->AddText("Menu color");
				Settings->AddCheckBox("Watermark", &g_Settings.Miscellaneous.bWatermark);
				Settings->AddButton("Unload", Detach);
			}

		} mainWindow->AddChild(Tab3);
	}
	this->vecChildren.push_back(mainWindow);

	/* Create our mouse cursor (one instance only) */
	mouseCursor = std::make_unique<MouseCursor>();

	/* Do the first init run through all of the objects */
	for (auto& it : vecChildren)
		it->Initialize();
}