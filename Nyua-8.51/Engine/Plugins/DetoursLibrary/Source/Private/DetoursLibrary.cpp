#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"


uint8_t* UDetoursLibrary::AllocateNearbyPage(void* targetAddr)
{
    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

    const uint64_t PageSize = SysInfo.dwPageSize;
    const uint64_t StartAddr = (uint64_t(targetAddr) & ~(PageSize - 1));
    const uint64_t MinAddr = min(StartAddr - 0x7FFFFF00, (uint64_t)SysInfo.lpMinimumApplicationAddress);
    const uint64_t MaxAddr = max(StartAddr + 0x7FFFFF00, (uint64_t)SysInfo.lpMaximumApplicationAddress);
    const uint64_t StartPage = (StartAddr - (StartAddr % PageSize));

    for (uint64_t PageOffset = 1; PageOffset; PageOffset++)
    {
        uint64_t ByteOffset = PageOffset * PageSize;
        uint64_t HighAddr = StartPage + ByteOffset;
        uint64_t LowAddr = (StartPage > ByteOffset) ? StartPage - ByteOffset : 0;

        bool NeedsExit = HighAddr > MaxAddr && LowAddr < MinAddr;

        if (HighAddr < MaxAddr)
        {
            if (void* OutAddr = VirtualAlloc((void*)HighAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
            {
                return (uint8_t*)OutAddr;
            }
        }

        if (LowAddr > MinAddr)
        {
            if (void* OutAddr = VirtualAlloc((void*)LowAddr, PageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
            {
                return (uint8_t*)OutAddr;
            }
        }

        if (NeedsExit)
        {
            break;
        }
    }

    return nullptr;
}

void UDetoursLibrary::InitializeDetourInternal(UObject* DefaultObject, void* Target, void* Detour, void** Original, EDetourType Type)
{
    switch (Type)
    {
    case EDetourType::EveryVFT: // sorry twin have to do this
        {
            for (int i = 0; i < UObject::GObjects->Num(); i++)
            {
                auto Obj = UObject::GObjects->GetByIndex(i);
                if (Obj)
                {
                    if (Obj->IsA(DefaultObject->Class))
                    {
                        int Index = (int)Target;

                        UE_LOG(LogServer, Log, "Index %d", Index);
        
                        if (Original)
                            *Original = Obj->VTable[Index];

                        DWORD oldProtect;
                        VirtualProtect(&Obj->VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        
                        Obj->VTable[Index] = Detour;

                        VirtualProtect(&Obj->VTable[Index], sizeof(void*), oldProtect, &oldProtect);
                    }
                }
            }
            break;
        }
    case EDetourType::VFT:
    {
        int Index = (int)Target;

        UE_LOG(LogServer, Log, "Index %d", Index);
        
        if (Original)
            *Original = DefaultObject->VTable[Index];

        DWORD oldProtect;
        VirtualProtect(&DefaultObject->VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
        
        DefaultObject->VTable[Index] = Detour;

        VirtualProtect(&DefaultObject->VTable[Index], sizeof(void*), oldProtect, &oldProtect);
        
        break;
    }

    case EDetourType::MinHook:
    {
        MH_CreateHook((void*)((uintptr_t)InSDKUtils::GetImageBase() + (uintptr_t)Target), Detour, (void**)Original);
        break;
    }

    case EDetourType::Exec:
    {
        UFunction* Func = reinterpret_cast<UFunction*>(Target);

        if (!Func)
        {
            UE_LOG(LogServer, Error, "Could not find Func for Exec Hook!");
            break;
        }

        UFunction::FNativeFuncPtr& ExecFunction = Func->ExecFunction;

        if (Original)
            *Original = ExecFunction;

        ExecFunction = (UFunction::FNativeFuncPtr)Detour;
        break;
    }
    }
}
void UDetoursLibrary::Rel32Swap(void** Target, void* Detour)
{
    auto Impl = (uint8*)(*Target);
    auto NearPage = AllocateNearbyPage(Impl);

    if (!NearPage)
    {
        return;
    }

    uint8_t Shellcode[] =
    {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp [$+6]
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    memcpy(Shellcode + 6, &Detour, 8);
    memcpy(NearPage, Shellcode, sizeof(Shellcode));

    auto Offset = static_cast<int>(NearPage - (Impl + 5));

    memcpy(Impl + 1, &Offset, sizeof(int));
}
