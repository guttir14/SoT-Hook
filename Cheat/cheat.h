#pragma once
#include <windows.h>
#include <HookLib/HookLib.h>
#include <Psapi.h>
#include <d3d11.h>
#include <iostream>
#include <mutex>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_internal.h>
#include <imgui\imgui_impl_win32.h>
#include "SDK.h"

class Cheat {
private:
	class Renderer {
	private:
		struct Drawing {
			static void RenderText(char* text, FVector2D& pos, bool outlined, bool centered);
			static void Render2DBox(FVector2D& top, FVector2D& bottom, float height, float width, ImVec4& color);
			static bool Render3DBox(AController*& controller, FVector& origin, FVector& extent, FRotator& rotation, ImVec4& color);
			static bool RenderColorBar(FVector& origin, float size,  ImU32*& colors, int n);
		};
	private:
		static inline void** vtable;
		static inline HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
		static inline HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
		static inline ID3D11Device* device = nullptr;
		static inline ID3D11DeviceContext* context = nullptr;
		static inline ID3D11RenderTargetView* renderTargetView = nullptr;
		static inline bool bGameInput = false;
		static inline WNDPROC WndProcOriginal = nullptr;
		static inline HWND gameWindow;
		static inline ImFont* Arial;
		//static inline ImDrawList* drawList = nullptr;
	private:
		static LRESULT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static void HookInput();
		static void RemoveInput();
		static HRESULT PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
		static HRESULT ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
	public:
		static inline bool Init();
		static inline bool Remove();
	};
	class Tools {
	private:
		static inline bool CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size);
		static inline BYTE* FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size);
		static void* FindPointer(BYTE* sig, SIZE_T size, int addition);
	public:
		static inline BYTE* FindFn(HMODULE mod, BYTE* sig, SIZE_T sigSize);
		static inline bool PatchMem(void* address, void* bytes, SIZE_T size);
		static inline BYTE* PacthFn(HMODULE mod, BYTE* sig, SIZE_T sigSize, BYTE* bytes, SIZE_T bytesSize);
		//static inline bool HookVT(void** vtable, UINT64 index, void* FuncH, void** FuncO);
		//static inline void ShowErrorMsg(const CHAR* lpszFunction);
		static inline bool FindNameArray();
		static inline bool FindObjectsArray();
		static inline bool FindWorld();
		static inline bool InitSDK();
		
	};

	class Logger {
	private:
		static inline HANDLE file = nullptr;
		static inline std::mutex mutex;
	public:
		static inline bool Init();
		static inline bool Remove();
		static void Log(const char* format, ...);
	};

public:
	static bool Init(HINSTANCE _hinstDLL);
	static void ClearingThread();
	static void Tests();
	static bool Remove();
private:
	inline static MODULEINFO gBaseMod;
	inline static HINSTANCE hinstDLL;
};


