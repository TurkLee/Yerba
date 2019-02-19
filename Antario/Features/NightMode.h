#pragma once
#include "..\SDK\IMaterialSystem.h"
#include "..\SDK\Singleton.h"
#include "..\SDK\Vector.h"
#include "..\SDK\Cvar.h"
#include "..\SDK\SpoofedConVar.h"
#include "..\Settings\Settings.h"

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
class MaterialBackup {
public:
	MaterialHandle_t handle;
	IMaterial* material;
	float color[3];
	float alpha;
	bool translucent;
	bool nodraw;

	MaterialBackup() {

	}

	MaterialBackup(MaterialHandle_t h, IMaterial* p) {
		handle = handle;
		material = p;

		material->GetColorModulation(&color[0], &color[1], &color[2]);
		alpha = material->GetAlphaModulation();

		translucent = material->GetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT);
		nodraw = material->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW);
	}

	void restore() {
		material->ColorModulate(color[0], color[1], color[2]);
		material->AlphaModulate(alpha);
		material->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, translucent);
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, nodraw);
	}
};

class nightmode
{
public:
	void clear_stored_materials();
	void modulate(MaterialHandle_t i, IMaterial *material, bool backup);
	void apply();
	void remove();
	void listener();
	create_event_listener(client_disconnect_event);
};

extern nightmode g_nightmode;