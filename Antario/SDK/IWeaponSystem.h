#pragma once
#include "Definitions.h"

class IWeaponSystem
{
	virtual void unused0() = 0;
	virtual void unused1() = 0;
public:
	virtual const WeaponInfo_t* GetWpnData(unsigned ItemDefinitionIndex) = 0;
};

extern IWeaponSystem* g_pWeaponSys;