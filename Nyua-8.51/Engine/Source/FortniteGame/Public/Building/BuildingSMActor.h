#pragma once

#include "CoreGlobals.h"

class BuildingSMActor
{
private:
    class Originals
    {
    public:
        static inline void (*PostUpdate)(ABuildingSMActor* BuildingSMActor);
    };
    
public:
    static void PostUpdate(ABuildingSMActor* BuildingSMActor);
    static bool AttemptSpawnResources(ABuildingSMActor* BuildingSMActor, AFortPlayerPawn* Pawn, float DamageDealt, bool WeakSpot);
    
    static void Setup();
};