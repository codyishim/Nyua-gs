#pragma once

#include "CoreGlobals.h"
#include <type_traits>

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

class FortAthenaCreativePortal {
private:
    class Originals
    {
    public:
       static inline void (*TeleportPlayerToLinkedVolume)(AFortAthenaCreativePortal*, FFrame&, void*);
    };
    
public:
    static void TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Portal, FFrame& Stack, void* Ret);
    static void Setup();
};
