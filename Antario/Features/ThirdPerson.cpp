#include "ThirdPerson.h"

/* Thirdperson */
thirdperson g_thirdperson;

void thirdperson::doThirdperson()
{
	/* Return if local is nullptr */
	if (g::pLocalEntity == nullptr)
		return;

	/* Check if we are connected */
	if (!g_pEngine->IsConnected() && !g_pEngine->IsInGame())
		return;

	/* Vec angles */
	static Vector vecAngles;

	/* Pointer to localplayer */
	C_BaseEntity* localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	/* Get view angles */
	g_pEngine->GetViewAngles(vecAngles);

	/* Keybind (if key pressed, make bool true) */
	if (g::pLocalEntity->IsAlive())
		Keybind(g_Settings.Visualizations.btThirdperonKeybind, g_Settings.Visualizations.iThirdpersonKeybind, bThirdperson);

	if (g_Settings.Visualizations.bSpecThirdperson && !g::pLocalEntity->IsAlive())
	{
		g::pLocalEntity->SetObserverMode() = 5;
	}

	/* If we have clicked the key */
	if (bThirdperson && g::pLocalEntity->IsAlive())
	{
		/* If we are not in thirdperson */
		if (!g_pInput->m_fCameraInThirdPerson)
		{
			/* Getting correct distance */
			auto GetCorrectDistance = [&localplayer](float ideal_distance) -> float
			{
				/* Vector for the inverse angles */
				Vector inverseAngles;
				g_pEngine->GetViewAngles(inverseAngles);

				/* Inverse angles by 180 */
				inverseAngles.x *= -1.f, inverseAngles.y += 180.f;

				/* Vector for direction */
				Vector direction;
				g_Math.AngleVectors(inverseAngles, &direction);

				/* Ray, trace & filters */
				Ray_t ray;
				trace_t trace;
				CTraceFilter filter;

				/* Dont trace local player */
				filter.pSkip = g::pLocalEntity;

				/* Create ray */
				ray.Init(localplayer->GetVecOrigin() + localplayer->GetViewOffset(), (localplayer->GetVecOrigin() + localplayer->GetViewOffset()) + (direction * ideal_distance));

				/* Trace ray */
				g_pEngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

				/* Return the ideal distance */
				return (ideal_distance * trace.fraction) - 10.f;
			};

			/* Change the distance from player to camera */

			/* Make player thirdperson */
			g_pInput->m_fCameraInThirdPerson = true;

			/* Set camera view */
			g_pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, vecAngles.z);
		}
	}
	else
	{
		/* Set player to firstperson */
		g_pInput->m_fCameraInThirdPerson = false;

		/* Return to default view */
		g_pInput->m_vecCameraOffset = Vector(vecAngles.x, vecAngles.y, 0);
	}
}