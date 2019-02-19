#pragma once
#include "..\GUI\Keybind.h"
#include "..\Settings\Settings.h"
#include "..\Utils\Interfaces.h"
#include "..\Utils\Mathematic.h"
#include "..\SDK\IEngineTrace.h"

class thirdperson
{
public:
	bool bThirdperson = false;
	void doThirdperson();
};

extern thirdperson g_thirdperson;