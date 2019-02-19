#include <thread>
#include "Hooks.h"
#include "..\Utils\Utils.h"
#include "..\GUI\Alpha.h"
#include "..\SDK\IMaterialSystem.h"
#include "..\SDK\IRenderView.h"
#include "..\SDK\Cvar.h"
#include "..\SDK\IPanel.h"
#include "..\SDK\SpoofedConVar.h"
#include "..\Features\ThirdPerson.h"
#include "..\Features\Miscellaneous.h"
#include "..\Features\ColoredModels.h"
#include "..\Features\HitMarker.h"
#include "..\Features\BulletTracer.h"
#include "..\Features\Visualizations.h"
#include "..\Features\Logs.h"
#include "..\Features\EnginePrediction.h"
#include "..\Features\VisualizeSounds.h"
#include "..\Features\GrenadeRange.h"
#include "..\Features\NightMode.h"
#include "..\Features\Backtrack.h"

Misc     g_Misc;
Hooks    g_Hooks;
Settings g_Settings;

typedef void(__cdecl* MsgFn)(const char* msg, va_list);
inline void ConMsg(const char* msg, ...)
{
	if (msg == nullptr)
		return; /* If no string was passed, or it was null then don't do anything */
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg"); /* This gets the address of export "Msg" in the dll "tier0.dll". The static keyword means it's only called once and then isn't called again (but the variable is still there) */
	char buffer[989];
	va_list list; /* Normal varargs stuff http://stackoverflow.com/questions/10482960/varargs-to-printf-all-arguments */
	va_start(list, msg);
	vsprintf_s(buffer, msg, list);
	va_end(list);
	fn(buffer, list); /* Calls the function, we got the address above */
}

void Hooks::Init()
{
	/* Get window handle */
	while (!(g_Hooks.hCSGOWindow = FindWindowA("Valve001", nullptr)))
	{
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(50ms);
	}

	/* Get interfaces */
	interfaces::Init();

	Utils::Log("Hooking in progress...");

	/* D3D Device pointer */
	const uintptr_t d3dDevice = **reinterpret_cast<uintptr_t**>(Utils::FindSignature("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

	if (g_Hooks.hCSGOWindow) /* Hook WNDProc to capture mouse / keyboard input */
		g_Hooks.pOriginalWNDProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Hooks::WndProc)));

	/* VMTHooks */
	g_Hooks.pD3DDevice9Hook = std::make_unique<VMTHook>(reinterpret_cast<void*>(d3dDevice));
	g_Hooks.pClientModeHook = std::make_unique<VMTHook>(g_pClientMode);
	g_Hooks.pClientHook = std::make_unique<VMTHook>(g_pClientDll);
	g_Hooks.pSurfaceHook = std::make_unique<VMTHook>(g_pSurface);
	g_Hooks.pSceneEndHook = std::make_unique<VMTHook>(g_pRenderView);
	g_Hooks.pDMEHook = std::make_unique<VMTHook>(g_MdlRender);
	g_Hooks.pMaterialSystemHook = std::make_unique<VMTHook>(g_MatSystem);
	g_Hooks.pVGUIHook = std::make_unique<VMTHook>(g_pVgui);
	g_Hooks.pPanelHook = std::make_unique<VMTHook>(g_pPanel);

	/* Hook the table functions */
	g_Hooks.pD3DDevice9Hook->Hook(vtable_indexes::reset, Hooks::Reset);
	g_Hooks.pD3DDevice9Hook->Hook(vtable_indexes::present, Hooks::Present);
	g_Hooks.pClientModeHook->Hook(vtable_indexes::createMove, Hooks::CreateMove);
	g_Hooks.pSurfaceHook->Hook(vtable_indexes::lockCursor, Hooks::LockCursor);
	g_Hooks.pClientModeHook->Hook(vtable_indexes::effects, Hooks::Effects);
	g_Hooks.pSceneEndHook->Hook(vtable_indexes::sceneend, Hooks::SceneEnd);
	g_Hooks.pDMEHook->Hook(vtable_indexes::dme, Hooks::DME);
	g_Hooks.pMaterialSystemHook->Hook(vtable_indexes::msh, Hooks::BeginFrame);
	g_Hooks.pClientModeHook->Hook(vtable_indexes::overrideview, Hooks::OverrideView);
	g_Hooks.pClientHook->Hook(vtable_indexes::fsn, Hooks::FrameStageNotify);
	g_Hooks.pVGUIHook->Hook(vtable_indexes::paintHook, Hooks::paintHook);
	g_Hooks.pPanelHook->Hook(vtable_indexes::painttraverse, Hooks::PaintTraverse);

	/* Create event listener, no need for it now so it will remain commented. */
	//const std::vector<const char*> vecEventNames = { };
	//g_Hooks.pEventListener = std::make_unique<EventListener>(vecEventNames);

	g_pEngine->ExecuteClientCmd("developer 1");

	g_pEngine->ExecuteClientCmd("con_filter_enable 2");

	g_pEngine->ExecuteClientCmd("clear");

	g_pCvar->console_printf("[");
	g_pCvar->console_color_printf(g_Settings.Miscellaneous.cMenuColor, "yerba");
	g_pCvar->console_printf("] ");
	g::pMsg("injected.\n");

	Utils::Log("Hooking completed!");
}

void Hooks::Restore()
{
	g_pCvar->console_printf("[");
	g_pCvar->console_color_printf(g_Settings.Miscellaneous.cMenuColor, "yerba");
	g_pCvar->console_printf("] ");
	g::pMsg("unloaded.\n");

	Utils::Log("Unhooking in progress...");
	{
		/* Unhook every function we hooked and restore original one */

		g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::reset);
		g_Hooks.pD3DDevice9Hook->Unhook(vtable_indexes::present);
		g_Hooks.pClientModeHook->Unhook(vtable_indexes::createMove);
		g_Hooks.pSurfaceHook->Unhook(vtable_indexes::lockCursor);
		g_Hooks.pClientModeHook->Unhook(vtable_indexes::effects);
		g_Hooks.pClientHook->Unhook(vtable_indexes::fsn);
		g_Hooks.pSceneEndHook->Unhook(vtable_indexes::sceneend);
		g_Hooks.pDMEHook->Unhook(vtable_indexes::dme);
		g_Hooks.pMaterialSystemHook->Unhook(vtable_indexes::msh);
		g_Hooks.pClientModeHook->Unhook(vtable_indexes::overrideview);
		g_Hooks.pVGUIHook->Unhook(vtable_indexes::paintHook);
		g_Hooks.pPanelHook->Unhook(vtable_indexes::painttraverse);
		SetWindowLongPtr(g_Hooks.hCSGOWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_Hooks.pOriginalWNDProc));
	}
	Utils::Log("Unhooking successed!");

	/* Destroy fonts and all textures we created */
	g_Render.InvalidateDeviceObjects();
	g_Fonts.DeleteDeviceObjects();
}

/* NULLPTR CRASHING */
bool __fastcall Hooks::CreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* pCmd)
{
	/* Call original createmove before we start screwing with it */
	static auto oCreateMove = g_Hooks.pClientModeHook->GetOriginal<CreateMove_t>(24);
	oCreateMove(thisptr, edx, sample_frametime, pCmd);

	if (!pCmd || !pCmd->command_number)
		return oCreateMove;

	/* Get globals */
	g::pCmd = pCmd;
	g::pLocalEntity = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!g::pLocalEntity)
		return oCreateMove;

	/* Initializes backtrack */
	g_Backtrack->OnCreateMove();

	g_Misc.OnCreateMove();
	/* Run outside enginepred */

	engine_prediction::RunEnginePred();
	/* Run in enginepred */
	engine_prediction::EndEnginePred();

	return false;
}

void __fastcall Hooks::LockCursor(ISurface* thisptr, void* edx)
{
	static auto oLockCursor = g_Hooks.pSurfaceHook->GetOriginal<LockCursor_t>(vtable_indexes::lockCursor);

	if (!g_Settings.bMenuOpened)
		return oLockCursor(thisptr, edx);

	g_pSurface->UnlockCursor();
}

HRESULT __stdcall Hooks::Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	static auto oReset = g_Hooks.pD3DDevice9Hook->GetOriginal<Reset_t>(16);

	if (g_Hooks.bInitializedDrawManager)
	{
		Utils::Log("Reseting draw manager.");
		g_Render.InvalidateDeviceObjects();
		HRESULT hr = oReset(pDevice, pPresentationParameters);
		g_Render.RestoreDeviceObjects(pDevice);
		Utils::Log("DrawManager reset succeded.");
		return hr;
	}

	return oReset(pDevice, pPresentationParameters);
}

void __fastcall Hooks::DME(void* ecx, void* edx, IMatRenderContext* context, const DrawModelState_t& state, const ModelRenderInfo_t& render_info, matrix3x4_t* matrix)
{
	static auto oDME = g_Hooks.pDMEHook->GetOriginal<DrawModelExecute_t>(vtable_indexes::dme);

	auto entity = g_pEntityList->GetClientEntity(render_info.entity_index);
	auto local_player = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

	if (entity != local_player && !g_Settings.Visualizations.bBlend)
		oDME(ecx, context, state, render_info, matrix);

	if (entity != local_player && g_Settings.Visualizations.bBlend)
		oDME(ecx, context, state, render_info, matrix);

	if (entity == local_player && !local_player->GetScoped())
	{
		g_pRenderView->SetBlend(g_Settings.Visualizations.iTransparency * 0.01f);
		oDME(ecx, context, state, render_info, matrix);
	}

	if (!g_Settings.Visualizations.bBlend && local_player && local_player->GetScoped())
	{
		g_pRenderView->SetBlend(g_Settings.Visualizations.iTransparency * 0.01f);
		oDME(ecx, context, state, render_info, matrix);
	}

	if (g_Settings.Visualizations.bBlend && local_player->GetScoped())
	{
		g_pRenderView->SetBlend(0.5f);
		oDME(ecx, context, state, render_info, matrix);
	}
}

void __fastcall Hooks::SceneEnd(void* thisptr, void* edx)
{
	static auto oSceneEnd = g_Hooks.pSceneEndHook->GetOriginal<SceneEnd_t>(vtable_indexes::sceneend);

	g_chams.Render();

	oSceneEnd(thisptr);
}

void __fastcall Hooks::paintHook(void* ecx, void*, paintmode_t mode)
{
	static auto oPaint = g_Hooks.pVGUIHook->GetOriginal<fn_root_paint>(vtable_indexes::paintHook);

	oPaint(ecx, mode);
}

void doglow(cglowmanager::GlowObjectDefinition_t* glowent, float r, float g, float b, float a, int style) {
	glowent->m_flRed = r;
	glowent->m_flGreen = g;
	glowent->m_flBlue = b;
	glowent->m_flAlpha = a;

	glowent->m_nGlowStyle = style;

	glowent->m_bRenderWhenOccluded = true;
	glowent->m_bRenderWhenUnoccluded = false;
}

int __fastcall Hooks::Effects(void *thisptr, void * _EDX, int a1)
{
	static auto oPostEffects = g_Hooks.pClientModeHook->GetOriginal<Effects_t>(44);

	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return oPostEffects(thisptr, _EDX, a1);

	static cglowmanager* objectmanager = *(cglowmanager**)(Utils::FindSignature("client_panorama.dll", "0F 11 05 ? ? ? ? 83 C8 01") + 3);

	for (int i = 0; i < objectmanager->size; i++)
	{
		cglowmanager::GlowObjectDefinition_t* glowent = &objectmanager->m_GlowObjectDefinitions[i];

		C_BaseEntity* entity = glowent->m_pEntity;
		C_BaseEntity* localplayer = g_pEntityList->GetClientEntity(g_pEngine->GetLocalPlayer());

		if (!entity)
			continue;

		if (entity->IsDormant())
			continue;

		switch (entity->GetClientClass()->ClassID)
		{
		case CCSPlayer:
			if (entity->GetTeam() != localplayer->GetTeam() && entity != localplayer && g_Settings.Visualizations.bGlow)  /* Enemies */
				doglow(glowent, g_Settings.Visualizations.cGlow.red / 255.f, g_Settings.Visualizations.cGlow.green / 255.f, g_Settings.Visualizations.cGlow.blue / 255.f,
					g_Settings.Visualizations.cGlow.alpha / 255.f, 0);
			break;
		default:
			break;
		}

	}

	return oPostEffects(thisptr, _EDX, a1);
}

auto smoke_count = *reinterpret_cast<uint32_t **>(Utils::FindSignature("client_panorama.dll", "A3 ? ? ? ? 57 8B CB") + 0x1);

void removeSmoke()
{
	/* Store the smoke material here */
	std::vector<const char*> smokeMaterial =
	{
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	/* Check if checkbox is on */
	if (g_Settings.Visualizations.bRemoveSmoke)
	{
		for (auto material : smokeMaterial)
		{
			/* Grab the smoke material */
			IMaterial* mat = g_MatSystem->FindMaterial(material, TEXTURE_GROUP_OTHER);

			/* Change the smoke material to wireframe */
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
		}

		*(int*)smoke_count = 0;
	}
	else
	{
		for (auto material : smokeMaterial)
		{
			/* Grab the smoke material */
			IMaterial* mat = g_MatSystem->FindMaterial(material, TEXTURE_GROUP_OTHER);

			/* Change the smoke material to wireframe */
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
		}
	}
}

Vector flb_aim_punch;
Vector flb_view_punch;

Vector * aim_punch;
Vector * view_punch;

void  __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	if (!g_pEngine->IsInGame() || !g_pEngine->IsConnected())
		return;

	if (g::pLocalEntity == nullptr)
		return;

	static auto oFrameStageNotify = g_Hooks.pClientHook->GetOriginal<FrameStageNotify_t>(37);

	if (!g::pLocalEntity)
	{
		g_nightmode.clear_stored_materials();

		return oFrameStageNotify(curStage);
	}

	aim_punch = nullptr;
	view_punch = nullptr;

	flb_aim_punch = Vector{ };
	flb_view_punch = Vector{ };

	if (g_Settings.Visualizations.bRemoveRecoil)
	{
		aim_punch = &g::pLocalEntity->GetAimPunch();
		view_punch = &g::pLocalEntity->GetViewPunch();

		flb_aim_punch = *aim_punch;
		flb_view_punch = *view_punch;

		*aim_punch = Vector(0, 0, 0);
		*view_punch = Vector(0, 0, 0);
	}

	if (curStage == FRAME_RENDER_START)
	{
		/* Remove smoke (wireframe) */
		removeSmoke();

		/* No flash */
		g::pLocalEntity->SetFlashAlpha() = g_Settings.Visualizations.iFlashbangEffectAlpha;
	}

	if (curStage == FRAME_NET_UPDATE_START)
	{
		/* Bullet tracers */
		g_bullettracer.draw();

		/* Sound esp */
		g_soundesp.draw();
	}

	if (curStage == FRAME_NET_UPDATE_END)
	{

	}

	if (curStage == FRAME_RENDER_END)
	{
		auto fullbright = g_pCvar->FindVar("mat_fullbright");

		if (g_Settings.Visualizations.bFullBright)
			fullbright->SetValue(1);
		else
			fullbright->SetValue(0);

		if (g_Settings.Visualizations.bNightmode)
			g_nightmode.apply();
		else
			g_nightmode.remove();
	}

	oFrameStageNotify(curStage);
}

void __fastcall Hooks::PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	static auto oPaintTraverse = g_Hooks.pPanelHook->GetOriginal<PaintTraverse_t>(vtable_indexes::painttraverse);

	if (!strcmp("HudZoom", g_pPanel->GetName(vguiPanel)) && g_Settings.Visualizations.bRemoveScope)
		return;

	static bool disabled_postprocessing = false;

	if (g_Settings.Visualizations.bDisablePostProcessing && !disabled_postprocessing)
	{
		auto mat_postprocess_enable = g_pCvar->FindVar("mat_postprocess_enable");
		auto mat_postprocess_enablespoof = new SpoofedConvar(mat_postprocess_enable);
		mat_postprocess_enablespoof->SetInt(0);

		disabled_postprocessing = true;
	}
	if (!g_Settings.Visualizations.bDisablePostProcessing && disabled_postprocessing)
	{
		auto mat_postprocess_enable = g_pCvar->FindVar("mat_postprocess_enable");
		auto mat_postprocess_enablespoof = new SpoofedConvar(mat_postprocess_enable);
		mat_postprocess_enablespoof->SetInt(1);

		disabled_postprocessing = false;
	}

	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}

void __stdcall Hooks::OverrideView(CViewSetup* vsView)
{
	static auto ofunc = g_Hooks.pClientModeHook->GetOriginal<OverrideView_t>(vtable_indexes::overrideview);

	if (g_Settings.Miscellaneous.iOverrideFOV > 0 && !g_thirdperson.bThirdperson)
		vsView->fov += g_Settings.Miscellaneous.iOverrideFOV;

	auto aspectRatio = g_pCvar->FindVar("r_aspectratio");
	aspectRatio->SetValue(g_Settings.Miscellaneous.iForceAspectRatio);

	if (g_Settings.Visualizations.iThirdpersonFOV > 0 && g_thirdperson.bThirdperson)
	{
		vsView->fov += g_Settings.Visualizations.iThirdpersonFOV;
	}

	/* Thirdperson */
	g_thirdperson.doThirdperson();

	ofunc(g_pClientMode, vsView);
}

void __fastcall Hooks::BeginFrame(void *thisptr, void*, float ft)
{
	static auto oBeginFrame = g_Hooks.pMaterialSystemHook->GetOriginal<BeginFrame_t>(vtable_indexes::msh);

	oBeginFrame(thisptr, ft);
}

HRESULT __stdcall Hooks::Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect,
	HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{

	static auto oPresent = g_Hooks.pD3DDevice9Hook->GetOriginal<Present_t>(17);

	IDirect3DStateBlock9* stateBlock = nullptr;
	IDirect3DVertexDeclaration9* vertDec = nullptr;

	pDevice->GetVertexDeclaration(&vertDec);
	pDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &stateBlock);

	[pDevice]()
	{
		if (!g_Hooks.bInitializedDrawManager)
		{
			Utils::Log("Initializing draw manager...");
			g_Render.InitDeviceObjects(pDevice);
			g_Hooks.nMenu.Initialize();
			g_Hooks.bInitializedDrawManager = true;
			Utils::Log("Draw manager initialized!");
			g_hitmarker.listener();
			g_bullettracer.listener();
			g_pEngine->ExecuteClientCmd("con_filter_text in the");
			g_logs.listener();
			g_soundesp.listener();
			g_grenaderange.listener();
		}
		else
		{
			/* Sets up proper render states for our state block */
			g_Render.SetupRenderStates();

			int screenWidth, screenHeight;
			g_pEngine->GetScreenSize(screenWidth, screenHeight);

			SIZE sz;
			INetChannelInfo *nci = g_pEngine->GetNetChannelInfo();
			std::string watermark_text = g_pEngine->IsInGame() && nci ? "yerba | rtt: " +
				std::to_string(int(nci->GetAvgLatency(FLOW_INCOMING) + nci->GetAvgLatency(FLOW_OUTGOING) * 1000.f)) + "ms" : "yerba";

			g_Fonts.pFontTahoma8->GetTextExtent(watermark_text.c_str(), &sz);

			if (g_Settings.Miscellaneous.bWatermark) {
				g_Render.RectFilled(screenWidth - sz.cx - 19, 25, screenWidth - 12, 10, g_Settings.Miscellaneous.cMenuColor(150));
				g_Render.String(screenWidth - sz.cx - 15, 12, CD3DFONT_DROPSHADOW, Color(210, 210, 210, 255), g_Fonts.pFontTahoma8.get(), watermark_text);
			}

			/* Esp */
			g_ESP.Render();

			/* Put your draw calls here */
			g_Alpha.run();
			g_hitmarker.run();

			/* Grenade range */
			g_grenaderange.draw();


			if ((g::pLocalEntity && g_Settings.Visualizations.bRemoveScope && g::pLocalEntity->GetScoped()) && g_pEngine->IsConnected() && g_pEngine->IsInGame())
			{
				g_Render.Line(0, screenHeight / 2, screenWidth, screenHeight / 2, Color::Black());
				g_Render.Line(screenWidth / 2, 0, screenWidth / 2, screenHeight, Color::Black());
			}

			if (g_Settings.bMenuFullFade)
				g_Hooks.nMenu.Render();

			if (g_Settings.bMenuOpened)
			{
				/* Render our menu */
				//g_Hooks.nMenu.Render();

				/* Render mouse cursor in the end so its not overlapped */
				g_Hooks.nMenu.mouseCursor->Render();
			}
		}
	}();

	stateBlock->Apply();
	stateBlock->Release();
	pDevice->SetVertexDeclaration(vertDec);

	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

LRESULT Hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	/* For now as a lambda, to be transfered somewhere */
	/* Working when you HOLD th button, not when you press it */
	const auto getButtonHeld = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;

		if (uMsg == WM_KEYDOWN)
			bButton = true;
		else if (uMsg == WM_KEYUP)
			bButton = false;
	};

	const auto getButtonToggle = [uMsg, wParam](bool& bButton, int vKey)
	{
		if (wParam != vKey) return;

		if (uMsg == WM_KEYUP)
			bButton = !bButton;
	};

	//if (g_pInputSystem->IsButtonDown(KEY_INSERT))
		//g_Settings.bMenuOpened = !g_Settings.bMenuOpened;

	getButtonToggle(g_Settings.bMenuOpened, VK_INSERT);

	if (g_Hooks.bInitializedDrawManager)
	{
		/* Our wndproc capture fn */
		if (g_Settings.bMenuOpened)
		{
			g_Hooks.nMenu.MsgProc(uMsg, wParam, lParam);
			return true;
		}
	}

	/* Call original wndproc to make game use input again */
	return CallWindowProcA(g_Hooks.pOriginalWNDProc, hWnd, uMsg, wParam, lParam);
}