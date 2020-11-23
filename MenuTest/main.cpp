#define WIN32_LEAN_AND_MEAN  
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <comdef.h>
#include <d3d11.h>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_win32.h>
#include <imgui\imgui_impl_dx11.h>
#include <stdio.h>
#include <imgui\imgui_internal.h>
#include <filesystem>

namespace fs = std::filesystem;

HWND window = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapchain = nullptr;
ID3D11RenderTargetView* view = nullptr;

inline bool CreateView() {
	ID3D11Texture2D* buffer = nullptr;
	if (FAILED(swapchain->GetBuffer(0, __uuidof(buffer), reinterpret_cast<PVOID*>(&buffer)))) return false;
	if (FAILED(device->CreateRenderTargetView(buffer, nullptr, &view))) return false;
	buffer->Release();
	return true;
}

void ShowErrorMsg(const char* lpszFunction, HRESULT hr)
{	
	char* buf = new char[0x200];
	_com_error err(hr);
	sprintf(buf, "%s failed with error 0x%lX: %s", lpszFunction, hr, err.ErrorMessage());
	MessageBoxA(nullptr, buf, "Error", 0);
	delete[] buf;
}

inline bool InitDX()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	HRESULT hr = 0;
	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &context))) {
		ShowErrorMsg("D3D11CreateDeviceAndSwapChain", hr);
		return false;
	}

	if (!CreateView()) 
	{
		if (swapchain) 
		{
			(swapchain)->Release();
			swapchain = nullptr;
		}
		if (device) 
		{
			(device)->Release();
			swapchain = nullptr;
		}
		return false;
	}
	return true;

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
		

	switch (msg)
	{
	case WM_SIZE:
		
		if (device != nullptr && wParam != SIZE_MINIMIZED)
		{
			if (view)
			{
				view->Release();
				view = nullptr;
			}

			swapchain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
			CreateView();
		}
		
		
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

struct Vec2 : public ImVec2 {
	using ImVec2::ImVec2;
	FORCEINLINE float Size() const { return sqrtf(x * x + y* y); }
	FORCEINLINE Vec2 operator*(float Scale) const { return Vec2(x * Scale, y * Scale); }
	FORCEINLINE Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
	FORCEINLINE Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
};


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "ImGui Example", NULL };
	RegisterClassExA(&wc);
	window = CreateWindowExA(0L, wc.lpszClassName, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, wc.hInstance, NULL);
	
	if (!window) 
	{
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	if (!InitDX()) 
	{
		return 1;
	}

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);

	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
	config.RasterizerMultiply = 1.125f;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.0f, &config);
	//io.Fonts->AddFontFromFileTTF(".\\OpenSans-Regular.ttf", 25.0f, &config);

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, context);

	MSG msg{};
	float time = 0.f;
	Vec2 TL;
	while (msg.message != WM_QUIT) {

		if (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			continue;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		static struct Config {
			enum class EBox : int {
				ENone,
				E2DBoxes,
				E3DBoxes,
				EDebugBoxes
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
					ImVec4 damageColor = { 1.f, 1.f, 1.f, 1.f };
					ImVec4 textCol = { 1.f, 1.f, 1.f, 1.f };
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
					float fYaw = 20.f;
					float fPitch = 20.f;
					float fSmoothness = 5.f;
				} players;
				struct {
					bool bEnable = false;
					bool bVisibleOnly = false;
					float fYaw = 20.f;
					float fPitch = 20.f;
					float fSmoothness = 5.f;
				} skeletons;
			} aim;

			struct {
				bool bEnable = false;
				struct {
					bool bEnable = false;
					bool bInfiniteAmmo = false;
					bool bIdleKick = false;
					float fGameSpeed = 1.f;
				} client;
			} misc;
		} cfg;


		static bool bIsOpen = false;
		if (ImGui::IsKeyPressed(VK_INSERT) & 0x1) bIsOpen = !bIsOpen;

		auto drawList = ImGui::GetForegroundDrawList();
		const Vec2 center = { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f };

		float BS = 3.f;
		Vec2 STL = { center.x - 200.f, center.y };
		Vec2 TV = { 4.f, 2.f };
		Vec2 SLL = center;
		Vec2 LV = { 0, 0.f };

		Vec2 RL = SLL - STL;
		Vec2 RV = TV - LV;
		
		
		ImU32 red = ImGui::GetColorU32(IM_COL32(255, 0, 0, 255));
		ImU32 green = ImGui::GetColorU32(IM_COL32(0, 255, 0, 255));
		ImU32 blue = ImGui::GetColorU32(IM_COL32(0, 100, 255, 255));
		ImU32 yellow = ImGui::GetColorU32(IM_COL32(255, 255, 0, 255));

		char buf[250];
		sprintf(buf, "FPS: %f", io.Framerate);
		drawList->AddText({ 50.f, 50.f }, green, buf);

		
		if (TL.x < 0 || TL.y < 0 || TL.x > io.DisplaySize.x || TL.y > io.DisplaySize.y) time = 0.f;
		TL = STL + RV * time;
		drawList->AddCircleFilled(SLL, 3.f, green);
		drawList->AddCircleFilled(TL, 3.f, red);
		drawList->AddLine(SLL, TL, blue);
		const float R = BS * time;
		drawList->AddCircle(SLL, R, yellow);

		if ( fabs((TL - SLL).Size() - R) <= 0.01f) time = 0.f;

		time += 0.005f;

		if (bIsOpen) {
			ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.7f), ImGuiCond_Once);
			ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			if (ImGui::BeginTabBar("Bars")) {
				if (ImGui::BeginTabItem("Visuals")) {

					ImGui::Text("Global Visuals");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.bEnable);
					}
					ImGui::EndChild();


					ImGui::Columns(2, "CLM1", false);
					const char* boxes[] = { "None", "2DBox", "3DBox", "DebugBox" };
					const char* bars[] = { "None", "2DRectLeft", "2DRectRight", "2DRectBottom", "2DRectTop" };
					ImGui::Text("Players");
					if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.players.bEnable);
						ImGui::Checkbox("Draw teammates", &cfg.visuals.players.bDrawTeam);
						ImGui::Checkbox("Draw name", &cfg.visuals.players.bName);
						ImGui::Checkbox("Draw skeleton", &cfg.visuals.players.bSkeleton);
						ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.players.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::Combo("Health bar type", reinterpret_cast<int*>(&cfg.visuals.players.barType), bars, IM_ARRAYSIZE(bars));
						ImGui::ColorEdit4("Visible Enemy color", &cfg.visuals.players.enemyColorVis.x, 0);
						ImGui::ColorEdit4("Invisible Enemy color", &cfg.visuals.players.enemyColorInv.x, 0);
						ImGui::ColorEdit4("Visible Team color", &cfg.visuals.players.teamColorVis.x, 0);
						ImGui::ColorEdit4("Invisible Team color", &cfg.visuals.players.teamColorInv.x, 0);
						ImGui::ColorEdit4("Text color", &cfg.visuals.players.textCol.x, 0);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Skeletons");
					if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.skeletons.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.skeletons.bName);
						ImGui::Checkbox("Draw skeleton", &cfg.visuals.skeletons.bSkeleton);
						ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.skeletons.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::ColorEdit4("Visible Color", &cfg.visuals.skeletons.colorVis.x, 0);
						ImGui::ColorEdit4("Invisible Color", &cfg.visuals.skeletons.colorInv.x, 0);
						ImGui::ColorEdit4("Text color", &cfg.visuals.skeletons.textCol.x, 0);

					}
					ImGui::EndChild();
					ImGui::Columns();





					ImGui::Columns(2, "CLM2", false);

					ImGui::Text("Ships");
					if (ImGui::BeginChild("ShipsSettings", ImVec2(0.f, 220.f), true, 0)) {

						ImGui::Checkbox("Enable", &cfg.visuals.ships.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.ships.bName);
						ImGui::Checkbox("Show holes", &cfg.visuals.ships.bDamage);
						ImGui::ColorEdit4("Damage color", &cfg.visuals.ships.damageColor.x, 0);
						ImGui::ColorEdit4("Text color", &cfg.visuals.ships.textCol.x, 0);

					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Islands");
					if (ImGui::BeginChild("IslandsSettings", ImVec2(0.f, 220.f), true, 0)) {
						ImGui::Checkbox("Enable", &cfg.visuals.islands.bEnable);
						ImGui::Checkbox("Draw names", &cfg.visuals.islands.bName);
						ImGui::SliderInt("Max distance", &cfg.visuals.islands.intMaxDist, 100, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
						ImGui::ColorEdit4("Text color", &cfg.visuals.islands.textCol.x, 0);
					}
					ImGui::EndChild();
					ImGui::Columns();



					ImGui::Columns(2, "CLM3", false);
					ImGui::Text("Items");
					if (ImGui::BeginChild("ItemsSettings", ImVec2(0.f, 220.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.items.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.items.bName);
						ImGui::ColorEdit4("Text color", &cfg.visuals.items.textCol.x, 0);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Animals");
					if (ImGui::BeginChild("AnimalsSettings", ImVec2(0.f, 220.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.animals.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.animals.bName);
						ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.animals.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::ColorEdit4("Visible Color", &cfg.visuals.animals.colorVis.x, 0);
						ImGui::ColorEdit4("Invisible Color", &cfg.visuals.animals.colorInv.x, 0);
						ImGui::ColorEdit4("Text color", &cfg.visuals.animals.textCol.x, 0);
					}

					ImGui::EndChild();
					ImGui::Columns();

					ImGui::Columns(2, "CLM4", false);
					ImGui::Text("Sharks");
					if (ImGui::BeginChild("SharksSettings", ImVec2(0.f, 220.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.sharks.bEnable);
						ImGui::Checkbox("Draw skeleton", &cfg.visuals.sharks.bSkeleton);
						ImGui::Checkbox("Draw name", &cfg.visuals.sharks.bName);
						//ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.sharks.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::ColorEdit4("Visible Color", &cfg.visuals.sharks.colorVis.x, 0);
						ImGui::ColorEdit4("Invisible Color", &cfg.visuals.sharks.colorInv.x, 0);
						ImGui::ColorEdit4("Text color", &cfg.visuals.sharks.textCol.x, 0);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Puzzles");
					if (ImGui::BeginChild("PuzzlesSettings", ImVec2(0.f, 220.f), true, 0))
					{

						ImGui::Checkbox("Enable", &cfg.visuals.puzzles.bEnable);
						ImGui::Checkbox("Draw doors", &cfg.visuals.puzzles.bDoor);
						ImGui::ColorEdit4("Text color", &cfg.visuals.puzzles.textCol.x, 0);

					}
					ImGui::EndChild();
					ImGui::Columns();

					ImGui::Columns(2, "CLM5", false);

					ImGui::Text("Client");
					if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 220.f), true, 0))
					{

						ImGui::Checkbox("Crosshair", &cfg.visuals.client.bCrosshair);
						if (cfg.visuals.client.bCrosshair)
						{
							ImGui::SameLine();
							ImGui::SetNextItemWidth(75.f);
							ImGui::SliderFloat("Radius##1", &cfg.visuals.client.fCrosshair, 1.f, 100.f);
						}



						ImGui::Checkbox("Oxygen level", &cfg.visuals.client.bOxygen);
						ImGui::Checkbox("Compass", &cfg.visuals.client.bCompass);

						ImGui::Checkbox("Debug", &cfg.visuals.client.bDebug);
						if (cfg.visuals.client.bDebug)
						{
							ImGui::SameLine();
							ImGui::SetNextItemWidth(150.f);
							ImGui::SliderFloat("Radius##2", &cfg.visuals.client.fDebug, 1.f, 1000.f);
						}
						ImGui::ColorEdit4("Crosshair color", &cfg.visuals.client.crosshairColor.x, ImGuiColorEditFlags_DisplayRGB);

					}

					ImGui::EndChild();
					ImGui::Columns();


					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Aim")) {

					ImGui::Text("Global Aim");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.aim.bEnable);
					}
					ImGui::EndChild();


					ImGui::Columns(2, "CLM1", false);
					ImGui::Text("Players");
					if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 365.f), true, 0))
					{
						// todo: add bones selection
						ImGui::Checkbox("Enable", &cfg.aim.players.bEnable);
						ImGui::Checkbox("Visible only", &cfg.aim.players.bVisibleOnly);
						ImGui::Checkbox("Aim at teammates", &cfg.aim.players.bTeam);
						ImGui::SliderFloat("Yaw", &cfg.aim.players.fYaw, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Pitch", &cfg.aim.players.fPitch, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Smoothness", &cfg.aim.players.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Skeletons");
					if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.aim.skeletons.bEnable);
						ImGui::Checkbox("Visible only", &cfg.aim.skeletons.bVisibleOnly);
						ImGui::SliderFloat("Yaw", &cfg.aim.skeletons.fYaw, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Pitch", &cfg.aim.skeletons.fPitch, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Smoothness", &cfg.aim.skeletons.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);


					}
					ImGui::EndChild();
					ImGui::Columns();



					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Misc")) {

					ImGui::Text("Global Misc");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.misc.bEnable);
					}
					ImGui::EndChild();

					ImGui::Columns(2, "CLM1", false);
					ImGui::Text("Client");
					if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.misc.client.bEnable);
						ImGui::Checkbox("Disable idle kick", &cfg.misc.client.bIdleKick);
						ImGui::Separator();
						if (ImGui::Button("Save settings"))
						{
							do {
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(GetModuleHandleA(0), buf, MAX_PATH);
								fs::path path = fs::path(buf).remove_filename() / "menu.cfg";
								auto file = CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (file == INVALID_HANDLE_VALUE) break;
								DWORD written;
								if (WriteFile(file, &cfg, sizeof(cfg), &written, 0)) ImGui::OpenPopup("##SettingsSaved");
								CloseHandle(file);
							} while (false);
						}
						ImGui::SameLine();
						if (ImGui::Button("Load settings"))
						{
							do {
								wchar_t buf[MAX_PATH];
								GetModuleFileNameW(GetModuleHandleA(0), buf, MAX_PATH);
								fs::path path = fs::path(buf).remove_filename() / "menu.cfg";
								auto file = CreateFileW(path.wstring().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
								if (file == INVALID_HANDLE_VALUE) break;
								DWORD readed;
								if (ReadFile(file, &cfg, sizeof(cfg), &readed, 0))  ImGui::OpenPopup("##SettingsLoaded");
								CloseHandle(file);
							} while (false);
						}
						ImGui::SameLine();
						if (ImGui::Button("Tests")) {
							/*auto h = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Tests), nullptr, 0, nullptr);
							if (h) CloseHandle(h);*/
						}

						ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
						ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						if (ImGui::BeginPopupModal("##SettingsSaved", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
						{
							ImGui::Text("\nSettings have been saved\n\n");
							ImGui::Separator();
							if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}
						ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
						if (ImGui::BeginPopupModal("##SettingsLoaded", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
						{
							ImGui::Text("\nSettings have been loaded\n\n");
							ImGui::Separator();
							if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}
					}
					ImGui::EndChild();


					ImGui::Columns();


					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			};
			ImGui::End();
		}

		ImGui::Render();
		context->OMSetRenderTargets(1, &view, NULL);
		const float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
		context->ClearRenderTargetView(view, clearColor);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}


	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


	if (view) {
		view->Release();
		view = nullptr;
	}
	if (swapchain)
	{
		swapchain->Release();
		swapchain = nullptr;
	}
	if (context)
	{
		context->Release();
		context = nullptr;
	}
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (DestroyWindow(window))
	{
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
	};
	


	return 0;
}