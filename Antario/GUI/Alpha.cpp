#include "..\Settings\Settings.h"
#include "..\Hooks\Hooks.h"
#include "Alpha.h"

Alpha g_Alpha;

void Alpha::run()
{
	if (g_Settings.bMenuOpened)
	{
		g_Settings.bMenuFullFade = true;
		g_Settings.iAlpha += 15;
		if (g_Settings.iAlpha > 255)
			g_Settings.iAlpha = 255;
	}
	else
	{
		g_Settings.iAlpha -= 15;
		if (g_Settings.iAlpha < 0)
		{
			g_Settings.iAlpha = 0;
			g_Settings.bMenuFullFade = false;
		}
	}
}