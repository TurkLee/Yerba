#pragma once
#include "Definitions.h"
#include "IClientUnknown.h"
#include "IClientEntityList.h"
#include "..\Utils\Utils.h"
#include "..\Utils\NetvarManager.h"
#include "Studio.h"

// class predefinition
class C_BaseCombatWeapon;

class C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable
{
private:
	template <typename T>
	T& read(uintptr_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}

	template <typename T>
	void write(uintptr_t offset, T data)
	{
		*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset) = data;
	}

public:
	NETVAR("DT_CSPlayer", "m_iHealth", GetHealth, int);
	NETVAR("DT_CSPlayer", "m_fFlags", GetFlags, int);
	NETVAR("DT_CSPlayer", "m_iTeamNum", GetTeam, int);
	NETVAR("DT_BaseEntity", "m_vecMins", GetOBBMins, Vector);
	NETVAR("DT_BaseEntity", "m_vecMaxs", GetOBBMaxs, Vector);
	NETVAR("DT_CSPlayer", "m_angEyeAngles", GetEyeAngles, Vector);
	NETVAR("DT_CSPlayer", "m_flLowerBodyYawTarget", GetLowerBodyYaw, float);
	NETVAR("DT_BaseAnimating", "m_flCycle", GetCycle, float);
	NETVAR("DT_BaseAnimating", "m_nSequence", GetSequence, int);
	NETVAR("DT_CSPlayer", "m_flSimulationTime", GetSimulationTime, float);
	NETVAR("DT_BasePlayer", "m_vecOrigin", GetVecOrigin, Vector);
	NETVAR("DT_BasePlayer", "m_vecVelocity[0]", GetVelocity, Vector);
	NETVAR("DT_CSPlayer", "m_ArmorValue", GetArmor, int);
	NETVAR("DT_CSPlayer", "m_bHasHelmet", HasHelmet, bool);
	NETVAR("DT_CSPlayer", "m_nTickBase", GetTickBase, int);
	NETVAR("DT_CSPlayer", "m_bGunGameImmunity", GetIsSpawnProtected, bool);
	NETVAR("DT_CSPlayer", "m_bIsScoped", GetScoped, bool);
	NETVAR("DT_CSPlayer", "m_nMoveType", GetMoveType, int);
	NETVAR("DT_CSPlayer", "m_iAccount", GetBalance, int);
	NETVAR("DT_BasePlayer", "m_vecViewOffset[0]", GetViewOffset, Vector);
	NETVAR("DT_BaseViewModel", "m_hOwner", GetOwnerHandle, HANDLE);
	NETVAR("DT_CSPlayer", "m_bIsPlayerGhost", GetIsPlayerGhost, bool);
	NETVAR("DT_BasePlayer", "m_aimPunchAngle", GetAimPunch, Vector);
	NETVAR("DT_BasePlayer", "m_viewPunchAngle", GetViewPunch, Vector);
	NETVAR("DT_BasePlayer", "m_nHitboxSet", GetGitboxSet, int);
	NETVAR("DT_CSPlayer", "m_flDuckAmount", GetCrouchAmount, float);
	NETVAR("DT_CSPlayer", "m_flNextAttack", GetNextAttack, float);
	NETVAR("DT_SmokeGrenadeProjectile", "m_nSmokeEffectTickBegin", GetGrenadeSmokeTickBegin, int);
	NETVAR("DT_HEGrenade", "m_flDamage", GetHeDamage, float);
	NETVAR("DT_CSPlayer", "m_iShotsFired", GetShotsFired, int);
	NETVAR("DT_CSPlayer", "m_flFlashDuration", SetFlashDuration, float);
	NETVAR("DT_CSPlayer", "m_flFlashMaxAlpha", SetFlashAlpha, float);
	NETVAR("DT_CSPlayer", "m_iObserverMode", SetObserverMode, int);
	NETVAR("DT_CSPlayer", "m_lifeState", GetLifeState, int);
	NETVAR("DT_BaseEntity", "m_nFallbackPaintKit", fallbackPaintKit, int);
	NETVAR("DT_CSPlayer", "m_bIsDefusing", isDefusing, bool);
	NETVAR("DT_CSPlayer", "m_bHasDefuser", hasDefuser, bool);
	NETVAR("DT_PlantedC4", "m_flC4Blow", GetC4Blow, float);
	NETVAR("DT_PlantedC4", "m_flDefuseCountDown", GetDefuseCountDown, int);
	NETVAR("DT_BaseCombatWeapon", "m_iItemDefinitionIndex", GetItemDefinitionIndex, int);

	PlayerInfo_t GetPlayerInfo()
	{
		PlayerInfo_t info;
		g_pEngine->GetPlayerInfo(EntIndex(), &info);

		return info;
	}

	Vector& GetAbsOrigin()
	{
		return Utils::CallVTable<Vector&(__thiscall*)(void*)>(this, 10)(this);
	}

	int C_BaseEntity::GetIndex()
	{
		return *reinterpret_cast<int*>((uintptr_t)this + 0x64);
	}

	void C_BaseEntity::SetOrigin(Vector origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector &origin);
		static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");
	
		SetAbsOrigin(this, origin);
	}

	void C_BaseEntity::SetAngles(const Vector &angles)
	{
		using SetAbsAnglesFn = void(__thiscall*)(void*, const Vector &angles);
		static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

		SetAbsAngles(this, angles);
	}

	bool IsAlive()
	{
		return this->GetHealth() > 0;
	}

	Vector GetEyePosition()
	{
		return this->GetVecOrigin() + this->GetViewOffset();
	}

	matrix3x4_t& get_rgflcoordinateframe()
	{
		static auto _m_rgflCoordinateFrame = sdk::util::getNetVar(sdk::util::fnv::hash("DT_CSPlayer"), sdk::util::fnv::hash("m_CollisionGroup")) - 0x30;
		return *reinterpret_cast<matrix3x4_t*>(reinterpret_cast<uintptr_t>(this) + _m_rgflCoordinateFrame);
	}

	C_BaseCombatWeapon* GetMyWeapons()
	{
		auto get_weapons = read<DWORD>(sdk::util::getNetVar(sdk::util::fnv::hash("DT_CSPlayer"), sdk::util::fnv::hash("m_hMyWeapons"))) & 0xFFF;
		return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntity(get_weapons));
	}

	C_BaseCombatWeapon* GetActiveWeapon()
	{
		auto active_weapon = read<DWORD>(sdk::util::getNetVar(sdk::util::fnv::hash("DT_CSPlayer"), sdk::util::fnv::hash("m_hActiveWeapon"))) & 0xFFF;
		return reinterpret_cast<C_BaseCombatWeapon*>(g_pEntityList->GetClientEntity(active_weapon));
	}

	Vector GetBonePos(int i)
	{
		matrix3x4_t boneMatrix[128];
		if (this->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, static_cast<float>(GetTickCount64())))
		{
			return Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
		}
		return Vector(0, 0, 0);
	}
};

class C_BaseCombatWeapon : public C_BaseEntity
{
private:
	template <typename T>
	T& read(uintptr_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset);
	}

	template <typename T>
	void write(uintptr_t offset, T data)
	{
		*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + offset) = data;
	}
public:
	NETVAR("DT_BaseCombatWeapon", "m_iItemDefinitionIndex", GetItemDefinitionIndex, short);
	NETVAR("DT_BaseCombatWeapon", "m_flNextPrimaryAttack", GetNextPrimaryAttack, float);
	NETVAR("DT_BaseCombatWeapon", "m_iClip1", GetAmmo, int);

	WeaponInfo_t* GetCSWpnData()
	{
		return Utils::CallVTable< WeaponInfo_t*(__thiscall*)(void*) >(this, 448)(this);
	}

	bool realWeapon()
	{
		short idx = GetItemDefinitionIndex();

		return (idx == WEAPON_C4 || idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
			|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET || idx == WEAPON_KNIFE_PUSH
			|| idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL || idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE
			|| idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE || idx == WEAPON_TAGRENADE || idx == WEAPON_FISTS
			|| idx == WEAPON_HEALTHSHOT || idx == WEAPON_BREACHCHARGE || idx == WEAPON_TABLET || idx == WEAPON_MELEE || idx == WEAPON_AXE || idx == WEAPON_HAMMER
			|| idx == WEAPON_SPANNER || idx == WEAPON_KNIFE_GHOST || idx == WEAPON_FIREBOMB || idx == WEAPON_DIVERSION || idx == WEAPON_FRAG_GRENADE || idx == WEAPON_KNIFE_URSUS
			|| idx == WEAPON_KNIFE_GYPSY_JACKKNIFE || idx == WEAPON_KNIFE_STILETTO || idx == WEAPON_KNIFE_WIDOWMAKER);
	}

	bool isGrenade()
	{
		short idx = GetItemDefinitionIndex();

		return (idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE
			|| idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY 
			|| idx == WEAPON_INCGRENADE || idx == WEAPON_TAGRENADE || idx == WEAPON_FIREBOMB || idx == WEAPON_FRAG_GRENADE);
	}

	char * getIcon()
	{
		switch (GetItemDefinitionIndex())
		{
		case WEAPON_KNIFE_BAYONET:
			return "1";
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
			return "7";
		case WEAPON_KNIFE_BUTTERFLY:
			return "8";
		case WEAPON_KNIFE:
			return "]";
		case WEAPON_KNIFE_FALCHION:
			return "0";
		case WEAPON_KNIFE_FLIP:
			return "2";
		case WEAPON_KNIFE_GUT:
			return "3";
		case WEAPON_KNIFE_KARAMBIT:
			return "4";
		case WEAPON_KNIFE_M9_BAYONET:
			return "5";
		case WEAPON_KNIFE_T:
			return "[";
		case WEAPON_KNIFE_TACTICAL:
			return "6";
		case WEAPON_KNIFE_PUSH:
			return "]";
		case WEAPON_DEAGLE:
			return "A";
		case WEAPON_ELITE:
			return "B";
		case WEAPON_FIVESEVEN:
			return "C";
		case WEAPON_GLOCK:
			return "D";
		case WEAPON_HKP2000:
			return "E";
		case WEAPON_P250:
			return "F";
		case WEAPON_USP_SILENCER:
			return "G";
		case WEAPON_TEC9:
			return "H";
		case WEAPON_REVOLVER:
			return "J";
		case WEAPON_MAC10:
			return "K";
		case WEAPON_UMP45:
			return "L";
		case WEAPON_BIZON:
			return "M";
		case WEAPON_MP7:
			return "N";
		case WEAPON_MP9:
			return "O";
		case WEAPON_P90:
			return "P";
		case WEAPON_GALILAR:
			return "Q";
		case WEAPON_FAMAS:
			return "R";
		case WEAPON_M4A1_SILENCER:
			return "S";
		case WEAPON_M4A1:
			return "T";
		case WEAPON_AUG:
			return "U";
		case WEAPON_SG556:
			return "V";
		case WEAPON_AK47:
			return "W";
		case WEAPON_G3SG1:
			return "X";
		case WEAPON_SCAR20:
			return "Y";
		case WEAPON_AWP:
			return "Z";
		case WEAPON_SSG08:
			return "a";
		case WEAPON_XM1014:
			return "b";
		case WEAPON_SAWEDOFF:
			return "c";
		case WEAPON_MAG7:
			return "d";
		case WEAPON_NOVA:
			return "e";
		case WEAPON_NEGEV:
			return "f";
		case WEAPON_M249:
			return "g";
		case WEAPON_TASER:
			return "h";
		case WEAPON_FLASHBANG:
			return "i";
		case WEAPON_HEGRENADE:
			return "j";
		case WEAPON_SMOKEGRENADE:
			return "k";
		case WEAPON_MOLOTOV:
			return "l";
		case WEAPON_DECOY:
			return "m";
		case WEAPON_INCGRENADE:
			return "n";
		case WEAPON_C4:
			return "o";
		case WEAPON_CZ75A:
			return "I";
		case WEAPON_KNIFE_URSUS:
			return "]";
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
			return "]";
		case WEAPON_KNIFE_STILETTO:
			return "]";
		case WEAPON_KNIFE_WIDOWMAKER:
			return "]";
		default:
			return "  ";
		}
	}

	char * getNameLowerCase()
	{
		switch (GetItemDefinitionIndex())
		{
		case WEAPON_KNIFE_BAYONET:
			return "BAYONET";
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
			return "BOWIE";
		case WEAPON_KNIFE_BUTTERFLY:
			return "BUTTERFLY";
		case WEAPON_KNIFE:
			return "KNIFE";
		case WEAPON_KNIFE_FALCHION:
			return "FALCHION";
		case WEAPON_KNIFE_FLIP:
			return "FLIP";
		case WEAPON_KNIFE_GUT:
			return "GUT";
		case WEAPON_KNIFE_KARAMBIT:
			return "KARAMBIT";
		case WEAPON_KNIFE_M9_BAYONET:
			return "M9 BAYONET";
		case WEAPON_KNIFE_T:
			return "KNIFE";
		case WEAPON_KNIFE_TACTICAL:
			return "HUNTSMAN";
		case WEAPON_KNIFE_PUSH:
			return "DAGGERS";
		case WEAPON_DEAGLE:
			return "DESERT EAGLE";
		case WEAPON_ELITE:
			return "DUAL BERETTAS";
		case WEAPON_FIVESEVEN:
			return "FIVE-SEVEN";
		case WEAPON_GLOCK:
			return "GLOCK-18";
		case WEAPON_HKP2000:
			return "P2000";
		case WEAPON_P250:
			return "P250";
		case WEAPON_USP_SILENCER:
			return "USP-S";
		case WEAPON_TEC9:
			return "TEC-9";
		case WEAPON_REVOLVER:
			return "R8 REVOLVER";
		case WEAPON_MAC10:
			return "MAC-10";
		case WEAPON_UMP45:
			return "UMP-45";
		case WEAPON_BIZON:
			return "PP-BIZON";
		case WEAPON_MP7:
			return "MP7";
		case WEAPON_MP5:
			return "MP5-SD";
		case WEAPON_MP9:
			return "MP9";
		case WEAPON_P90:
			return "P90";
		case WEAPON_GALILAR:
			return "GALIL AR";
		case WEAPON_FAMAS:
			return "FAMAS";
		case WEAPON_M4A1_SILENCER:
			return "M4A1-S";
		case WEAPON_M4A1:
			return "M4A4";
		case WEAPON_AUG:
			return "AUG";
		case WEAPON_SG556:
			return "SG 553";
		case WEAPON_AK47:
			return "AK-47";
		case WEAPON_G3SG1:
			return "G3SG1";
		case WEAPON_SCAR20:
			return "SCAR-20";
		case WEAPON_AWP:
			return "AWP";
		case WEAPON_SSG08:
			return "SCOUT";
		case WEAPON_XM1014:
			return "XM1014";
		case WEAPON_SAWEDOFF:
			return "SAWED-OFF";
		case WEAPON_MAG7:
			return "MAG-7";
		case WEAPON_NOVA:
			return "NOVA";
		case WEAPON_NEGEV:
			return "NEGEV";
		case WEAPON_M249:
			return "M249";
		case WEAPON_TASER:
			return "ZEUS";
		case WEAPON_FLASHBANG:
			return "FLASHBANG";
		case WEAPON_HEGRENADE:
			return "HE GRENADE";
		case WEAPON_SMOKEGRENADE:
			return "SMOKE";
		case WEAPON_MOLOTOV:
			return "MOLOTOV";
		case WEAPON_DECOY:
			return "DECOY";
		case WEAPON_INCGRENADE:
			return "INCENDIARY";
		case WEAPON_C4:
			return "BOMB";
		case WEAPON_CZ75A:
			return "CZ75A";
		case WEAPON_KNIFE_URSUS:
			return "URSUS";
		case WEAPON_KNIFE_GYPSY_JACKKNIFE:
			return "NAVAJA";
		case WEAPON_KNIFE_STILETTO:
			return "STILETTO";
		case WEAPON_KNIFE_WIDOWMAKER:
			return "TALON";
		case WEAPON_TAGRENADE:
			return "TACTICAL AWARENESS NADE";
		case WEAPON_HEALTHSHOT:
			return "HEALTH SHOT";
		case WEAPON_FISTS:
			return "FISTS";
		case WEAPON_BREACHCHARGE:
			return "BREACH CHARGE";
		case WEAPON_TABLET:
			return "TABLET";
		case WEAPON_MELEE:
			return "KNIFE";
		case WEAPON_AXE:
			return "AXE";
		case WEAPON_HAMMER:
			return "HAMMER";
		case WEAPON_SPANNER:
			return "WRENCH";
		case WEAPON_KNIFE_GHOST:
			return "GHOST KNIFE";
		case WEAPON_FIREBOMB:
			return "FIRE BOMB";
		case WEAPON_DIVERSION:
			return "DIVERSION";
		case WEAPON_FRAG_GRENADE:
			return "FRAG";
		default:
			return "ERROR";
		}
	}
};