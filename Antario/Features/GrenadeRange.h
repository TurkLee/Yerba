#pragma once
#include "..\SDK\IGameEvent.h"
#include "..\SDK\Vector.h"
#include "..\SDK\IClientEntity.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntityList.h"
#include "..\Utils\GlobalVars.h"
#include "..\SDK\ISurface.h"
#include "..\Settings\Settings.h"
#include "..\SDK\IViewRenderBeams.h"
#include "..\SDK\Singleton.h"
#include "..\SDK\IVModelInfoClient.h"

/* Definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener2\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\

/* Molotov */
class molotov_info
{
public:
	molotov_info(Vector positions, bool enableds, int entityids)
	{
		this->position = positions;
		this->enabled = enableds;
		this->entityid = entityids;
	}

	Vector position;
	bool   enabled;
	int    entityid;
};

/* Functions */
class GrenadeRange
{
public:
	void listener();
	void draw();
	void TraceCircle(Vector position);
	void molotov();

	/* Molotov */
	create_event_listener(inferno_startburn_event);
	create_event_listener(inferno_expire_event);
	create_event_listener(round_end_event);
};

/* Extern grenade range */
extern GrenadeRange g_grenaderange;