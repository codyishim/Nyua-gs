#pragma once

#include "CoreGlobals.h"
#include "nlohmann.hpp"

namespace Builder
{
    static TArray<AActor*> GetActorsInVol(AFortVolume* Volume);
    static std::string GetIslandName(AFortPlayerControllerAthena* Controller);
    static std::string GetIslandPath(AFortPlayerControllerAthena* Controller);

    void LoadAllTraps(AFortPlayerControllerAthena* Controller, nlohmann::json& j, AFortVolume* Volume);
    bool LoadIsland(AFortPlayerControllerAthena* Controller);
    void LoadActorSettings(AActor* Actor, nlohmann::json optionsJson);

    static void SaveActor(nlohmann::json& j, nlohmann::json& ja, AActor* actor, FVector VolumeLocation, FRotator VolumeRotation);
    bool SaveIsland(AFortPlayerControllerAthena* Controller);

    bool ResetIsland(AFortPlayerControllerAthena* Controller);

    UFortDecoItemDefinition* GetTIDFromTrap(std::string trap);
	
}
