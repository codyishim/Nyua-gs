#pragma once

#include "CoreGlobals.h"

enum EDetourType
{
    VFT,
    Rel32,
    Exec,
    MinHook,
    EveryVFT
};

class UDetoursLibrary
{
public:
    template<typename Class = UObject, EDetourType DetourTypeValue, typename TargetType, typename OriginalPtrType = void*>
    static void InitializeDetour(TargetType Target, void* Detour, OriginalPtrType* Original = nullptr)
    {
        if constexpr (DetourTypeValue == EDetourType::VFT)
        {
            static_assert(std::is_same_v<TargetType, int>, "VFT detours require int vtable index");
        }
        else if constexpr (DetourTypeValue == EDetourType::MinHook)
        {
            static_assert(std::is_same_v<TargetType, int>, "MinHook detours require int");
        }
        else if constexpr (DetourTypeValue == EDetourType::Exec)
        {
            static_assert(std::is_same_v<TargetType, UFunction*>, "Exec detours require UFunction*");
        }
        else if constexpr (DetourTypeValue == EDetourType::Rel32)
        {
            static_assert(std::is_same_v<TargetType, uint64_t>, "Rel32 detours require uint64_t");
        } else if constexpr (DetourTypeValue == EDetourType::EveryVFT)
        {
            static_assert(std::is_same_v<TargetType, int>, "EveryVFT detours require int vtable index");
        }
        
        InitializeDetourInternal(Class::GetDefaultObj(), (void*)Target, Detour, reinterpret_cast<void**>(Original), DetourTypeValue);
    }
private:
    static inline uint8_t* AllocateNearbyPage(void* targetAddr);

    static void InitializeDetourInternal(UObject* DefaultObject, void* Target, void* Detour, void** Original, EDetourType Type);
public:
    static void Rel32Swap(void** Target, void* Detour);
public:
    static inline void Setup()
    {
        srand(time(0));

        MH_Initialize();
    }

    static inline void Finalize()
    {
        MH_EnableHook(MH_ALL_HOOKS);
    }
public:
    static inline bool ReturnTrueDetour() { return true; }
    static inline bool ReturnFalseDetour() { return false; }
};