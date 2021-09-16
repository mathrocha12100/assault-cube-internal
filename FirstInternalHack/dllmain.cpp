// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "mem.h"
#include "hook.h"
#include "proc.h"
#include "glText.h"
#include "glDraw.h"
#include "esp.h"
#include "console.h"

typedef ent* (__cdecl* tGetCrossHairEnt)();

tGetCrossHairEnt GetCrossHairEnt = nullptr;

typedef BOOL(__stdcall* twglSwapBuffers) (HDC hDc);

twglSwapBuffers wglSwapBuffersGateway;

GL::Font glFont;

const int FONT_HEIGHT = 15;
const int FONT_WIDTH = 9;

bool canEject = false;
bool bHealth = false, bAmmo = false, bRecoil = false, espActive = false, espLifebar = false, bTriggerbot = false;

uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");


ESP esp;

void Draw()
{
    HDC currentHDC = wglGetCurrentDC();

    if (!glFont.bBuilt || currentHDC != glFont.hdc)
    {
        glFont.Build(FONT_HEIGHT);
    }

    GL::SetupOrtho();
    esp.Draw(glFont, espLifebar);
    GL::RestoreGL();
}


void HackFeatures() {

    if (GetAsyncKeyState(VK_NUMPAD6) & 1) {
        bTriggerbot = !bTriggerbot;
    }
    
    if (GetAsyncKeyState(VK_NUMPAD0) & 1) {
        canEject = true;
    }

    if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
        bHealth = !bHealth;
        console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);

    }

    if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
        bAmmo = !bAmmo;
        console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);

    }

    if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
        bRecoil = !bRecoil;

        if (bRecoil) {
            //nop
            mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
        }
        else {
            // write back original instructions
            mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2", 10);
        }

        console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);

    }

    if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
        espActive = !espActive;
        if (!espLifebar) {
            espLifebar = false;
        }
        else {
            espLifebar = true;
        }
        console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);
    }

    if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
        espLifebar = !espLifebar;
        console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);
    }

    uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);
    
    ent* playerPtr = *(ent**)(moduleBase + 0x10F4F4);

    if (playerPtr) {
        if (bTriggerbot) {
            ent* crosshairEnt = GetCrossHairEnt();

            if (crosshairEnt) {
                if (playerPtr->team != crosshairEnt->team) {
                    playerPtr->bAttacking = 1; 
                }
            }
            else {
                playerPtr->bAttacking = 0;
            }

        }
    }


    if (localPlayerPtr) {
        if (bHealth) {
            *(int*)(*localPlayerPtr + 0xf8) = 1337;
        }
    }
    if (bAmmo) {
        uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x10f4f4, { 0x374, 0x14, 0x0 });

        // 3 ways to change the value

        // first
        /*
            int* ammo = (int*)ammoAddr;
            *ammo = 1337;
        */
        // second
        /*
        *   i have createad a func to do this for me
        *
            mem::ChangeValueOfMemoryValue(MemoryTypes::Integer, ammoAddr);
        */

        // three
        // or just

        // *(int*)mem::FindDMAAddy(moduleBase + 0x10f4f4, { 0x374, 0x14, 0x0 }) = 1337;
    }
}

BOOL __stdcall hkwglSwapBuffers(HDC hDc) {
    // std::cout << "Hooked!" << std::endl;

    HackFeatures();
    
    if (espActive) {
        Draw();
    }

    return wglSwapBuffersGateway(hDc);
}

DWORD WINAPI HackThread(HMODULE hModule) {
    // Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    GetCrossHairEnt = (tGetCrossHairEnt)(moduleBase + 0x607c0);

    console::DrawInConsole(bHealth, bAmmo, bRecoil, espActive, espLifebar, bTriggerbot);

    //Get Module base
    Hook SwapBuffersHook("wglSwapBuffers", "opengl32.dll", (BYTE*)hkwglSwapBuffers, (BYTE*)&wglSwapBuffersGateway, 5);

    SwapBuffersHook.Enable();

    while (!canEject);

    SwapBuffersHook.Disable();

    //cleaanup & eject

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

