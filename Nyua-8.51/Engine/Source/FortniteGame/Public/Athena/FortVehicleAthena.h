#pragma once
#include "CoreGlobals.h"

class FortVehicleAthena
{
public:
    class Originals
    {
    public:
    };
public:
    static void ServerUpdatePhysicsParams(AFortPhysicsPawn* Pawn, FReplicatedPhysicsPawnState& InState);

    static void Setup();
};