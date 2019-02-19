#pragma once

#include "..\Utils\DrawManager.h"
#include "..\Utils\Interfaces.h"
#include "..\SDK\IClientMode.h"
#include "..\SDK\ISurface.h"
#include "..\Features\EventListener.h"
#include "..\SDK\CInput.h"
#include "..\GUI\GUI.h"
#include "..\SDK\IVModelRender.h"
#include "..\Features\Glow.h"
#include "..\SDK\IBaseClientDll.h"
#include "..\SDK\IPanel.h"

namespace vtable_indexes
{
	constexpr auto reset = 16;
	constexpr auto present = 17;
	constexpr auto createMove = 24;
	constexpr auto lockCursor = 67;
	constexpr auto dme = 21;
	constexpr auto effects = 44;
	constexpr auto sceneend = 9;
	constexpr auto paintHook = 14;
	constexpr auto overrideview = 18;
	constexpr auto msh = 42;
	constexpr auto fsn = 37;
	constexpr auto painttraverse = 41;
}

class VMTHook;
class Hooks
{
public:
	/* Initialization setup, called on injection */
	static  void Init();
	static  void Restore();

	/*---------------------------------------------*/
	/*-------------Hooked functions----------------*/
	/*---------------------------------------------*/

	static bool     __fastcall  CreateMove(IClientMode*, void*, float, CUserCmd*);
	static void     __fastcall  LockCursor(ISurface*, void*);
	static HRESULT  __stdcall   Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	static void     __fastcall  SceneEnd(void* thisptr, void* edx);
	static void     __fastcall  DME(void*, void*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	static HRESULT  __stdcall   Present(IDirect3DDevice9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion);
	static LRESULT  __stdcall   WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int		__fastcall	Effects(void *thisptr, void * _EDX, int a1);
	static void     __fastcall  BeginFrame(void *thisptr, void*, float ft);
	static void     __fastcall  PaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
	static void	    __stdcall   OverrideView(CViewSetup* vsView);
	static void     __stdcall   FrameStageNotify(ClientFrameStage_t curtime);
	static void     __fastcall  paintHook(void* ecx, void*, paintmode_t mode);

private:
	/*---------------------------------------------*/
	/*-------------VMT Hook pointers---------------*/
	/*---------------------------------------------*/

	std::unique_ptr<VMTHook> pD3DDevice9Hook;
	std::unique_ptr<VMTHook> pClientModeHook;
	std::unique_ptr<VMTHook> pClientHook;
	std::unique_ptr<VMTHook> pSurfaceHook;
	std::unique_ptr<VMTHook> pDMEHook;
	std::unique_ptr<VMTHook> pSceneEndHook;
	std::unique_ptr<VMTHook> pMaterialSystemHook;
	std::unique_ptr<VMTHook> pVGUIHook;
	std::unique_ptr<VMTHook> pPanelHook;

	/*---------------------------------------------*/
	/*-------------Hook prototypes-----------------*/
	/*---------------------------------------------*/

	typedef bool(__fastcall* CreateMove_t) (IClientMode*, void*, float, CUserCmd*);
	typedef void(__fastcall* LockCursor_t) (ISurface*, void*);
	typedef long(__stdcall*  Reset_t)      (IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	typedef long(__stdcall*  Present_t)    (IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
	typedef  int(__fastcall  *Effects_t)(void*, void*, int);
	using FrameStageNotify_t = void(__stdcall*)(ClientFrameStage_t);
	typedef void(__thiscall  *BeginFrame_t)(void*, float);
	typedef void(__thiscall*  SceneEnd_t)  (void*);
	typedef void(__thiscall*  DrawModelExecute_t)(void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	using PaintTraverse_t = void(__thiscall*)(void*, VPANEL, bool, bool);
	using OverrideView_t = void(__thiscall*)(IClientMode*, CViewSetup*);
	using fn_root_paint = void(__thiscall*)(void* ecx, paintmode_t mode);
private:
	/* CSGO window handle */
	HWND hCSGOWindow = nullptr;

	/* Check if we initialized our draw manager */
	bool bInitializedDrawManager = false;

	/* Original CSGO window proc */
	WNDPROC pOriginalWNDProc = nullptr;

	/* Listens to csgo events, needs to be created */
	std::unique_ptr<EventListener> pEventListener = nullptr;
	ui::MenuMain nMenu;
};

extern Hooks g_Hooks;

class VMTHook
{
public:
	VMTHook(void* ppClass)
	{
		this->ppBaseClass = static_cast<std::uintptr_t**>(ppClass);

		/* Loop through all valid class indexes. When it will hit invalid (not existing) it will end the loop */
		while (static_cast<std::uintptr_t*>(*this->ppBaseClass)[this->indexCount])
			++this->indexCount;

		const std::size_t kSizeTable = this->indexCount * sizeof(std::uintptr_t);


		this->pOriginalVMT = *this->ppBaseClass;
		this->pNewVMT = std::make_unique<std::uintptr_t[]>(this->indexCount);

		/* Copy original vtable to our local copy of it */
		std::memcpy(this->pNewVMT.get(), this->pOriginalVMT, kSizeTable);

		/* Replace original class with our local copy */
		*this->ppBaseClass = this->pNewVMT.get();
	};
	~VMTHook() { *this->ppBaseClass = this->pOriginalVMT; };

	template<class Type>
	Type GetOriginal(const std::size_t index)
	{
		return reinterpret_cast<Type>(this->pOriginalVMT[index]);
	};

	template<typename T>
	T GetMethod(size_t nIndex)
	{
		return (T)OriginalTable[nIndex];
	}

	HRESULT Hook(const std::size_t index, void* fnNew)
	{
		if (index > this->indexCount) /* check if given index is valid */
			return E_INVALIDARG;

		this->pNewVMT[index] = reinterpret_cast<std::uintptr_t>(fnNew);
		return S_OK;
	};

	HRESULT Unhook(const std::size_t index)
	{
		if (index > this->indexCount)
			return E_INVALIDARG;

		this->pNewVMT[index] = this->pOriginalVMT[index];
		return S_OK;
	};

private:
	std::unique_ptr<std::uintptr_t[]> pNewVMT = nullptr; /* Actual used vtable */
	std::uintptr_t** ppBaseClass = nullptr; /* Saved pointer to original class */
	std::uintptr_t* pOriginalVMT = nullptr; /* Saved original pointer to the VMT */
	std::size_t indexCount = 0; /* Count of indexes inside out f-ction */
};