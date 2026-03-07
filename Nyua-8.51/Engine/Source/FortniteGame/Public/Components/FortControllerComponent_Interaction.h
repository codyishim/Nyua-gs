#pragma once

#include "CoreGlobals.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

class FortControllerComponent_Interaction
{
private:
    class Originals
    {
    public:
        static inline void (*ServerAttemptInteract)(UFortControllerComponent_Interaction* InteractionComp, FFrame& Stack, void* Ret);
    };
    
public:
    static void ServerAttemptInteract(UFortControllerComponent_Interaction* InteractionComp, FFrame& Stack, void* Ret);
    
    static void Setup();
};
