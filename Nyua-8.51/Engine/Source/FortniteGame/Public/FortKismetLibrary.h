#pragma once

#include "CoreGlobals.h"

class FortKismetLibrary
{
public:
    static TArray<FFortItemEntry> PickLootDrops(FName TierGroupName, int ForcedLootTier = -1, int ReturnCount = 0);
    static FName RedirectLootTierGroup(FName TierGroupName);
    static int32 GetClipSize(UFortWeaponItemDefinition* WeaponDefinition);
    
    static AFortPickupAthena* SpawnPickup(FFortItemEntry Entry, FVector Loc, EFortPickupSourceTypeFlag SourceFlag = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset, AFortPlayerPawn* Pawn = nullptr, bool bToss = true);
    static AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDef, FVector Loc, int Count, EFortPickupSourceTypeFlag SourceFlag = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset, AFortPlayerPawn* Pawn = nullptr, bool bToss = true);
};