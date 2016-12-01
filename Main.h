void HideModule( HINSTANCE hModule )
{
    DWORD dwPEB_LDR_DATA = 0;

    _asm
    {
        pushad;
        pushfd;
        mov eax, fs:[30h]                    // PEB
        mov eax, [eax+0Ch]                    // PEB->ProcessModuleInfo
        mov dwPEB_LDR_DATA, eax                // Save ProcessModuleInfo

InLoadOrderModuleList:
        mov esi, [eax+0Ch]                    // ProcessModuleInfo->InLoadOrderModuleList[FORWARD]
        mov edx, [eax+10h]                    //  ProcessModuleInfo->InLoadOrderModuleList[BACKWARD]

LoopInLoadOrderModuleList: 
        lodsd                                //  Load First Module
            mov esi, eax                      //  ESI points to Next Module
            mov ecx, [eax+18h]                  //  LDR_MODULE->BaseAddress
        cmp ecx, hModule                  //  Is it Our Module ?
            jne SkipA                        //  If Not, Next Please (@f jumps to nearest Unamed Lable @@:)
            mov ebx, [eax]                        //  [FORWARD] Module 
        mov ecx, [eax+4]                 //  [BACKWARD] Module
        mov [ecx], ebx                        //  Previous Module's [FORWARD] Notation, Points to us, Replace it with, Module++
            mov [ebx+4], ecx                    //  Next Modules, [BACKWARD] Notation, Points to us, Replace it with, Module--
            jmp InMemoryOrderModuleList            //  Hidden, so Move onto Next Set
SkipA:
        cmp edx, esi                        //  Reached End of Modules ?
            jne LoopInLoadOrderModuleList        //  If Not, Re Loop

InMemoryOrderModuleList:
        mov eax, dwPEB_LDR_DATA                //  PEB->ProcessModuleInfo
            mov esi, [eax+14h]                    //  ProcessModuleInfo->InMemoryOrderModuleList[START]
        mov edx, [eax+18h]                    //  ProcessModuleInfo->InMemoryOrderModuleList[FINISH]

LoopInMemoryOrderModuleList: 
        lodsd
            mov esi, eax
            mov ecx, [eax+10h]
        cmp ecx, hModule
            jne SkipB
            mov ebx, [eax] 
        mov ecx, [eax+4]
        mov [ecx], ebx
            mov [ebx+4], ecx
            jmp InInitializationOrderModuleList
SkipB:
        cmp edx, esi
            jne LoopInMemoryOrderModuleList

InInitializationOrderModuleList:
        mov eax, dwPEB_LDR_DATA                    //  PEB->ProcessModuleInfo
            mov esi, [eax+1Ch]                        //  ProcessModuleInfo->InInitializationOrderModuleList[START]
        mov edx, [eax+20h]                        //  ProcessModuleInfo->InInitializationOrderModuleList[FINISH]

LoopInInitializationOrderModuleList: 
        lodsd
            mov esi, eax       
            mov ecx, [eax+08h]
        cmp ecx, hModule       
            jne SkipC
            mov ebx, [eax] 
        mov ecx, [eax+4]
        mov [ecx], ebx
            mov [ebx+4], ecx
            jmp Finished
SkipC:
        cmp edx, esi
            jne LoopInInitializationOrderModuleList

Finished:
        popfd;
        popad;
    }
}

bool Match(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
for(;*szMask;++szMask,++pData,++bMask)
if(*szMask=='x' && *pData!=*bMask )
return false;
return (*szMask) == NULL;
}

DWORD FindPattern(DWORD dwAddress,DWORD dwLen,BYTE *bMask,char * szMask)
{
for(DWORD i=0; i<dwLen; i++)
if(Match((BYTE*)(dwAddress + i), bMask, szMask))
return (DWORD)(dwAddress+i);
return 0;
}

void MakeJMP(BYTE *pAddress, DWORD dwJumpTo, DWORD dwLen)
{
DWORD dwOldProtect, dwBkup, dwRelAddr;
VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);
dwRelAddr = (DWORD) (dwJumpTo - (DWORD) pAddress) - 5;
*pAddress = 0xE9;
*((DWORD *)(pAddress + 0x1)) = dwRelAddr;
for(DWORD x = 0x5; x < dwLen; x++) *(pAddress + x) = 0x90;
VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);
return;
}
