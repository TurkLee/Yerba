#pragma once

#include "..\Utils\Utils.h"

enum paintmode_t
{
	PAINT_UIPANELS = 1 << 0,
	PAINT_INGAMEPANELS = 1 << 1,
	PAINT_CURSOR = 1 << 2
};

class IEngineVGUI
{
public:

};

extern IEngineVGUI* g_pVgui;

class IPanel
{
public:
	const char *GetName(unsigned int vguiPanel)
	{
		return Utils::CallVFunc<36, const char *, unsigned int>(this, vguiPanel);
	}
};

extern IPanel* g_pPanel;