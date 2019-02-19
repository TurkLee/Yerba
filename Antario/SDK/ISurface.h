#pragma once
#include "..\Utils\Utils.h"

typedef unsigned long HFont;
typedef unsigned int VPANEL;

class ISurface
{
public:
	void UnlockCursor()
	{
		return Utils::CallVFunc<66, void>(this);
	}

	void play_sound(const char * sound_path)
	{
		Utils::CallVFunc<82, void, const char* >(this, sound_path);
	}
};
extern ISurface* g_pSurface;