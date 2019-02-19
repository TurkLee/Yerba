#include "Interfaces.h"
#include "Utils.h"
#include "..\SDK\CDebugOverlay.h"
#include "..\SDK\IClientMode.h"
#include "..\SDK\IBaseClientDll.h"
#include "..\SDK\IClientEntityList.h"
#include "..\SDK\IVEngineClient.h"
#include "..\SDK\CPrediction.h"
#include "..\SDK\IGameEvent.h"
#include "..\SDK\ISurface.h"
#include "..\SDK\IRenderView.h"
#include "..\SDK\IVModelRender.h"
#include "..\SDK\IVModelInfoClient.h"
#include "..\SDK\IViewRenderBeams.h"
#include "..\SDK\IEngineSound.h"
#include "..\SDK\Cvar.h"
#include "..\SDK\IWeaponSystem.h"
#include "..\SDK\CClientState.h"
#include "..\SDK\IPanel.h"

// Initializing global interfaces

IBaseClientDLL*     g_pClientDll       = nullptr;
IClientMode*        g_pClientMode      = nullptr;
IClientEntityList*  g_pEntityList      = nullptr;
IClientState*         g_pClientState = nullptr;
IVEngineClient*     g_pEngine          = nullptr;
CPrediction*        g_pPrediction      = nullptr;
IGameMovement*      g_pMovement        = nullptr;
IMoveHelper*        g_pMoveHelper      = nullptr;
CGlobalVarsBase*    g_pGlobalVars      = nullptr;
IGameEventManager2* g_pEventManager    = nullptr;
ISurface*           g_pSurface         = nullptr;
IVRenderView*       g_pRenderView	   = nullptr;
IVModelRender*		g_MdlRender		   = nullptr;
IVModelInfoClient*  g_MdlInfo          = nullptr;
IMaterialSystem*    g_MatSystem        = nullptr;
CDebugOverlay*      g_pDebugOverlay    = nullptr;
IViewRenderBeams*   g_pViewRenderBeams = nullptr;
IInputSystem*       g_pInputSystem     = nullptr;
ICVar*              g_pCvar            = nullptr;
IEngineTrace*		g_pEngineTrace     = nullptr;
CInput*             g_pInput           = nullptr;
CHudChat*           g_pChatElement     = nullptr;
IWeaponSystem*      g_pWeaponSys       = nullptr;
IPanel*             g_pPanel           = nullptr;
IEngineVGUI*        g_pVgui            = nullptr;
IEngineSound*       g_pEngineSound      = nullptr;

namespace interfaces
{
    template<typename T>
    T* CaptureInterface(const char* szModuleName, const char* szInterfaceVersion)
    {
        HMODULE moduleHandle = GetModuleHandleA(szModuleName);
        if (moduleHandle)   /* In case of not finding module handle, throw an error. */
        {
            CreateInterfaceFn pfnFactory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(moduleHandle, "CreateInterface"));
            return reinterpret_cast<T*>(pfnFactory(szInterfaceVersion, nullptr));
        }
        Utils::Log("Error getting interface %", szInterfaceVersion);
        return nullptr;
    }


    void Init()
    {
		g_pClientDll       = CaptureInterface<IBaseClientDLL>("client_panorama.dll", "VClient018");																		// Get IBaseClientDLL
		g_pClientMode      = **reinterpret_cast<IClientMode***>    ((*reinterpret_cast<uintptr_t**>(g_pClientDll))[10] + 0x5u);											// Get IClientMode
		g_pGlobalVars      = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(g_pClientDll))[0] + 0x1Bu);											// Get CGlobalVarsBase
		g_pEntityList      = CaptureInterface<IClientEntityList>("client_panorama.dll", "VClientEntityList003");														// Get IClientEntityList
		g_pEngine          = CaptureInterface<IVEngineClient>("engine.dll", "VEngineClient014");																		// Get IVEngineClient
		g_pPrediction      = CaptureInterface<CPrediction>("client_panorama.dll", "VClientPrediction001");																// Get CPrediction
		g_pMovement        = CaptureInterface<IGameMovement>("client_panorama.dll", "GameMovement001");																	// Get IGameMovement
		g_pMoveHelper      = **reinterpret_cast<IMoveHelper***>((Utils::FindSignature("client_panorama.dll", "8B 0D ? ? ? ? 8B 46 08 68") + 0x2));						// Get IMoveHelper
		g_pEventManager    = CaptureInterface<IGameEventManager2>("engine.dll", "GAMEEVENTSMANAGER002");																// Get IGameEventManager2
		g_pSurface         = CaptureInterface<ISurface>("vguimatsurface.dll", "VGUI_Surface031");																		// Get ISurface
		g_pRenderView      = CaptureInterface<IVRenderView>("engine.dll", "VEngineRenderView014");																		// Get IVRenderView
		g_MdlRender        = CaptureInterface<IVModelRender>("engine.dll", "VEngineModel016");					                                   						// Get IVMofelRender
		g_MdlInfo          = CaptureInterface<IVModelInfoClient>("engine.dll", "VModelInfoClient004");																	// Get IVModelInfoClient
		g_MatSystem        = CaptureInterface<IMaterialSystem>("materialsystem.dll", "VMaterialSystem080");																// Get IMaterialSystem
		g_pDebugOverlay    = CaptureInterface<CDebugOverlay>("engine.dll", "VDebugOverlay004");																		    // Get IDebugOverlay
		g_pViewRenderBeams = *reinterpret_cast<IViewRenderBeams**>((Utils::FindSignature("client_panorama.dll", "B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 1));     // Get IViewRenderBeams
		g_pInputSystem     = CaptureInterface<IInputSystem>("inputsystem.dll", "InputSystemVersion001");																// Get IInputSystem
		g_pCvar            = CaptureInterface<ICVar>("vstdlib.dll", "VEngineCvar007");																				    // Get ICVar
		g_pEngineTrace     = CaptureInterface<IEngineTrace>("engine.dll", "EngineTraceClient004");																	    // Get IEngineTrace
		g_pInput           = *reinterpret_cast<CInput**>(Utils::FindSignature("client_panorama.dll", "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 1);					    // Get CInput
		g_pWeaponSys       = *reinterpret_cast<IWeaponSystem**>(Utils::FindSignature("client_panorama.dll", "8B 35 ? ? ? ? FF 10 0F B7 C0") + 0x2);						// Get IWeaponSystem
		g_pVgui			   = CaptureInterface<IEngineVGUI>("engine.dll", "VEngineVGui001");																				// Get IEngineVGUI
		g_pPanel           = CaptureInterface<IPanel>("vgui2.dll", "VGUI_Panel009");																					// Get IPanel
		g_pClientState     = **reinterpret_cast<IClientState***>((Utils::FindSignature("engine.dll", "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0") + 0x1));			    // Get IClientState
		g_pEngineSound     = CaptureInterface<IEngineSound>("engine.dll", "IEngineSoundClient003");																		// Get IEngineSound
		//g_pChatElement   = FindHudElement<CHudChat>("CHudChat");
    }
}
