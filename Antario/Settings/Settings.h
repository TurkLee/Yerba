#pragma once
#include "..\GUI\GUI.h"
#include <filesystem>
#include "..\Utils\GlobalVars.h"
#include <fstream>
#include <iostream>
#include <functional>
#include <ShlObj.h>
#include "..\Hooks\Hooks.h"

using namespace ui;
namespace fs = std::experimental::filesystem;

class Settings : public MenuMain
{
private:

	fs::path fsPath{};
	std::vector<std::string> vecFileNames{};

public:
	/* All our settings variables will be here */

	int iAlpha;

	bool bCheatActive = true;
	bool bMenuOpened = false;
	bool bMenuFullFade = false;

	class Legit {
	public:
		/* Other */

		/// Checkboxes
		bool bBacktrack;
	};

	class Visualizations {
	public:
		/* Player ESP */

		/// Colors
		Color cBox = { 255, 255, 255, 255 };
		Color cName = { 255, 255, 255, 255 };
		Color cWeaponIcon = { 255, 255, 255, 255 };
		Color cWeaponName = { 255, 255, 255, 255 };
		Color cWeaponAmmo = { 66, 182, 244, 255 };
		Color cSound = { 233, 88, 243, 255 };
		Color cOffscreenESP = { 255, 255, 255, 255 };
		Color cGlow = { 255, 255, 255, 255 };
		Color cSkeleton = { 255, 255, 255, 255 };

		/// Checkboxes 
		bool bBox;
		bool bDynamic;
		bool bName;
		bool bHealth;
		bool bWeaponIcon;
		bool bWeaponName;
		bool bWeaponAmmo;
		bool bSound;
		bool bOffscreenESP;
		bool bGlow;
		bool bSkeleton;
		bool bDormant;

		/// Multibox
		bool bFlags[5];

		/* Colored models */

		/// Colors
		Color cBackTrack = { 255, 255, 255, 255 };
		Color cChams = { 244, 241, 66, 255 };
		Color cChamsZ = { 229, 66, 244, 255 };
		Color cLocalChams = { 66, 244, 137, 255 };

		/// Checkboxes
		bool bChams;
		bool bChamsZ;
		bool bLocalChams;
		bool bBlend;
		bool bChamsWhileScoped;
		bool bBackTrackChams;

		/// Comboboxes
		int iTransparency = 100;
		int iMaterial;
		int iMaterialLocal;

		/* Removals */

		/// Checkboxes
		bool bRemoveSmoke;
		bool bRemoveRecoil;
		bool bRemoveScope;
		bool bDisablePostProcessing;

		/// Sliders
		int iFlashbangEffectAlpha = 255;

		/* World */

		/// Colors
		Color cItems = { 255, 255, 255, 255 };
		Color cProjectiles = { 255, 255, 255, 255 };
		Color cBomb = { 255, 255, 255, 255 };

		/// Checkboxes 
		bool bItems;
		bool bProjectiles;
		bool bBomb;
		bool bNightmode;
		bool bFullBright;

		/* Other */

		/// Sliders
		int iTracerTime;
		int iThirdpersonFOV;

		/// Modes
		int iThirdpersonKeybind = 2;

		/// Keybinds
		ButtonCode_t btThirdperonKeybind;

		/// Colors
		Color cLocalTracer = { 233, 88, 243, 255 };
		Color cLocalHurt = { 244, 158, 66, 255 };
		Color cEnemyTracer = { 0, 255, 255, 255 };
		Color cGrenadePred = { 255, 255, 255, 255 };

		/// Checkboxes
		bool bBulletTracer;
		bool bHitmarker;
		bool bGrenadePrediction;
		bool bForceCrosshair;
		bool bHitmarkerSound;
		bool bSpecThirdperson;
	};

	class Miscellaneous {
	public:
		/* Miscellaneous */
		
		/// Checkboxes
		bool bEventLogs[2];
		bool bBhopEnabled;
		bool bAutoStrafer;
		bool bInfiniteDuck;
		bool bWeaponSwitchSpammer;
		bool bRevealCompetitiveRanks;
		bool bChatSpammer;
		bool bRadioSpammer;
		bool bClanTagSpammer;

		/// Sliders
		int iOverrideFOV;
		int iForceAspectRatio;

		/* Settings */

		/// Colors
		Color cMenuColor = { 128, 255, 0, 255 };

		/// Checkboxes
		bool bWatermark = true;
	};

	Legit Legit;
	Visualizations Visualizations;
	Miscellaneous Miscellaneous;
};

extern Settings g_Settings;