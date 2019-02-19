#pragma once
#include "..\SDK\IGameEvent.h"
#include "..\SDK\Vector.h"
#include "..\SDK\IClientEntity.h"
#include "..\SDK\CGlobalVarsBase.h"
#include "..\SDK\IClientEntityList.h"
#include "..\Utils\GlobalVars.h"
#include "..\SDK\ISurface.h"
#include "..\Settings\Settings.h"
#include "..\SDK\Singleton.h"
#include "..\SDK\Cvar.h"

/* Definitions */
#define create_event_listener(class_name)\
class class_name : public IGameEventListener2\
{\
public:\
	~class_name() { g_pEventManager->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* p_event);\
};\

/* Functions */
class Logs
{
public:
	void listener();
	create_event_listener(player_hurt_event);
};

/* Extern logs */
extern Logs g_logs;