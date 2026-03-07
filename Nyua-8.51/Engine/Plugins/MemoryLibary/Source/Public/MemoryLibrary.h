#pragma once

#include "CoreGlobals.h"

enum MNEMONIC : uint8_t
{
    JMP_REL8 = 0xEB,
    JMP_REL32 = 0xE9,
    JMP_EAX = 0xE0,
    CALL = 0xE8,
    LEA = 0x8D,
    CDQ = 0x99,
    CMOVL = 0x4C,
    CMOVS = 0x48,
    CMOVNS = 0x49,
    NOP = 0x90,
    RET = 0xC3,
    INT3 = 0xCC,
    RETN_REL8 = 0xC2,
    RETN = 0xC3,
    POP = 0x58,
    MAXOP = 0x5F,
    CMOVNO = 0x41,
    NONE = 0x00,
    PUSH = 0x40,
    JE = 0x74
};

class UMemoryLibrary
{
public:
    template<class T = uint8_t>
    static void Patch(uint64 Target, T Byte)
    {
        Target += InSDKUtils::GetImageBase();

        DWORD OldProtect;
        VirtualProtect(reinterpret_cast<void*>(Target), sizeof(T),
                             PAGE_EXECUTE_READWRITE, &OldProtect);
        
        *(T*)(Target) = Byte;

        VirtualProtect(reinterpret_cast<void*>(Target), sizeof(T),
                      OldProtect, &OldProtect);
    }

    static void PatchBytes(uint64_t Target, const void* Bytes, size_t Length)
    {
        Target += InSDKUtils::GetImageBase();

        DWORD OldProtect;
        VirtualProtect(reinterpret_cast<void*>(Target), Length,
                       PAGE_EXECUTE_READWRITE, &OldProtect);

        std::memcpy(reinterpret_cast<void*>(Target), Bytes, Length);

        VirtualProtect(reinterpret_cast<void*>(Target), Length,
                       OldProtect, &OldProtect);
    }
    
    static void Nop(uint64_t Target, size_t Length)
    {
        std::vector<uint8_t> NopBuffer(Length, 0x90);
        PatchBytes(Target, NopBuffer.data(), Length);
    }

    static void Null(uint64 Target)
    {
        Patch<uint8_t>(Target, MNEMONIC::RETN);
    }

    static bool IsReadablePointer(void* ptr, size_t size)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(ptr, &mbi, sizeof(mbi))) 
            return false;
        return (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ)) != 0;
    }
};