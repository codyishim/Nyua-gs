#pragma once

#include "CoreGlobals.h"

class BuildingContainer
{
public:
    static bool SpawnLoot(ABuildingContainer* Container, AFortPlayerPawn* Pawn = nullptr);

    static void Setup();
};