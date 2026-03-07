#include "FortKismetLibrary.h"

#include "Engine/World.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"

template <typename T>
static T* GetWeightedRandom(TArray<T*>& Items)
{
    if (Items.Num() == 0) return nullptr;

    float TotalWeight = 0.0f;
    for (T* Item : Items)
        TotalWeight += Item->Weight;

    float RandomNumber = UKismetMathLibrary::RandomFloatInRange(0.0f, TotalWeight);

    for (T* Item : Items)
    {
        if (RandomNumber <= Item->Weight)
            return Item;

        RandomNumber -= Item->Weight;
    }
    
    return nullptr;
}

TArray<FFortItemEntry> FortKismetLibrary::PickLootDrops(FName TierGroupName, int ForcedLootTier, int ReturnCount)
{
    TArray<FFortItemEntry> LootDrops;
    
    if (ReturnCount >= 10)
        return LootDrops;

    static UDataTable* LootTierDataTable = nullptr;
    static UDataTable* LootPackageTable = nullptr;
    
    static bool bSetTables = false;

    if (!bSetTables)
    {
        bSetTables = true;

        auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
        auto Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

        LootTierDataTable = Playlist->LootTierData.IsValid() ? Playlist->LootTierData.Get()
            : StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
        
        LootPackageTable = Playlist->LootPackages.IsValid() ? Playlist->LootPackages.Get()
            : StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
    }

    if (!LootTierDataTable || !LootPackageTable)
    {
        UE_LOG(LogServer, Error, "PickLootDrops: Failed to find any valid DataTables");
        return LootDrops;
    }
    
    TArray<FFortLootTierData*> TierGroupLTDs;

    for (auto& LootTier : LootTierDataTable->GetAs<FFortLootTierData>())
    {
        FFortLootTierData* Value = LootTier.Value();

        if (TierGroupName == Value->TierGroup && Value->Weight != 0 && (ForcedLootTier == -1 || ForcedLootTier == Value->LootTier))
        {
            TierGroupLTDs.Add(Value);
        }
    }

    if (TierGroupLTDs.Num() == 0)
        return LootDrops;

    FFortLootTierData* ChosenRowLootTierData = GetWeightedRandom(TierGroupLTDs);

    if (!ChosenRowLootTierData)
    {
        if (!bProd)
            UE_LOG(LogServer, Warning, "PickLootDrops: ChosenRowLootTierData is null. Retrying.");
        
        return PickLootDrops(TierGroupName, ForcedLootTier, ++ReturnCount);
    }

    if (ChosenRowLootTierData->LootPackageCategoryMinArray.Num() != ChosenRowLootTierData->LootPackageCategoryWeightArray.Num() ||
            ChosenRowLootTierData->LootPackageCategoryMinArray.Num() != ChosenRowLootTierData->LootPackageCategoryMaxArray.Num())
    {
        if (!bProd)
            UE_LOG(LogServer, Warning, "PickLootDrops: ChosenRowLootTierData Arrays are mismatched! Retrying.");
        
        return PickLootDrops(TierGroupName, ForcedLootTier, ++ReturnCount);
    }
    
    float NumLootPackageDrops = std::floor(ChosenRowLootTierData->NumLootPackageDrops);

    TArray<FFortLootPackageData*> TierGroupLPs;

    for (auto& CurrentLP : LootPackageTable->GetAs<FFortLootPackageData>())
    {
        auto LootPackage = CurrentLP.Value();
        if (LootPackage && LootPackage->LootPackageID == ChosenRowLootTierData->LootPackage && LootPackage->Weight != 0)
            TierGroupLPs.Add(LootPackage);
    }

    auto LPName = ChosenRowLootTierData->LootPackage.ToString();
    
    if (LPName.contains(".Empty"))
    {
        return PickLootDrops(TierGroupName, ForcedLootTier, ++ReturnCount);
    }

    bool bIsWorldList = LPName.contains("WorldList");

    for (float i = 0; i < NumLootPackageDrops; i++)
    {
        if (i >= TierGroupLPs.Num())
            break;

        auto TierGroupLP = TierGroupLPs[i];

        if (TierGroupLP->LootPackageCall.ToString().contains(".Empty"))
        {
            NumLootPackageDrops++;
            continue;
        }

        TArray<FFortLootPackageData*> LootPackageCalls;

        if (bIsWorldList)
        {
            for (int idx = 0; idx < TierGroupLPs.Num(); idx++)
            {
                auto& CurrentLP = TierGroupLPs[idx];

                if (CurrentLP->Weight != 0)
                    LootPackageCalls.Add(CurrentLP);
            }
        }
        else
        {
            for (auto& CurrentLP : LootPackageTable->GetAs<FFortLootPackageData>())
            {
                auto Value = CurrentLP.Value();

                if (Value->LootPackageID.ToString() == TierGroupLP->LootPackageCall.ToString() && Value->Weight != 0)
                    LootPackageCalls.Add(Value);
            }
        }

        if (LootPackageCalls.Num() == 0)
        {
            NumLootPackageDrops++;
            continue;
        }

        auto LootPackageCall = GetWeightedRandom(LootPackageCalls);

        if (!LootPackageCall)
            continue;

        auto ItemDef = LootPackageCall->ItemDefinition.Get();

        if (!ItemDef)
        {
            NumLootPackageDrops++;
            continue;
        }

        FFortItemEntry NewItemEntry{};
        NewItemEntry.ItemDefinition = ItemDef;
        NewItemEntry.LoadedAmmo = GetClipSize(Cast<UFortWeaponItemDefinition>(ItemDef));
        NewItemEntry.Count = LootPackageCall->Count;

        LootDrops.Add(NewItemEntry);
    }
    
    return LootDrops;
}

FName FortKismetLibrary::RedirectLootTierGroup(FName TierGroupName)
{
    static FName Loot_TreasureFName = UKismetStringLibrary::Conv_StringToName(L"Loot_Treasure");
    static FName Loot_AmmoFName = UKismetStringLibrary::Conv_StringToName(L"Loot_Ammo");

    if (TierGroupName == Loot_TreasureFName)
        return UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure");

    if (TierGroupName == Loot_AmmoFName)
        return UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge");

    return TierGroupName;
}

// shameless nova copy
int32 FortKismetLibrary::GetClipSize(UFortWeaponItemDefinition* WeaponDefinition)
{
    if (!WeaponDefinition)
        return 0;
        
    auto Table = WeaponDefinition->WeaponStatHandle.DataTable;

    if (!Table)
        return 0;

    for (auto&& Pair : Table->GetAs<FFortRangedWeaponStats>())
    {
        if (Pair.Key() == WeaponDefinition->WeaponStatHandle.RowName)
            return Pair.Value()->ClipSize;
    }

    return 0;
}

AFortPickupAthena* FortKismetLibrary::SpawnPickup(FFortItemEntry Entry, FVector Loc, EFortPickupSourceTypeFlag SourceFlag, EFortPickupSpawnSource SpawnSource, AFortPlayerPawn* Pawn, bool bToss)
{
    if (auto Pickup = World::SpawnActor<AFortPickupAthena>(Loc))
    {
        if (Entry.Count <= 0)
            Entry.Count = 1;

        if (Entry.LoadedAmmo == 0 && SourceFlag != EFortPickupSourceTypeFlag::Player)
        {
            if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(Entry.ItemDefinition))
                Entry.LoadedAmmo = GetClipSize(WeaponDef);
        }
        
        Pickup->PrimaryPickupItemEntry.ItemDefinition = Entry.ItemDefinition;
        Pickup->PrimaryPickupItemEntry.Count = Entry.Count;
        Pickup->PrimaryPickupItemEntry.LoadedAmmo = Entry.LoadedAmmo;
        Pickup->PrimaryPickupItemEntry.Level = Entry.Level;
        Pickup->PawnWhoDroppedPickup = Pawn;
        Pickup->bRandomRotation = true;

        Pickup->TossPickup(Loc, Pawn, 0, bToss, SourceFlag, SpawnSource);

        if (SourceFlag == EFortPickupSourceTypeFlag::Container)
            Pickup->bTossedFromContainer = true;

        return Pickup;
    } 

    return nullptr;
}

AFortPickupAthena* FortKismetLibrary::SpawnPickup(UFortItemDefinition* ItemDef, FVector Loc, int Count, EFortPickupSourceTypeFlag SourceFlag, EFortPickupSpawnSource SpawnSource, AFortPlayerPawn* Pawn, bool bToss)
{
    FFortItemEntry NewEntry = {};

    int LoadedAmmo = 0;

    if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDef))
        LoadedAmmo = GetClipSize(WeaponDef);

    NewEntry.ItemDefinition = ItemDef;
    NewEntry.Count = Count;
    NewEntry.Level = 0;
    NewEntry.LoadedAmmo = LoadedAmmo;

    return SpawnPickup(NewEntry, Loc, SourceFlag, SpawnSource, Pawn, bToss);
}
