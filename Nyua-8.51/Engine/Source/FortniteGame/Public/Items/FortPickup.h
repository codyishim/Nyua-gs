#pragma once

#include "CoreGlobals.h"

class FortPickup
{
private:
    class Originals
    {
    public:
        static inline void (*OnAboutToEnterBackpack)(AFortPickupAthena*);
    };
    
public:
    static void OnAboutToEnterBackpack(AFortPickupAthena* Pickup);
    
    static void Setup();
};