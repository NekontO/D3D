#include <Windows.h>
#include <iostream>
#include <process.h>
#include <d3d9.h>
#include "XOR.h"
#include "Main.h"

DWORD retMyDIP;


__declspec(naked) HRESULT WINAPI MyDIP()
{
    static LPDIRECT3DDEVICE9 pDevice;

    __asm
    {
        MOV EDI,EDI
        PUSH EBP
        MOV EBP,ESP
        MOV EAX,DWORD PTR SS:[EBP + 0x8]
        MOV pDevice,EAX
    }
        pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    __asm
    {
        JMP retMyDIP
    }
}

unsigned __stdcall RAMLeague(LPVOID param)
{
    DWORD hD3D = (DWORD)LoadLibrary(ed3d9);
    DWORD *vtbl;

    DWORD adr = FindPattern(hD3D, 0x128000, (PBYTE)"\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86", "xx????xx????xx");
    if(adr)
    {
        memcpy(&vtbl,(void*)(adr + 2),4);
        retMyDIP = vtbl[147] + 0x5;
        MakeJMP((PBYTE)vtbl[147],(DWORD)MyDIP,0x5);
    }
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
    DisableThreadLibraryCalls(hModule);
    HideModule(hModule);
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _beginthreadex(0, 0, RAMLeague, 0, 0, 0);
    //system( "start http://goo.gl/YTc9Up" );
    }
    return TRUE;
}
