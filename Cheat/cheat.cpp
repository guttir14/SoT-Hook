#include "cheat.h"
#include <filesystem>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_win32.h>
#include <HookLib/HookLib.h>


#define STEAM 1

#if not STEAM
#if 0
#define UWPDEBUG
#endif
#endif

namespace fs = std::filesystem;

void Cheat::Hacks::OnWeaponFiredHook(UINT64 arg1, UINT64 arg2)
{
    Logger::Log("arg1: %p, arg2: %p\n", arg1, arg2);
    auto& cameraCache = cache.localCamera->CameraCache.POV;
    auto prev = cameraCache.Rotation;
    cameraCache.Rotation = { -cameraCache.Rotation.Pitch, -cameraCache.Rotation.Yaw, 0.f };
    return OnWeaponFiredOriginal(arg1, arg2);
}



void Cheat::Hacks::Init()
{
    /*UFunction* fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.OnWeaponFired");
    if (fn) {
        if (SetHook(fn->Func, OnWeaponFiredHook, reinterpret_cast<void**>(&OnWeaponFiredOriginal)))
        {
            Logger::Log("StartFire: %p\n", fn->Func);
        }
    }*/
}

inline void Cheat::Hacks::Remove()
{
    //RemoveHook(orig);
}

void Cheat::Renderer::Drawing::RenderText(const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined = true, const bool centered = true)
{
    if (!text) return;
    auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
    if (centered)
    {
        auto size = ImGui::CalcTextSize(text);
        ImScreen.x -= size.x * 0.5f;
        ImScreen.y -= size.y;
    }
    auto window = ImGui::GetCurrentWindow();

    if (outlined) { window->DrawList->AddText(nullptr, 0.f, ImVec2(ImScreen.x - 1.f, ImScreen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text); }

    window->DrawList->AddText(nullptr, 0.f, ImScreen, ImGui::GetColorU32(color), text);

}

void Cheat::Renderer::Drawing::Render2DBox(const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color)
{
    ImGui::GetCurrentWindow()->DrawList->AddRect({ top.X - width * 0.5f, top.Y}, { top.X + width * 0.5f, bottom.Y }, ImGui::GetColorU32(color), 0.f, 15, 1.5f);
}

bool Cheat::Renderer::Drawing::Render3DBox(AController* const controller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color)
{
    FVector vertex[2][4];
    vertex[0][0] = { -extent.X, -extent.Y,  -extent.Z };
    vertex[0][1] = { extent.X, -extent.Y,  -extent.Z };
    vertex[0][2] = { extent.X, extent.Y,  -extent.Z };
    vertex[0][3] = { - extent.X, extent.Y, -extent.Z };

    vertex[1][0] = { -extent.X, -extent.Y, extent.Z };
    vertex[1][1] = { extent.X, -extent.Y, extent.Z };
    vertex[1][2] = { extent.X, extent.Y, extent.Z };
    vertex[1][3] = { -extent.X, extent.Y, extent.Z };

    FVector2D screen[2][4];
    FTransform const Transform(rotation);
    for (auto k = 0; k < 2; k++)
    {
        for (auto i = 0; i < 4; i++)
        {
            auto& vec = vertex[k][i];
            vec = Transform.TransformPosition(vec) + origin;
            if (!controller->ProjectWorldLocationToScreen(vec, screen[k][i])) return false;
        }

    }

    auto ImScreen = reinterpret_cast<ImVec2(&)[2][4]>(screen);
    
    auto window = ImGui::GetCurrentWindow();
    for (auto i = 0; i < 4; i++)
    {
        window->DrawList->AddLine(ImScreen[0][i], ImScreen[0][(i + 1) % 4], ImGui::GetColorU32(color));
        window->DrawList->AddLine(ImScreen[1][i], ImScreen[1][(i + 1) % 4], ImGui::GetColorU32(color));
        window->DrawList->AddLine(ImScreen[0][i], ImScreen[1][i], ImGui::GetColorU32(color));
    }

    return true;
}

bool Cheat::Renderer::Drawing::RenderSkeleton(AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color)
{
    
    for (auto s = 0; s < size; s++)
    {
        auto& bone = skeleton[s];
        FVector2D previousBone;
        for (auto i = 0; i < skeleton[s].second; i++)
        {
            FVector loc;
            if (!mesh->GetBone(bone.first[i], comp2world, loc)) return false;
            FVector2D screen;
            if (!controller->ProjectWorldLocationToScreen(loc, screen)) return false;
            if (previousBone.Size() != 0) {
                auto ImScreen1 = *reinterpret_cast<ImVec2*>(&previousBone);
                auto ImScreen2 = *reinterpret_cast<ImVec2*>(&screen);
                ImGui::GetCurrentWindow()->DrawList->AddLine(ImScreen1, ImScreen2, ImGui::GetColorU32(color));
            }
            previousBone = screen;
        }
    }
    
    return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI Cheat::Renderer::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) && bIsOpen) return true;
    if (bIsOpen)
    {
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        SetCursorOriginal(LoadCursorA(nullptr, win32_cursor));
        
    }
    if (!bIsOpen || uMsg == WM_KEYUP) return CallWindowProcA(WndProcOriginal, hwnd, uMsg, wParam, lParam);
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

HCURSOR WINAPI Cheat::Renderer::SetCursorHook(HCURSOR hCursor)
{
   if (bIsOpen) return 0;
   return SetCursorOriginal(hCursor);
}

BOOL WINAPI Cheat::Renderer::SetCursorPosHook(int X, int Y)
{
    if (bIsOpen) return FALSE;
    return SetCursorPosOriginal(X, Y);
}

void Cheat::Renderer::HookInput()
{
    RemoveInput();
    WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
    Logger::Log("WndProcOriginal = %p\n", WndProcOriginal);
}

void Cheat::Renderer::RemoveInput()
{
    if (WndProcOriginal) 
    {
        SetWindowLongPtrA(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcOriginal));
        WndProcOriginal = nullptr;
    }
}

HRESULT Cheat::Renderer::PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{ 
    if (!device)
    {
        ID3D11Texture2D* surface = nullptr;
        goto init;
    cleanup:
        Cheat::Remove();
        if (surface) surface->Release();
        return fnPresent(swapChain, syncInterval, flags);
    init:

        if (FAILED(swapChain->GetBuffer(0, __uuidof(surface), reinterpret_cast<PVOID*>(&surface))))  { goto cleanup; };
       Logger::Log("ID3D11Texture2D* surface = %p\n", surface); 

        if (FAILED(swapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device)))) goto cleanup;

        Logger::Log("ID3D11Device* device = %p\n", device);

        if (FAILED(device->CreateRenderTargetView(surface, nullptr, &renderTargetView))) goto cleanup;

       Logger::Log("ID3D11RenderTargetView* renderTargetView = %p\n", renderTargetView);

        surface->Release();
        surface = nullptr;

        device->GetImmediateContext(&context);

        Logger::Log("ID3D11DeviceContext* context = %p\n", context);

        ImGui::CreateContext();

        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontConfig config;
            config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
            config.RasterizerMultiply = 1.125f;
            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.0f, &config);
            io.IniFilename = nullptr;
        }
#ifdef STEAM
        DXGI_SWAP_CHAIN_DESC desc;
        swapChain->GetDesc(&desc);
        auto& window = desc.OutputWindow;
        gameWindow = window;
#else
        auto window = FindWindowA("Windows.UI.Core.CoreWindow", "Sea of Thieves");
        gameWindow = window;    
#endif
        Logger::Log("gameWindow = %p\n", window);

        if (!ImGui_ImplWin32_Init(window)) goto cleanup;
        if (!ImGui_ImplDX11_Init(device, context)) goto cleanup;
        if (!ImGui_ImplDX11_CreateDeviceObjects()) goto cleanup;
        Logger::Log("ImGui initialized successfully!\n");

        HookInput();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    
    ImGui::Begin("#1", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    auto& io = ImGui::GetIO();
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);
   
    auto drawList = ImGui::GetCurrentWindow()->DrawList; 
    
    try 
    {
        do 
        {
            memset(&cache, 0, sizeof(Cache));
            auto const world = *UWorld::GWorld;
           
            if (!world) break;
            auto const game = world->GameInstance;
            if (!game) break;
            auto const gameState = world->GameState;
            if (!gameState) break;
            cache.gameState = gameState;
            if (!game->LocalPlayers.Data) break;
            
            auto const localPlayer = game->LocalPlayers[0];
            if (!localPlayer) break;
            auto const localController = localPlayer->PlayerController;
            if (!localController) break;
            cache.localController = localController;
            auto const camera = localController->PlayerCameraManager;
            if (!camera) break;
            cache.localCamera = camera;
            const auto cameraLoc = camera->GetCameraLocation();
            const auto cameraRot = camera->GetCameraRotation();
            auto const localCharacter = localController->Character;
            if (!localCharacter) break;
            const auto levels = world->Levels;
            if (!levels.Data) break;
            const auto localLoc = localCharacter->K2_GetActorLocation();
           
            bool isWieldedWeapon = false;
            auto item = localCharacter->GetWieldedItem();
            if (item) isWieldedWeapon = item->isWeapon();

           // check isWieldedWeapon before accessing!
           auto const localWeapon = *reinterpret_cast<AProjectileWeapon**>(&item);
           ACharacter* attachObject = localCharacter->GetAttachParentActor();;
           bool isHarpoon = false;
           if (attachObject)
           {
               if (cfg.aim.harpoon.bEnable && attachObject->isHarpoon()) { isHarpoon = true; }
           }

           cache.good = true;

            static struct {
                ACharacter* target = nullptr;
                FVector location;
                FRotator delta;
                float best = FLT_MAX;
                float smoothness = 1.f;
            } aimBest;

            aimBest.target = nullptr;
            aimBest.best = FLT_MAX;

            for (auto l = 0u; l < levels.Count; l++)
            {
                auto const level = levels[l];
                if (!level) continue;
                const auto actors = level->AActors;
                if (!actors.Data) continue;

                // todo: make functions for similar code 
                for (auto a = 0u; a < actors.Count; a++)
                {
                    auto const actor = actors[a];
                    if (!actor) continue;
                    if (cfg.aim.bEnable)
                    {
                        
                        if (isHarpoon)
                        {
                            if (actor->isItem())
                            {
                                do {
                                
                                    FVector location = actor->K2_GetActorLocation();
                                    float dist = cameraLoc.DistTo(location);
                                    if (dist > 7500.f || dist < 260.f) { break; }
                                    if (cfg.aim.harpoon.bVisibleOnly) if (!localController->LineOfSightTo(actor, cameraLoc, false)) { break; }
                                    auto harpoon = reinterpret_cast<AHarpoonLauncher*>(attachObject);
                                    auto center = UKismetMathLibrary::NormalizedDeltaRotator(cameraRot, harpoon->rotation);
                                    FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, location), center);
                                    if (delta.Pitch < -35.f || delta.Pitch > 67.f || abs(delta.Yaw) > 50.f) { break; }
                                    FRotator diff = delta - harpoon->rotation;
                                    float absPitch = abs(diff.Pitch);
                                    float absYaw = abs(diff.Yaw);
                                    if (absPitch > cfg.aim.harpoon.fPitch || absYaw > cfg.aim.harpoon.fYaw) { break; }
                                    float sum = absYaw + absPitch;
                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = location;
                                        aimBest.delta = delta;
                                        aimBest.best = sum;
                                    }

                                } while (false);
                            }
                        }
                        else if (!attachObject && isWieldedWeapon)
                        {
                            if (cfg.aim.players.bEnable && actor->isPlayer() && actor != localCharacter && !actor->IsDead())
                            {
                                do {

                                    FVector playerLoc = actor->K2_GetActorLocation();
                                    float dist = localLoc.DistTo(playerLoc);
                                    if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange) { break; }

                                    if (cfg.aim.players.bVisibleOnly) if (!localController->LineOfSightTo(actor, cameraLoc, false)) { break; }
                                    if (!cfg.aim.players.bTeam) if (UCrewFunctions::AreCharactersInSameCrew(actor, localCharacter)) break;

                                    FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, playerLoc), cameraRot);

                                    float absYaw = abs(rotationDelta.Yaw);
                                    float absPitch = abs(rotationDelta.Pitch);
                                    if (absYaw > cfg.aim.players.fYaw || absPitch > cfg.aim.players.fPitch) { break; }
                                    float sum = absYaw + absPitch;

                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = playerLoc;
                                        aimBest.delta = rotationDelta;
                                        aimBest.best = sum;
                                        aimBest.smoothness = cfg.aim.players.fSmoothness;
                                    }

                                } while (false);
                            }
                            else if (cfg.aim.skeletons.bEnable && actor->isSkeleton() && !actor->IsDead())
                            {
                                do {
                                    const FVector playerLoc = actor->K2_GetActorLocation();
                                    const float dist = localLoc.DistTo(playerLoc);

                                    if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange) break;
                                    if (cfg.aim.skeletons.bVisibleOnly) if (!localController->LineOfSightTo(actor, cameraLoc, false)) break;

                                    const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, playerLoc), cameraRot);

                                    const float absYaw = abs(rotationDelta.Yaw);
                                    const float absPitch = abs(rotationDelta.Pitch);
                                    if (absYaw > cfg.aim.skeletons.fYaw || absPitch > cfg.aim.skeletons.fPitch) break;
                                    const float sum = absYaw + absPitch;

                                    if (sum < aimBest.best)
                                    {
                                        aimBest.target = actor;
                                        aimBest.location = playerLoc;
                                        aimBest.delta = rotationDelta;
                                        aimBest.best = sum;
                                        aimBest.smoothness = cfg.aim.skeletons.fSmoothness;
                                    }

                                } while (false);
                            }
                        }
                        
                    }
                    if (cfg.visuals.bEnable)
                    {
                        if (cfg.visuals.client.bDebug)
                        {
                            const FVector location = actor->K2_GetActorLocation();
                            const float dist = localLoc.DistTo(location) * 0.01f;
                            if (dist < cfg.visuals.client.fDebug)
                            {
                                auto const actorClass = actor->Class;
                                if (!actorClass) continue;
                                auto super = actorClass->SuperField;
                                if (!super) continue;
                                FVector2D screen;
                                if (localController->ProjectWorldLocationToScreen(location, screen))
                                {
                                    auto superName = super->GetNameFast();
                                    auto className = actorClass->GetNameFast();
                                    if (superName && className)
                                    {
                                        char buf[0x128];
                                        sprintf_s(buf, "%s %s [%d] (%p)", className, superName, (int)dist, actor);
                                        Drawing::RenderText(buf, screen, ImVec4(1.f, 1.f, 1.f, 1.f));
                                    }
                                }
                            }
                        }
                        else {

                            if (cfg.visuals.items.bEnable && actor->isItem()) {

                                if (cfg.visuals.items.bName)
                                {
                                    auto location = actor->K2_GetActorLocation();
                                    FVector2D screen;
                                    if (localController->ProjectWorldLocationToScreen(location, screen))
                                    {
                                        auto const desc = actor->GetItemInfo()->Desc;
                                        if (!desc) continue;
                                        const int dist = localLoc.DistTo(location) * 0.01f;
                                        char name[0x64];
                                        const int len = desc->Title->multi(name, 0x50);
                                        snprintf(name + len, sizeof(name) - len, " [%d]", dist);
                                        Drawing::RenderText(name, screen, cfg.visuals.items.textCol);
                                    };
                                }

                                continue;
                            }

                            else if (cfg.visuals.shipwrecks.bEnable && actor->isShipwreck())
                            {
                                auto location = actor->K2_GetActorLocation();
                                FVector2D screen;
                                if (localController->ProjectWorldLocationToScreen(location, screen))
                                {
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    char name[0x64];
                                    sprintf_s(name, "Shipwreck [%d]", dist);
                                    Drawing::RenderText(name, screen, cfg.visuals.items.textCol);
                                };
                                continue;
                            }

                            else if (cfg.visuals.players.bEnable && actor->isPlayer()  && actor != localCharacter && !actor->IsDead())
                            {

                                const bool teammate = UCrewFunctions::AreCharactersInSameCrew(actor, localCharacter);
                                if (teammate && !cfg.visuals.players.bDrawTeam) continue;

                                FVector origin, extent;
                                actor->GetActorBounds(true, origin, extent);
                                const FVector location = actor->K2_GetActorLocation();

                                FVector2D headPos;
                                if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos)) continue;
                                FVector2D footPos;
                                if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos)) continue;

                                const float height = abs(footPos.Y - headPos.Y);
                                const float width = height * 0.4f;

                                

                                const bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                                ImVec4 col;
                                if (teammate) col = bVisible ? cfg.visuals.players.teamColorVis : cfg.visuals.players.teamColorInv;
                                else  col = bVisible ? cfg.visuals.players.enemyColorVis : cfg.visuals.players.enemyColorInv;
                               
                                switch (cfg.visuals.players.boxType)
                                {
                                case Config::EBox::E2DBoxes: 
                                {
                                    Drawing::Render2DBox(headPos, footPos, height, width, col);
                                    break;
                                }
                                case Config::EBox::E3DBoxes: 
                                {
                                    FRotator rotation = actor->K2_GetActorRotation();
                                    FVector ext = { 35.f, 35.f, extent.Z };
                                    if (!Drawing::Render3DBox(localController, location, ext, rotation, col)) continue;
                                    break;
                                }
                                /*
                                case Config::EBox::EDebugBoxes: 
                                {
                                    FVector ext = { 35.f, 35.f, extent.Z };
                                    UKismetMathLibrary::DrawDebugBox(actor, location, ext, *reinterpret_cast<FLinearColor*>(&col), actor->K2_GetActorRotation(), 0.0f);
                                    break;
                                }
                                */
                                }


                                if (cfg.visuals.players.bName)
                                {

                                    auto const playerState = actor->PlayerState;
                                    if (!playerState) continue;
                                    const auto playerName = playerState->PlayerName;
                                    if (!playerName.Data) continue;
                                    char name[0x30];
                                    const int len = playerName.multi(name, 0x20);
                                    const int dist = localLoc.DistTo(origin) * 0.01f;
                                    snprintf(name + len, sizeof(name) - len, " [%d]", dist);
                                    const float adjust = height * 0.05f;
                                    FVector2D pos = { headPos.X, headPos.Y - adjust };
                                    Drawing::RenderText(name, pos, cfg.visuals.players.textCol);
                                }

                                if (cfg.visuals.players.barType != Config::EBar::ENone)
                                {
                                    auto const healthComp = actor->HealthComponent;
                                    if (!healthComp) continue;
                                    const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
                                    const float width2 = width * 0.5f;
                                    const float adjust = height * 0.025f;
                                    switch (cfg.visuals.players.barType)
                                    {
                                    case Config::EBar::ELeft: 
                                    {
                                        const float len = height * hp;
                                        drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::ERight:
                                    {
                                        const float len = height * hp;
                                        drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::EBottom:
                                    {
                                        const float len = width * hp;
                                        drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::ETop:
                                    {
                                        const float len = width * hp;
                                        drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        break;
                                    }
                                    }
          
                                }
                            
                                if (cfg.visuals.players.bSkeleton)
                                {
                                    auto const mesh = actor->Mesh;
                                    if (!actor->Mesh) continue;


                                    const BYTE bodyHead[] = { 4, 5, 6, 51, 7, 6, 80, 7, 8, 9 };
                                    const BYTE neckHandR[] = { 80, 81, 82, 83, 84 };
                                    const BYTE neckHandL[] = { 51, 52, 53, 54, 55 };
                                    const BYTE bodyFootR[] = { 4, 111, 112, 113, 114 };
                                    const BYTE bodyFootL[] = { 4, 106, 107, 108, 109 };

                                    const std::pair<const BYTE*, const BYTE> skeleton[] = { {bodyHead, 10}, {neckHandR, 5}, {neckHandL, 5}, {bodyFootR, 5}, {bodyFootL, 5} };


                                    
                                    const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();

                                    if (!Drawing::RenderSkeleton(localController, mesh, comp2world, skeleton, 5, col)) continue;

                                   
                                }
                            

                                continue;
                            
                                
                            
                            }
                       
                            else if (cfg.visuals.skeletons.bEnable && actor->isSkeleton() && !actor->IsDead()) {
                                // todo: make a function to draw both skeletons and players as they are similar
                                FVector origin, extent;
                                actor->GetActorBounds(true, origin, extent);
                            
                                const FVector location = actor->K2_GetActorLocation();
                                FVector2D headPos;
                                if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos)) continue;
                                FVector2D footPos;
                                if (!localController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos)) continue;

                                const float height = abs(footPos.Y - headPos.Y);
                                const float width = height * 0.4f;

                                const bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                                const ImVec4 col = bVisible ? cfg.visuals.skeletons.colorVis : cfg.visuals.skeletons.colorInv;
                               
                                if (cfg.visuals.skeletons.bSkeleton)
                                {
                                    auto const mesh = actor->Mesh;
                                    if (!actor->Mesh) continue;

                                    const BYTE bodyHead[] = { 4, 5, 6, 7, 8, 9 };
                                    const BYTE neckHandR[] = { 7, 41, 42, 43 };
                                    const BYTE neckHandL[] = { 7, 12, 13, 14 };
                                    const BYTE bodyFootR[] = { 4, 71, 72, 73, 74 };
                                    const BYTE bodyFootL[] = { 4, 66, 67, 68, 69 };

                                    const std::pair<const BYTE*, const BYTE> skeleton[] = { {bodyHead, 6}, {neckHandR, 4}, {neckHandL, 4}, {bodyFootR, 5}, {bodyFootL, 5} };

                                    const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();

                                    /*for (auto i = 0; i < 122; i++)
                                    {
                                        FVector pos;
                                        if (mesh->GetBone(i, comp2world, pos))
                                        {
                                            FVector2D screen;
                                            if (!localController->ProjectWorldLocationToScreen(pos, screen)) continue;
                                            char text[0x30];
                                            auto len = sprintf_s(text, "%d", i);
                                            Drawing::RenderText(text, screen, ImVec4(1.f, 1.f, 1.f, 1.f));
                                        };
                                    }*/

                                    if (!Drawing::RenderSkeleton(localController, mesh, comp2world, skeleton, 5, col)) continue;

                                    
                                }

                                switch (cfg.visuals.skeletons.boxType)
                                {
                                case Config::EBox::E2DBoxes:
                                {
                                    Drawing::Render2DBox(headPos, footPos, height, width, col);
                                    break;
                                }
                                case Config::EBox::E3DBoxes:
                                {
                                    FRotator rotation = actor->K2_GetActorRotation();
                                    if (!Drawing::Render3DBox(localController, origin, extent, rotation, col)) continue;
                                    break;
                                }
                                /*
                                case Config::EBox::EDebugBoxes:
                                {
                                    UKismetMathLibrary::DrawDebugBox(actor, origin, extent, *reinterpret_cast<const FLinearColor*>(&col), actor->K2_GetActorRotation(), 0.0f);
                                    break;
                                }
                                */
                                }

                                if (cfg.visuals.skeletons.bName)
                                {
                                    const int dist = localLoc.DistTo(location) * 0.01f;
                                    char name[0x64];
                                    sprintf_s(name, "Skeleton [%d]", dist);
                                    Drawing::RenderText(name, headPos, cfg.visuals.skeletons.textCol);
                                }

                                if (cfg.visuals.skeletons.barType != Config::EBar::ENone)
                                {
                                    auto const healthComp = actor->HealthComponent;
                                    if (!healthComp) continue;
                                    const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
                                    const float width2 = width * 0.5f;
                                    const float adjust = height * 0.025f;

                                    switch (cfg.visuals.skeletons.barType)
                                    {
                                    case Config::EBar::ELeft:
                                    {
                                        const float len = height * hp;
                                        drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::ERight:
                                    {
                                        const float len = height * hp;
                                        drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::EBottom:
                                    {
                                        const float len = width * hp;
                                        drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        break;
                                    }
                                    case Config::EBar::ETop:
                                    {
                                        const float len = width * hp;
                                        drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
                                        drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
                                        break;
                                    }
                                    }

                                }

                                
                                continue;
                            }

                            else if (cfg.visuals.animals.bEnable && actor->isAnimal())
                            {
                                FVector origin, extent;
                                actor->GetActorBounds(true, origin, extent);
                               
                                FVector2D headPos;
                                if (!localController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) continue;
                                FVector2D footPos;
                                if (!localController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) continue;

                                float height = abs(footPos.Y - headPos.Y);
                                float width = height * 0.6f;

                                bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                                ImVec4 col = bVisible ? cfg.visuals.animals.colorVis : cfg.visuals.animals.colorInv;

                                switch (cfg.visuals.animals.boxType)
                                {
                                case Config::EBox::E2DBoxes:
                                {
                                    Drawing::Render2DBox(headPos, footPos, height, width, col);
                                    break;
                                }
                                case Config::EBox::E3DBoxes:
                                {
                                    FRotator rotation = actor->K2_GetActorRotation();
                                    FVector ext = { 40.f, 40.f, extent.Z };
                                    if (!Drawing::Render3DBox(localController, origin, ext, rotation, col)) continue;
                                    break;
                                }
                                /*
                                case Config::EBox::EDebugBoxes:
                                {
                                    FVector ext = { 40.f, 40.f, extent.Z };
                                    UKismetMathLibrary::DrawDebugBox(actor, origin, ext, *reinterpret_cast<const FLinearColor*>(&col), actor->K2_GetActorRotation(), 0.0f);
                                    break;
                                }
                                 */
                                }

                                if (cfg.visuals.animals.bName)
                                {

                                    auto displayName = reinterpret_cast<AFauna*>(actor)->DisplayName;
                                    if (displayName) {
                                        const int dist = localLoc.DistTo(origin) * 0.01f;
                                        char name[0x64];
                                        const int len = displayName->multi(name, 0x50);
                                        snprintf(name + len, sizeof(name) - len, " [%d]", dist);
                                        const float adjust = height * 0.05f;
                                        FVector2D pos = { headPos.X, headPos.Y - adjust };
                                        Drawing::RenderText(name, pos, cfg.visuals.animals.textCol);
                                    }
                                }

                                continue;
                            }
                            else if (cfg.visuals.sharks.bEnable && actor->isShark())
                            {
                                FVector origin, extent;
                                actor->GetActorBounds(true, origin, extent);

                                FVector2D headPos;
                                if (!localController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) continue;
                                FVector2D footPos;
                                if (!localController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) continue;

                                const float height = abs(footPos.Y - headPos.Y);
                                const float width = height * 0.6f;

                                const bool bVisible = localController->LineOfSightTo(actor, cameraLoc, false);
                                const ImVec4 col = bVisible ? cfg.visuals.animals.colorVis : cfg.visuals.animals.colorInv;


                                if (cfg.visuals.sharks.bSkeleton)
                                {
                                    auto const shark = reinterpret_cast<ASharkPawn*>(actor);
                                    auto const mesh = shark->Mesh;
                                    if (!actor->Mesh) continue;
                                    const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();
                                    switch (shark->SwimmingCreatureType)
                                    {
                                    case ESwimmingCreatureType::Shark: 
                                    {
                                        const BYTE bone1[] = { 17, 16, 5, 6, 7, 8, 9, 10, 11, 12 };
                                        const BYTE bone2[] = { 10, 13, 14 };
                                        const BYTE bone3[] = { 5, 18, 19 };
                                        const BYTE bone4[] = { 6, 15, 7 };
                                        const std::pair<const BYTE*, const BYTE> skeleton[] = { {bone1, 10}, {bone2, 3}, {bone3, 3}, {bone4, 3} };
                                        if (!Drawing::RenderSkeleton(localController, mesh, comp2world, skeleton, 4, col)) continue;
                                        break;
                                    }
                                    case ESwimmingCreatureType::TinyShark:
                                    {
                                        const BYTE bone1[] = { 26, 25, 24, 23, 22, 21, 20, 19 };
                                        const BYTE bone2[] = { 28, 27, 24 };
                                        const BYTE bone3[] = { 33, 32, 31, 21, 34, 35, 36 };
                                        const std::pair<const BYTE*, const BYTE> skeleton[] = { {bone1, 8}, {bone2, 3}, {bone3, 7}};
                                        if (!Drawing::RenderSkeleton(localController, mesh, comp2world, skeleton, 3, col)) continue;
                                        break;
                                    }
                                    }
                                   
                                    
                                }

                                if (cfg.visuals.sharks.bName)
                                {
                                    char name[0x20];
                                    const int dist = localLoc.DistTo(origin) * 0.01f;
                                    sprintf_s(name, "Shark [%d]", dist);
                                    const float adjust = height * 0.05f;
                                    FVector2D pos = { headPos.X, headPos.Y - adjust };
                                    Drawing::RenderText(name, pos, cfg.visuals.sharks.textCol);
                                }

                                continue;
                            }
                            else if (cfg.visuals.ships.bEnable)
                            {
                                if (actor->isShip()) 
                                {
                                    const FVector location = actor->K2_GetActorLocation();
                                    const int dist = localLoc.DistTo(location) * 0.01f;

                                    if (cfg.visuals.ships.bName && dist <= 1500)
                                    {
                                        FVector2D screen;
                                        if (localController->ProjectWorldLocationToScreen(location, screen)) {
                                            int amount = 0;
                                            auto water = actor->GetInternalWater();
                                            if (water) amount = water->GetNormalizedWaterAmount() * 100.f;
                                            char name[0x40];
                                            sprintf_s(name, "Ship (%d%%) [%d]", amount, dist);
                                            Drawing::RenderText(const_cast<char*>(name), screen, cfg.visuals.ships.textCol);
                                        };
                                    }

                                    if (cfg.visuals.ships.bDamage && dist <= 300)
                                    {
                                        auto const damage = actor->GetHullDamage();
                                        if (!damage) continue;
                                        const auto holes = damage->ActiveHullDamageZones;
                                        for (auto h = 0u; h < holes.Count; h++)
                                        {
                                            auto const hole = holes[h];
                                            const FVector location = hole->K2_GetActorLocation();
                                            FVector2D screen;
                                            if (localController->ProjectWorldLocationToScreen(location, screen))
                                            {
                                                auto color = cfg.visuals.ships.damageColor;
                                                drawList->AddLine({ screen.X - 6.f, screen.Y + 6.f }, { screen.X + 6.f, screen.Y - 6.f }, ImGui::GetColorU32(color));
                                                drawList->AddLine({ screen.X - 6.f, screen.Y - 6.f }, { screen.X + 6.f, screen.Y + 6.f }, ImGui::GetColorU32(color));
                                            }
                                        }
                                    }

                                    switch (cfg.visuals.ships.boxType)
                                    {
                                    case Config::EShipBox::E3DBoxes:
                                    {
                                        
                                        FVector origin, extent;
                                        actor->GetActorBounds(true, origin, extent);
                                        FRotator rotation = actor->K2_GetActorRotation();
                                        if (!Drawing::Render3DBox(localController, origin, extent, rotation, cfg.visuals.ships.boxColor)) continue;
                                        break;
                                    }
                                    /*
                                    case Config::EBox::EDebugBoxes:
                                    {
                                        FVector origin, extent;
                                        actor->GetActorBounds(true, origin, extent);
                                        UKismetMathLibrary::DrawDebugBox(reinterpret_cast<UObject*>(world), origin, extent, *reinterpret_cast<const FLinearColor*>(&cfg.visuals.ships.boxColor), actor->K2_GetActorRotation(), 0.f);
                                        break;
                                    }
                                    */
                                    }

                                    continue;
                                }
                                else if (actor->isFarShip())
                                {
                                    const FVector location = actor->K2_GetActorLocation();
                                    const int dist = localLoc.DistTo(location) * 0.01f;

                                    if (cfg.visuals.ships.bName && dist > 1500)
                                    {
                                        FVector2D screen;
                                        if (localController->ProjectWorldLocationToScreen(location, screen)) {
                                            char name[0x30];
                                            sprintf_s(name, "Ship [%d]", dist);
                                            Drawing::RenderText(const_cast<char*>(name), screen, cfg.visuals.ships.textCol);
                                        };
                                    }
                                    continue;
                                }
                            }
                            if (cfg.visuals.puzzles.bEnable && actor->isPuzzleVault())
                            {
                                auto vault = reinterpret_cast<APuzzleVault*>(actor);
                                if (cfg.visuals.puzzles.bDoor)
                                {
                                    const FVector location = reinterpret_cast<ACharacter*>(vault->OuterDoor)->K2_GetActorLocation();
                                    FVector2D screen;
                                    if (localController->ProjectWorldLocationToScreen(location, screen)) {
                                        char name[0x64];
                                        const int dist = localLoc.DistTo(location) * 0.01f;
                                        sprintf_s(name, "Vault door [%d]", dist);
                                        Drawing::RenderText(name, screen, cfg.visuals.puzzles.textCol);
                                    };
                                }
                                continue;
                            }
                        }
                    }
                }
            }
            
            if (cfg.visuals.bEnable)
            {

                if (cfg.visuals.islands.bEnable)
                {
                    if (cfg.visuals.islands.bName)
                    {
                        do {
                            auto const islandService = gameState->IslandService;
                            if (!islandService) break;
                            auto const islandDataAsset = islandService->IslandDataAsset;
                            if (!islandDataAsset) break;
                            auto const islandDataEntries = islandDataAsset->IslandDataEntries;
                            if (!islandDataEntries.Data) break;
                            for (auto i = 0u; i < islandDataEntries.Count; i++)
                            {
                                auto const island = islandDataEntries[i];
                                auto const WorldMapData = island->WorldMapData;
                                if (!WorldMapData) continue;

                                const FVector islandLoc = WorldMapData->WorldSpaceCameraPosition;
                                const int dist = localLoc.DistTo(islandLoc) * 0.01f;
                                if (dist > cfg.visuals.islands.intMaxDist) continue;
                                FVector2D screen;
                                if (localController->ProjectWorldLocationToScreen(islandLoc, screen))
                                {
                                    char name[0x64];
                                    auto len = island->LocalisedName->multi(name, 0x50);
                                    
                                    
                                    snprintf(name + len, sizeof(name) - len, " [%d]", dist);
                                    Drawing::RenderText(name, screen, cfg.visuals.islands.textCol);
                                    
                                    
                                }
                                
                            }

                        } while (false);
                    
                        
                    }
                }
                
                if (cfg.visuals.client.bCrosshair)
                {
                    drawList->AddLine({ io.DisplaySize.x * 0.5f - cfg.visuals.client.fCrosshair, io.DisplaySize.y * 0.5f }, { io.DisplaySize.x * 0.5f + cfg.visuals.client.fCrosshair, io.DisplaySize.y * 0.5f }, ImGui::GetColorU32(cfg.visuals.client.crosshairColor));
                    drawList->AddLine({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f - cfg.visuals.client.fCrosshair }, { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f + cfg.visuals.client.fCrosshair }, ImGui::GetColorU32(cfg.visuals.client.crosshairColor));
                }

                if (cfg.visuals.client.bOxygen && localCharacter->IsInWater())
                {
                    auto drownComp = localCharacter->DrowningComponent;
                    if (!drownComp) break;
                    auto level = drownComp->GetOxygenLevel();
                    auto posX = io.DisplaySize.x * 0.5f;
                    auto posY = io.DisplaySize.y * 0.85f;
                    auto barWidth2 = io.DisplaySize.x * 0.05f;
                    auto barHeight2 = io.DisplaySize.y * 0.0030f;
                    drawList->AddRectFilled({ posX - barWidth2, posY - barHeight2 }, { posX + barWidth2, posY + barHeight2 }, ImGui::GetColorU32(IM_COL32(0, 0, 0, 255)));
                    drawList->AddRectFilled({ posX - barWidth2, posY - barHeight2 }, { posX - barWidth2 + barWidth2 * level * 2.f, posY + barHeight2 }, ImGui::GetColorU32(IM_COL32(0, 200, 255, 255)));
                }

                if (cfg.visuals.client.bCompass)
                {
               
                    const char* directions[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
                    int yaw = ((int)cameraRot.Yaw + 450) % 360;
                    int index = int(yaw + 22.5f) % 360 * 0.0222222f;

                
                    FVector2D pos = { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.02f };
                    auto col = ImVec4(1.f, 1.f, 1.f, 1.f);
                    Drawing::RenderText(const_cast<char*>(directions[index]), pos, col);
                    char buf[0x30];
                    int len = sprintf_s(buf, "%d", yaw);
                    pos.Y += 15.f;
                    Drawing::RenderText(buf, pos, col);
                
                
                }
            }

            if (aimBest.target != nullptr)
            {
                FVector2D screen;
                if (localController->ProjectWorldLocationToScreen(aimBest.location, screen)) 
                {
                    auto col = ImGui::GetColorU32(IM_COL32(0, 200, 0, 255));
                    drawList->AddLine({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f }, { screen.X, screen.Y }, col);
                    drawList->AddCircle({ screen.X, screen.Y }, 3.f, col);
                }

                if (ImGui::IsMouseDown(1))
                {
                    
                    if (isHarpoon)
                    {
                        reinterpret_cast<AHarpoonLauncher*>(attachObject)->rotation = aimBest.delta;
                    }
                    else {
                        /*
                        * LV - Local velocity
                        * TV - Target velocity
                        * RV - Target relative velocity
                        * BS - Bullet speed
                        * RL - Relative local location
                        */
                        FVector LV = localCharacter->GetVelocity();
                        if (auto const localShip = localCharacter->GetCurrentShip()) LV += localShip->GetVelocity();
                        FVector TV = aimBest.target->GetVelocity();
                        if (auto const targetShip = aimBest.target->GetCurrentShip()) TV += targetShip->GetVelocity();
                        const FVector RV = TV - LV;
                        const float BS = localWeapon->WeaponParameters.AmmoParams.Velocity;
                        const FVector RL = localLoc - aimBest.location;
                        const float a = RV.Size() - BS * BS;
                        const float b = (RL * RV * 2.f).Sum();
                        const float c = RL.SizeSquared();
                        const float D = b*b - 4 * a * c;
                        if (D > 0)
                        {
                            const float DRoot = sqrtf(D);
                            const float x1 = (-b + DRoot) / (2 * a);
                            const float x2 = (-b - DRoot) / (2 * a);
                            if (x1 >= 0 && x1 >= x2) aimBest.location += RV * x1;
                            else if (x2 >= 0) aimBest.location += RV * x2;

                            aimBest.delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, aimBest.location), cameraRot);
                            auto smoothness = 1.f / aimBest.smoothness;
                            localController->AddYawInput(aimBest.delta.Yaw* smoothness);
                            localController->AddPitchInput(aimBest.delta.Pitch * -smoothness);
                        }

                    }
                }
               
            }

            if (!localController->IdleDisconnectEnabled && !(cfg.misc.bEnable && cfg.misc.client.bEnable && cfg.misc.client.bIdleKick))
            {
                localController->IdleDisconnectEnabled = true;
            }

            if (cfg.misc.bEnable)
            {
                if (cfg.misc.client.bEnable) 
                {
                    if (cfg.misc.client.bShipInfo)
                    {
                        auto ship = localCharacter->GetCurrentShip();
                        if (ship)
                        {
                            FVector velocity = ship->GetVelocity() / 100.f;
                            char buf[0xFF];
                            sprintf(buf, "X: %.0f Y: %.0f Z: %.0f", velocity.X, velocity.Y, velocity.Z);

                            FVector2D pos {10.f, 50.f};
                            ImVec4 col{ 1.f,1.f,1.f,1.f };
                            Drawing::RenderText(buf, pos, col, true, false);

                            auto speed = velocity.Size();
                            sprintf(buf, "Speed: %.0f", speed);
                            pos.Y += 15.f;
                            Drawing::RenderText(buf, pos, col, true, false);

                        }
                    }
                    if (localController->IdleDisconnectEnabled && cfg.misc.client.bIdleKick)
                    {
                        localController->IdleDisconnectEnabled = false;
                    }

                }
                if (cfg.misc.game.bEnable)
                {
                    if (cfg.misc.game.bShowPlayers) 
                    {
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar(2);
                        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.13f, io.DisplaySize.y * 0.25f), ImGuiCond_Once);
                        ImGui::Begin("PlayersList", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
                        
                        auto shipsService = gameState->ShipService;
                        if (shipsService)
                        {
                            ImGui::BeginChild("Info", { 0.f, 15.f });
                            ImGui::Text("Total ships (including AI): %d", shipsService->GetNumShips());
                            ImGui::EndChild();
                        }
                        
                        auto crewService = gameState->CrewService;
                        auto crews = crewService->Crews;
                        if (crews.Data)
                        {
                            ImGui::Columns(2, "CrewPlayers");
                            ImGui::Separator();
                            ImGui::Text("Name"); ImGui::NextColumn();
                            ImGui::Text("Activity"); ImGui::NextColumn();
                            ImGui::Separator();
                            for (uint32_t i = 0; i < crews.Count; i++)
                            {
                                auto& crew = crews[i];
                                auto players = crew.Players;
                                if (players.Data)
                                {
                                    for (uint32_t k = 0; k < players.Count; k++)
                                    {
                                        auto& player = players[k];
                                        char buf[0x64];
                                        player->PlayerName.multi(buf, 0x50);
                                        ImGui::Selectable(buf);
                                        if (ImGui::BeginPopupContextItem(buf))
                                        {
                                            ImGui::Button("gay");
                                            ImGui::EndPopup();
                                        }
                                        ImGui::NextColumn();
                                        const char* actions[] = { "None", "Bailing", "Cannon", "CannonEnd", "Capstan", "CapstanEnd", "CarryingBooty", "CarryingBootyEnd", "Dead", "DeadEnd", "Digging", "Dousing", "EmptyingBucket", "Harpoon", "Harpoon_END", "LoseHealth", "Repairing", "Sails", "Sails_END", "Wheel", "Wheel_END" };
                                        auto activity = (uint8_t)player->GetPlayerActivity();
                                        if (activity < 21) { ImGui::Text(actions[activity]); }
                                      
                                        ImGui::NextColumn();
                                    }
                                    ImGui::Separator();
                                }

                                
                                
                                
                            }
                            ImGui::Columns();
                        }
                        ImGui::End();
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                    }
                    
                }
            }

            
        } while (false);
    }
    catch (...) 
    {
        // todo: somehow get the address where the error occurred
        Logger::Log("Exception\n");
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    
    
#ifdef STEAM
    if (ImGui::IsKeyPressed(VK_INSERT)) bIsOpen = !bIsOpen;
#else
    // if you would like to have full input in UWP version: reverse game and find array of key states (see: https://github.com/MICROSOFT-XBOX-ATG/MICROSOFT_UWP_UNREAL/blob/release_uwp/Engine/Source/Runtime/ApplicationCore/Private/UWP/UWPInputInterface.cpp#L9). 
    static const FKey insert("Insert");
    if (cache.localController && cache.localController->WasInputKeyJustPressed(insert)) { bIsOpen = !bIsOpen; } // todo: change this shit
#endif

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
                const char* boxes[] = { "None", "2DBox", "3DBox" };
                
                ImGui::Text("Players");
                if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 310.f), true, 0))
                {
                    const char* bars[] = { "None", "2DRectLeft", "2DRectRight", "2DRectBottom", "2DRectTop" };
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
                if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 310.f), true, 0))
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

                ImGui::NextColumn();

                ImGui::Text("Ships");
                if (ImGui::BeginChild("ShipsSettings", ImVec2(0.f, 200.f), true, 0)) {

                    const char* shipBoxes[] = {"None", "3DBox"};
                    ImGui::Checkbox("Enable", &cfg.visuals.ships.bEnable);
                    ImGui::Checkbox("Draw name", &cfg.visuals.ships.bName);
                    ImGui::Checkbox("Show holes", &cfg.visuals.ships.bDamage);
                    ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.ships.boxType), shipBoxes, IM_ARRAYSIZE(shipBoxes));
                    ImGui::ColorEdit4("Box color", &cfg.visuals.ships.boxColor.x, 0);
                    ImGui::ColorEdit4("Damage color", &cfg.visuals.ships.damageColor.x, 0);
                    ImGui::ColorEdit4("Text color", &cfg.visuals.ships.textCol.x, 0);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Islands");
                if (ImGui::BeginChild("IslandsSettings", ImVec2(0.f, 200.f), true, 0)) {
                    ImGui::Checkbox("Enable", &cfg.visuals.islands.bEnable);
                    ImGui::Checkbox("Draw names", &cfg.visuals.islands.bName);
                    ImGui::SliderInt("Max distance", &cfg.visuals.islands.intMaxDist, 100, 10000, "%d", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::ColorEdit4("Text color", &cfg.visuals.islands.textCol.x, 0);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

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

                ImGui::NextColumn();

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

                ImGui::NextColumn();

                ImGui::Text("Shipwrecks");
                if (ImGui::BeginChild("ShipwrecksSettings", ImVec2(0.f, 220.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.visuals.shipwrecks.bEnable);
                    ImGui::Checkbox("Draw name", &cfg.visuals.shipwrecks.bName);
                    ImGui::ColorEdit4("Text color", &cfg.visuals.shipwrecks.textCol.x, 0);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

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
                if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 180.f), true, 0))
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
                if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.skeletons.bEnable);
                    ImGui::Checkbox("Visible only", &cfg.aim.skeletons.bVisibleOnly);
                    ImGui::SliderFloat("Yaw", &cfg.aim.skeletons.fYaw, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.skeletons.fPitch, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Smoothness", &cfg.aim.skeletons.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    

                }
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Harpoon");
                if (ImGui::BeginChild("HarpoonSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.harpoon.bEnable);
                    ImGui::Checkbox("Visible only", &cfg.aim.harpoon.bVisibleOnly);
                    ImGui::SliderFloat("Yaw", &cfg.aim.harpoon.fYaw, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.harpoon.fPitch, 1.f, 102.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                }
                ImGui::EndChild();

                ImGui::NextColumn();

                /*
                
               
                ImGui::Text("Cannon");
                if (ImGui::BeginChild("CannonSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.aim.cannon.bEnable);
                    ImGui::Checkbox("Visible only", &cfg.aim.cannon.bVisibleOnly);
                    ImGui::SliderFloat("Yaw", &cfg.aim.cannon.fYaw, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::SliderFloat("Pitch", &cfg.aim.cannon.fPitch, 1.f, 102.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
                }
                ImGui::EndChild();

                 */

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
                if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.misc.client.bEnable);
                    ImGui::Checkbox("Ship speed", &cfg.misc.client.bShipInfo);
                    ImGui::Checkbox("Disable idle kick", &cfg.misc.client.bIdleKick);
                    ImGui::Separator();
                    if (ImGui::Button("Save settings"))
                    {
                        do {
                            wchar_t buf[MAX_PATH];
                            GetModuleFileNameW(hinstDLL, buf, MAX_PATH);
                            fs::path path = fs::path(buf).remove_filename() / ".settings";
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
                            GetModuleFileNameW(hinstDLL, buf, MAX_PATH);
                            fs::path path = fs::path(buf).remove_filename() / ".settings";
                            auto file = CreateFileW(path.wstring().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (file == INVALID_HANDLE_VALUE) break;
                            DWORD readed;
                            if (ReadFile(file, &cfg, sizeof(cfg), &readed, 0))  ImGui::OpenPopup("##SettingsLoaded");
                            CloseHandle(file);
                        } while (false);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Tests")) {
                        auto h = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Tests), nullptr, 0, nullptr);
                        if (h) CloseHandle(h);
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

                ImGui::NextColumn();

                ImGui::Text("Game");
                if (ImGui::BeginChild("GameSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    ImGui::Checkbox("Enable", &cfg.misc.game.bEnable);
                    ImGui::Checkbox("Show players list", &cfg.misc.game.bShowPlayers);
                }
                ImGui::EndChild();

                ImGui::NextColumn();


                ImGui::Text("Kraken");
                if (ImGui::BeginChild("KrakenSettings", ImVec2(0.f, 180.f), true, 0))
                {
                    

                    AKrakenService* krakenService;
                    bool isActive = false;
                    if (cache.good)  
                    { 
                        krakenService = cache.gameState->KrakenService;
                        if (krakenService) { krakenService->IsKrakenActive(); }
                    }
                    ImGui::Text("IsKrakenActive: %d", isActive);

                    /*
                    if (ImGui::Button("Attempt to request kraken"))
                    {
                        if (krakenService) { krakenService->RequestKrakenWithLocation(localCharacter->K2_GetActorLocation(), localCharacter); }
                    }
                    if (ImGui::Button("Dismiss kraken"))
                    {
                        if (krakenService) { krakenService->DismissKraken(); }
                    }
                    */
                }
                ImGui::EndChild();

                ImGui::Columns();


                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        };
        ImGui::End();
    }
  
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return PresentOriginal(swapChain, syncInterval, flags);
}

HRESULT Cheat::Renderer::ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{

    if (renderTargetView)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        renderTargetView->Release();
        renderTargetView = nullptr;
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
    
    return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}


inline bool Cheat::Renderer::Init()
{
    HMODULE dxgi = GetModuleHandleA("dxgi.dll");
    Logger::Log("dxgi: %p\n", dxgi);
    static BYTE PresentSig[] = { 0x55, 0x57, 0x41, 0x56, 0x48, 0x8d, 0x6c, 0x24, 0x90, 0x48, 0x81, 0xec, 0x70, 0x01 };
    //static BYTE PresentHead[] = { 0x48, 0x89, 0x5c, 0x24, 0x10 };
    //BYTE* fnPresent = Tools::PacthFn(dxgi, PresentSig, sizeof(PresentSig), PresentHead, sizeof(PresentHead));
    fnPresent = reinterpret_cast<decltype(fnPresent)>(Tools::FindFn(dxgi, PresentSig, sizeof(PresentSig)));
    Logger::Log("IDXGISwapChain::Present: %p\n", fnPresent);
    if (!fnPresent) return false;
    

    static BYTE ResizeSig[] = { 0x48, 0x81, 0xec, 0xc0, 0x00, 0x00, 0x00, 0x48, 0xc7, 0x45, 0x1f };
    //static BYTE ResizeHead[] = { 0x48, 0x8b, 0xc4, 0x55, 0x41, 0x54 };  
    //BYTE* fnResize = Tools::PacthFn(dxgi, ResizeSig, sizeof(ResizeSig), ResizeHead, sizeof(ResizeHead));
    fnResize = reinterpret_cast<decltype(fnResize)>(Tools::FindFn(dxgi, ResizeSig, sizeof(ResizeSig)));
    Logger::Log("IDXGISwapChain::ResizeBuffers: %p\n", fnResize);
    if (!fnResize) return false;
    

    if (!SetHook(fnPresent, PresentHook, reinterpret_cast<void**>(&PresentOriginal)))
    {
        return false;
    };

    Logger::Log("PresentHook: %p\n", PresentHook);
    Logger::Log("PresentOriginal: %p\n", PresentOriginal);

    if (!SetHook(fnResize, ResizeHook, reinterpret_cast<void**>(&ResizeOriginal)))
    {
        return false;
    };

    Logger::Log("ResizeHook: %p\n", ResizeHook);
    Logger::Log("ResizeOriginal: %p\n", ResizeOriginal);

    if (!SetHook(SetCursorPos, SetCursorPosHook, reinterpret_cast<void**>(&SetCursorPosOriginal)))
    {
        Logger::Log("Can't hook SetCursorPos\n");
        return false;
    };

    if (!SetHook(SetCursor, SetCursorHook, reinterpret_cast<void**>(&SetCursorOriginal)))
    {
        Logger::Log("Can't hook SetCursor\n");
        return false;
    };

    return true;
}

inline bool Cheat::Renderer::Remove()
{
    Renderer::RemoveInput(); 
    if (!RemoveHook(PresentOriginal) || !RemoveHook(ResizeOriginal) || !RemoveHook(SetCursorPosOriginal) || !RemoveHook(SetCursorOriginal))
    {
        return false;
    }
    if (renderTargetView)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext();
        renderTargetView->Release();
        renderTargetView = nullptr;
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
    return true;
}

inline bool Cheat::Tools::CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size)
{
    for (SIZE_T i = 0; i < size; i++) {
        if (data[i] != sig[i]) {
            if (sig[i] == 0x00) continue;
            return false;
        }
    }
    return true;
}

inline BYTE* Cheat::Tools::FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size)
{
    for (BYTE* it = start; it < end - size; it++) {
        if (CompareByteArray(it, sig, size)) {
            return it;
        };
    }
    return 0;
}

void* Cheat::Tools::FindPointer(BYTE* sig, SIZE_T size, int addition = 0)
{
    auto base = static_cast<BYTE*>(gBaseMod.lpBaseOfDll);
    auto address = FindSignature(base, base + gBaseMod.SizeOfImage - 1, sig, size);
    if (!address) return nullptr;
    auto k = 0;
    for (; sig[k]; k++);
    auto offset = *reinterpret_cast<UINT32*>(address + k);
    return address + k + 4 + offset + addition;
}

inline BYTE* Cheat::Tools::FindFn(HMODULE mod, BYTE* sig, SIZE_T sigSize)
{
    if (!mod || !sig || !sigSize) return 0;
    MODULEINFO modInfo;
    if (!K32GetModuleInformation(GetCurrentProcess(), mod, &modInfo, sizeof(MODULEINFO))) return 0;
    auto base = static_cast<BYTE*>(modInfo.lpBaseOfDll);
    auto fn = Tools::FindSignature(base, base + modInfo.SizeOfImage - 1, sig, sigSize);
    if (!fn) return 0;
    for (; *fn != 0xCC && *fn != 0xC3; fn--);
    fn++;
    return fn;
}

inline bool Cheat::Tools::PatchMem(void* address, void* bytes, SIZE_T size)
{
    DWORD oldProtection;
    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtection))
    {
        memcpy(address, bytes, size);
        return VirtualProtect(address, size, oldProtection, &oldProtection);
    };
    return false;
}

/*inline bool Cheat::Tools::HookVT(void** vtable, UINT64 index, void* FuncH, void** FuncO)
{
    if (!vtable || !FuncH || !vtable[index]) return false;
    if (FuncO) { *FuncO = vtable[index]; }
    PatchMem(&vtable[index], &FuncH, 8);
    return FuncH == vtable[index];
}*/

inline BYTE* Cheat::Tools::PacthFn(HMODULE mod, BYTE* sig, SIZE_T sigSize, BYTE* bytes, SIZE_T bytesSize)
{
    if (!mod || !sig || !sigSize || !bytes || !bytesSize) return 0;
    auto fn = FindFn(mod, sig, sigSize);
    if (!fn) return 0;
    return Tools::PatchMem(fn, bytes, bytesSize) ? fn : 0;
}

inline bool Cheat::Tools::FindNameArray()
{
    static BYTE sig[] = { 0x48, 0x8b, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xff, 0x75, 0x3c };
    auto address = reinterpret_cast<decltype(FName::GNames)*>(FindPointer(sig, sizeof(sig)));
    if (!address) return 0;
    Logger::Log("%p\n", address);
    FName::GNames = *address;
    return FName::GNames;
}

inline bool Cheat::Tools::FindObjectsArray()
{
    static BYTE sig[] = { 0x89, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0xDF, 0x48, 0x89, 0x5C, 0x24 };
    UObject::GObjects = reinterpret_cast<decltype(UObject::GObjects)>(FindPointer(sig, sizeof(sig), 16));
    return UObject::GObjects;
}

inline bool Cheat::Tools::FindWorld()
{
    static BYTE sig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC9, 0x74, 0x06, 0x48, 0x8B, 0x49, 0x70 };
    UWorld::GWorld = reinterpret_cast<decltype(UWorld::GWorld)>(FindPointer(sig, sizeof(sig)));
    return UWorld::GWorld;
}

inline bool Cheat::Tools::InitSDK()
{
    if (!UCrewFunctions::Init()) return false;
    if (!UKismetMathLibrary::Init()) return false;
    return true;
}

inline bool Cheat::Logger::Init()
{
    fs::path log;
#ifdef STEAM
    wchar_t buf[MAX_PATH];
    if (!GetModuleFileNameW(hinstDLL, buf, MAX_PATH)) return false;
    log = fs::path(buf).remove_filename() / "log.txt";
#else
#ifdef UWPDEBUG
    log = "C:\\Users\\dimae\\AppData\\Local\\Packages\\Microsoft.SeaofThieves_8wekyb3d8bbwe\\TempState\\log.txt";
#else
    return true;
#endif
#endif
    file = CreateFileW(log.wstring().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    return file != INVALID_HANDLE_VALUE;
}

inline bool Cheat::Logger::Remove()
{
    if (!file) return true;
    return CloseHandle(file);
}

void Cheat::Logger::Log(const char* format, ...)
{
#if defined STEAM || defined UWPDEBUG
    SYSTEMTIME rawtime;
    GetSystemTime(&rawtime);
    char buf[MAX_PATH];
    auto size = GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, 0, &rawtime, "[HH':'mm':'ss] ", buf, MAX_PATH) - 1;
    size += snprintf(buf + size, sizeof(buf) - size, "[TID: 0x%X] ", GetCurrentThreadId());
    va_list argptr;
    va_start(argptr, format);
    size += vsnprintf(buf + size, sizeof(buf) - size, format, argptr);
    WriteFile(file, buf, size, NULL, NULL);
    va_end(argptr);
#endif
}

bool Cheat::Init(HINSTANCE _hinstDLL)
{
    hinstDLL = _hinstDLL;
    if (!Logger::Init())
    {
        return false;
    };
    if (!K32GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &gBaseMod, sizeof(MODULEINFO))) 
    {
        return false;
    };
    Logger::Log("Base address: %p\n", gBaseMod.lpBaseOfDll);
    if (!Tools::FindNameArray()) 
    {
        Logger::Log("Can't find NameArray!\n");
        return false;
    }
    Logger::Log("NameArray: %p\n", FName::GNames);
    if (!Tools::FindObjectsArray()) 
    {
        Logger::Log("Can't find ObjectsArray!\n");
        return false;
    } 
    Logger::Log("ObjectsArray: %p\n", UObject::GObjects);
    if (!Tools::FindWorld())
    {
        Logger::Log("Can't find World!\n");
        return false;
    }
    Logger::Log("World: %p\n", UWorld::GWorld);
    if (!Tools::InitSDK())
    {
        Logger::Log("Can't find important objects!\n");
        return false;
    };
    
    if (!Renderer::Init())
    {
        Logger::Log("Can't initialize renderer\n");
        return false;
    }
    Hacks::Init();

#ifdef STEAM
    auto t = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ClearingThread), nullptr, 0, nullptr);
    if (t) CloseHandle(t);
#endif

    return true;
}

void Cheat::ClearingThread()
{
    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            FreeLibraryAndExitThread(hinstDLL, 0);
        }
        Sleep(20);
    }
}

void Cheat::Tests()
{
    /*
    auto world = *UWorld::GWorld;
    auto localController = world->GameInstance->LocalPlayers[0]->PlayerController;
    Logger::Log("%p\n", localController);
    */
}

bool Cheat::Remove()
{

    Logger::Log("Removing cheat...\n");
    
    if (!Renderer::Remove() || !Logger::Remove())
    {
        return false;
    };

    Hacks::Remove();

    //Hacks::Remove();

    // some other stuff...

    

    return true;
}
