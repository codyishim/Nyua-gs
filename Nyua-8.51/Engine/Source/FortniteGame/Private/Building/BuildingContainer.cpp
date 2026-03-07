#include "Building/BuildingContainer.h"

#include "FortKismetLibrary.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"

bool BuildingContainer::SpawnLoot(ABuildingContainer* Container, AFortPlayerPawn* Pawn)
{
    if (!Container || Container->bAlreadySearched)
        return false;
    
    FName RedirectedLootTierGroup = FortKismetLibrary::RedirectLootTierGroup(Container->SearchLootTierGroup);
    UE_LOG(LogServer, Log, ("SpawnLoot: RedirectedLootTierGroup %s"), RedirectedLootTierGroup.ToString().c_str());

    TArray<FFortItemEntry> LootDrops = FortKismetLibrary::PickLootDrops(RedirectedLootTierGroup);

    if (!LootDrops.Num())
        return true;

    FVector CorrectLocation = Pawn ? Container->K2_GetActorLocation() + Container->GetActorRightVector() * 70.0f + FVector{ 0, 0, 50 } : Container->K2_GetActorLocation();
    
    for (auto LootDrop : LootDrops)
    {
        FortKismetLibrary::SpawnPickup(LootDrop, CorrectLocation, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset, Pawn);
    }

    Container->SearchBounceData.SearchAnimationCount++;
    Container->BounceContainer();
    Container->bAlreadySearched = true;
    Container->OnRep_bAlreadySearched();

    return true;
}

void BuildingContainer::Setup()
{
   // UDetoursLibrary::InitializeDetour<ABuildingContainer, MinHook>(0x10F9FC0, SpawnLoot);
}

