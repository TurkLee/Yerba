#pragma once
#include "..\SDK\CInput.h"

inline void Keybind(ButtonCode_t button, int mode, bool& enabled)
{
	static bool holding;

	if (mode == 0)
	{
		enabled = true;
	}
	else if (mode == 1)
	{
		if (g_pInputSystem->IsButtonDown(button))
			enabled = true;
		else
			enabled = false;
	}
	else if (mode == 2)
	{
		if (g_pInputSystem->IsButtonDown(button) && !holding)
		{
			enabled = !enabled;
			holding = true;
		}
		else if (!g_pInputSystem->IsButtonDown(button) && holding)
			holding = false;
	}
	else if (mode == 3)
	{
		if (g_pInputSystem->IsButtonDown(button))
			enabled = false;
		else
			enabled = true;
	}
}