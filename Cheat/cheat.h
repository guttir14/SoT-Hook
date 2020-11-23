#pragma once
#include <windows.h>
#include <Psapi.h>
#include <d3d11.h>
#include <imgui/imgui.h>
#include "SDK.h"

class Cheat {
private:
    static inline struct Cache{
        AController* localController;
        APlayerCameraManager* localCamera;
        AAthenaGameState* gameState;
        bool good;
    } cache;
    static inline struct Config {
        enum class EBox : int {
            ENone,
            E2DBoxes,
            E3DBoxes,
            EDebugBoxes
        };
        enum class EShipBox {
            ENone,
            E3DBoxes
        };
        enum class EBar : int {
            ENone,
            ELeft,
            ERight,
            EBottom,
            ETop,
            ETriangle
        };
        enum class EAim {
            ENone,
            EClosest,
            EFOV,
        };
        struct {
            bool bEnable = false;
            struct {
                bool bEnable = false;
                bool bSkeleton = false;
                bool bDrawTeam = false;
                bool bHealth = false;
                bool bName = false;
                EBox boxType = EBox::ENone;
                EBar barType = EBar::ENone;
                ImVec4 enemyColorVis = { 1.f, 0.f, 0.f, 1.f };
                ImVec4 enemyColorInv = { 1.f, 1.f, 0.f, 1.f };
                ImVec4 teamColorVis = { 0.f, 1.f, 0.0f, 1.f };
                ImVec4 teamColorInv = { 0.f, 1.f, 1.f, 1.f };
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } players;
            struct {
                bool bEnable = false;
                bool bSkeleton = false;
                bool bName = false;
                EBox boxType = EBox::ENone;
                EBar barType = EBar::ENone;
                ImVec4 colorVis = { 0.f, 1.f, 0.5f, 1.f };
                ImVec4 colorInv = { 1.f, 0.f, 1.f, 1.f };
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };

            } skeletons;
            struct {
                
                bool bEnable = false;
                bool bSkeleton = false;
                bool bHealth = false;
                bool bName = false;
                bool bDamage = false;
                EShipBox boxType = EShipBox::ENone;
                ImVec4 damageColor = { 1.f, 1.f, 1.f, 1.f };
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
                ImVec4 boxColor = { 1.f, 0.f, 1.f, 1.f };
            } ships;
            struct {
                bool bEnable = false;
                bool bName = false;
                int intMaxDist = 3500;
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } islands;
            struct {
                bool bEnable = false;
                bool bName = false;
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } items;
            struct {
                bool bEnable = false;
                bool bName = false;
                EBox boxType = EBox::ENone;
                ImVec4 colorVis = { 0.f, 1.f, 0.5f, 1.f };
                ImVec4 colorInv = { 0.7f, 1.f, 0.f, 1.f };
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } animals;
            struct {
                bool bEnable = false;
                bool bSkeleton = false;
                bool bName = false;
                ImVec4 colorVis = { 0.f, 1.f, 0.5f, 1.f };
                ImVec4 colorInv = { 0.7f, 1.f, 0.f, 1.f };
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } sharks;
            struct {
                bool bEnable = false;
                bool bName = false;
                bool bDoor = false;
                bool bKeyPlace = false;
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } puzzles;
            struct {
                bool bEnable = false;
                bool bName = false;
                ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
            } shipwrecks;
            struct {
                bool bCrosshair = false;
                bool bOxygen = false;
                bool bCompass = false;
                bool bDebug = false;
                float fCrosshair = 7.f;
                float fDebug = 10.f;
                ImVec4 crosshairColor = { 1.f, 1.f, 1.f, 1.f };
            } client;
        } visuals;
        struct {
            bool bEnable = false;
            struct {
                bool bEnable = false;
                bool bVisibleOnly = false;
                bool bTeam = false;
                float fYaw = 5.f;
                float fPitch = 5.f;
                float fSmoothness = 5.f;
            } players;
            struct {
                bool bEnable = false;
                bool bVisibleOnly = false;
                float fYaw = 5.f;
                float fPitch = 5.f;
                float fSmoothness = 5.f;
            } skeletons;
            struct {
                bool bEnable = false;
                bool bVisibleOnly = false;
                bool bAutoshoot = false;
                float fYaw = 100.f;
                float fPitch = 102.f;
            } harpoon;
            struct {
                bool bEnable = false;
                bool bVisibleOnly = false;
                float fYaw = 100.f;
                float fPitch = 102.f;
            } cannon;
        } aim;

        struct {
            bool bEnable = false;
            struct {
                bool bEnable = false;
                bool bShipInfo = false;
                bool bIdleKick = false;
            } client;
            struct {
                bool bEnable = false;
                bool bShowPlayers = false;
            } game;
            struct {
                bool bEnable = false;
                bool bShowKrakenState = false;
            } kraken;
        } misc;
    } cfg;
    class Hacks {
    private:
        static void OnWeaponFiredHook(UINT64 arg1, UINT64 arg2);
        static inline decltype(OnWeaponFiredHook)* OnWeaponFiredOriginal = nullptr;
    public:
        static inline void Init();
        static inline void Remove();
    };
    class Renderer {
	private:
		struct Drawing {
			static void RenderText(const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined, const bool centered);
			static void Render2DBox(const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color);
			static bool Render3DBox(AController* constcontroller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color);
			static bool RenderSkeleton(AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color);
		};
	private:
        static inline HRESULT(*fnPresent)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
        static inline HRESULT(*fnResize)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
		static inline HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
		static inline HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
		static inline decltype(SetCursorPos)* SetCursorPosOriginal = nullptr;
        static inline decltype(SetCursor)* SetCursorOriginal = nullptr;
		static inline ID3D11Device* device = nullptr;
		static inline ID3D11DeviceContext* context = nullptr;
		static inline ID3D11RenderTargetView* renderTargetView = nullptr;
        static inline bool bIsOpen = false;
		static inline WNDPROC WndProcOriginal = nullptr;
		static inline HWND gameWindow;
	private:
		static LRESULT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static BOOL WINAPI SetCursorPosHook(int X, int Y);
        static HCURSOR WINAPI SetCursorHook(HCURSOR hCursor);
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
		static inline bool FindNameArray();
		static inline bool FindObjectsArray();
		static inline bool FindWorld();
		static inline bool InitSDK();
		
	};
	class Logger {
	private:
		static inline HANDLE file = nullptr;
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
