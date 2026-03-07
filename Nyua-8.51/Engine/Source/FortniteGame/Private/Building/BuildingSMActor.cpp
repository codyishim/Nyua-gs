#include "Building/BuildingSMActor.h"

#include "Building/BuildingContainer.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"

void BuildingSMActor::PostUpdate(ABuildingSMActor* BuildingSMActor)
{
    if (ABuildingContainer* BuildingContainer = Cast<ABuildingContainer>(BuildingSMActor))
    {
        ::BuildingContainer::SpawnLoot(BuildingContainer);
    }

    return Originals::PostUpdate(BuildingSMActor);
}

bool BuildingSMActor::AttemptSpawnResources(ABuildingSMActor* BuildingSMActor, AFortPlayerPawn* Pawn, float DamageDealt, bool WeakSpot)
{
    static UCurveTable* AthenaResourceRates = StaticLoadObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

    EEvaluateCurveTableResult STFU;
    float Result;
    UDataTableFunctionLibrary::EvaluateCurveTableRow(AthenaResourceRates, BuildingSMActor->BuildingResourceAmountOverride.RowName, 0, &STFU, &Result, FString());

    int ResourceCount = (int)round(Result / (BuildingSMActor->GetMaxHealth() / DamageDealt));
    
    if (ResourceCount == 0)
        return false;
    
    UFortResourceItemDefinition* ItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType);

    if (!ItemDefinition)
        return false;

    auto Controller = (AFortPlayerController*)Pawn->GetController();
    FortInventory::GiveItem(Controller, ItemDefinition, ResourceCount);

    Controller->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->ResourceType, ResourceCount, BuildingSMActor->GetHealth() - DamageDealt <= 0, WeakSpot);
    return true;
}

void BuildingSMActor::Setup()
{
    //UDetoursLibrary::InitializeDetour<ABuildingSMActor, MinHook>(0x1123260, PostUpdate, &Originals::PostUpdate);
    UDetoursLibrary::InitializeDetour<ABuildingSMActor, EveryVFT>(0x153, AttemptSpawnResources);
}
