#pragma once
#include "..\Settings\Settings.h"
#include "..\Utils\GlobalVars.h"
#include "..\SDK\IGameEvent.h"
#include "..\SDK\Vector.h"
#include "..\SDK\IClientEntity.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntityList.h"
#include "..\SDK\ISurface.h"
#include "..\SDK\IViewRenderBeams.h"
#include "..\SDK\Singleton.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\SDK\IEngineSound.h"
#include "..\SDK\UtlVector.h"

/* Definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener2\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\

/* Sound info */
class sound_info
{
public:
	sound_info(Vector positions, float times, int userids)
	{
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	float time;
	int userid;
};

/* Functions */
class soundesp
{
public:
	void draw();
	void draw_circle(Color color, Vector position);
	void listener();
	create_event_listener(player_hurt_event);
	create_event_listener(player_footstep_event);
};

/* Extern sound esp */
extern soundesp g_soundesp;