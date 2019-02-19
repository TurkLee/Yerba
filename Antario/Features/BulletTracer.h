#pragma once
#include "..\SDK\IGameEvent.h"
#include "..\SDK\Vector.h"
#include "..\SDK\IClientEntity.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntityList.h"
#include "..\SDK\ISurface.h"
#include "..\SDK\IViewRenderBeams.h"
#include "..\SDK\Singleton.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\Settings\Settings.h"
#include "..\Utils\GlobalVars.h"

/* Definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener2\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\

/* Trace info */
class trace_info
{
public:
	trace_info(Vector starts, Vector positions, float times, int userids)
	{
		this->start = starts;
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	Vector start;
	float time;
	int userid;
};

/* Functions */
class bullettracer
{
public:
	void draw();
	void listener();
	create_event_listener(player_hurt_event);
	create_event_listener(bullet_impact_event);
	void draw_beam(Vector src, Vector end, Color color);
};

/* Extern bullet tracer */
extern bullettracer g_bullettracer;