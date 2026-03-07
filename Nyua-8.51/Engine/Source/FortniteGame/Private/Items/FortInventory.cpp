#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"

#include "FortKismetLibrary.h"
#include "Vendor/Memcury.h"

UFortWorldItem* FortInventory::GiveItem(
    AFortPlayerController* PlayerController, 
    UFortItemDefinition* Definition, 
    int Count, 
    int LoadedAmmo, 
    int Level,
    bool& bWasSwapped
)
{
    bWasSwapped = false;
    
    if (Count <= 0 || !PlayerController || !PlayerController->WorldInventory || !Definition)
        return nullptr;

    AFortInventory* Inventory = PlayerController->WorldInventory;
    int32 Remaining = Count;
    UFortWorldItem* ResultItem = nullptr;

    for (auto& ItemInstance : Inventory->Inventory.ItemInstances)
    {
        if (ItemInstance->ItemEntry.ItemDefinition == Definition)
        {
            int32 CurrentCount = ItemInstance->ItemEntry.Count;
            int32 MaxStack = Definition->MaxStackSize;
            int32 SpaceLeft = MaxStack - CurrentCount;

            if (SpaceLeft > 0)
            {
                int32 ToAdd = UKismetMathLibrary::Min(Remaining, SpaceLeft);
                ItemInstance->ItemEntry.Count += ToAdd;

                auto ItemEntry = FindItem<FFortItemEntry>(PlayerController, ItemInstance->ItemEntry.ItemGuid);
                ItemEntry->Count = ItemInstance->ItemEntry.Count;

                Remaining -= ToAdd;

                Inventory->Inventory.MarkItemDirty(*ItemEntry);
                ResultItem = ItemInstance;
            }

            if (Remaining <= 0)
                break;
        }
    }

    if (Remaining > 0 && (Definition->IsA(UFortWeaponItemDefinition::StaticClass()) || Definition->IsA(UFortGadgetItemDefinition::StaticClass())))
    {
        if (IsQuickBarFull(PlayerController))
        {
            FVector SpawnLocation = PlayerController->Pawn
                ? PlayerController->Pawn->K2_GetActorLocation()
                : FVector();

            if (auto* CurrentWeapon = PlayerController->MyFortPawn->CurrentWeapon)
            {
                if (auto ItemInstance = FindItem<UFortWorldItem>(PlayerController, CurrentWeapon->ItemEntryGuid))
                {
                    auto ItemEntry = ItemInstance->ItemEntry;

                    if (ItemInstance->CanBeDropped())
                    {
                        FortKismetLibrary::SpawnPickup(
                            ItemEntry,
                            SpawnLocation,
                            EFortPickupSourceTypeFlag::Player,
                            EFortPickupSpawnSource::Unset,
                            PlayerController->MyFortPawn,
                            true
                        );

                        RemoveItem(PlayerController, ItemEntry.ItemGuid, ItemEntry.Count);
                        bWasSwapped = true;
                    }
                    else
                    {
                        FortKismetLibrary::SpawnPickup(
                            Definition,
                            SpawnLocation,
                            Remaining,
                            EFortPickupSourceTypeFlag::Player,
                            EFortPickupSpawnSource::Unset,
                            PlayerController->MyFortPawn,
                            true
                        );

                        bWasSwapped = true;
                        return nullptr;
                    }
                }
            }
            else
            {
                FortKismetLibrary::SpawnPickup(
                    Definition,
                    SpawnLocation,
                    Remaining,
                    EFortPickupSourceTypeFlag::Player,
                    EFortPickupSpawnSource::Unset,
                    PlayerController->MyFortPawn,
                    true
                );
                Remaining = 0;
            }
        }
    }

    if (Remaining > 0)
    {
        if (UFortWorldItem* NewItem = Cast<UFortWorldItem>(Definition->CreateTemporaryItemInstanceBP(Remaining, Level)))
        {
            FFortItemEntryStateValue StateValue{};
            StateValue.IntValue = 1;
            StateValue.StateType = EFortItemEntryState::ShouldShowItemToast;

            NewItem->SetOwningControllerForTemporaryItem(PlayerController);
            NewItem->ItemEntry.Count = Remaining;
            NewItem->ItemEntry.LoadedAmmo = LoadedAmmo;
            NewItem->ItemEntry.ItemDefinition = Definition;
            NewItem->ItemEntry.StateValues.Add(StateValue);
            NewItem->ItemEntry.Level = Level;

            Inventory->Inventory.ItemInstances.Add(NewItem);
            Inventory->Inventory.ReplicatedEntries.Add(NewItem->ItemEntry);
            Inventory->Inventory.MarkItemDirty(NewItem->ItemEntry);
            
            ResultItem = NewItem;
            Remaining = 0;
        }
    }

    if (ResultItem)
    {
        Inventory->bRequiresLocalUpdate = true;
        Inventory->HandleInventoryLocalUpdate();
        Inventory->Inventory.MarkArrayDirty();

        if (!bProd)
            UE_LOG(LogServer, Log, TEXT("Updating PC Inventory"));
    }

    return ResultItem;
}

void FortInventory::ClearInventory(AFortPlayerController* PlayerController)
{
    static auto FortEditToolItemDefinitionClass = StaticFindObject<UClass>("/Script/FortniteGame.FortEditToolItemDefinition");
    static auto FortBuildingItemDefinitionClass = StaticFindObject<UClass>("/Script/FortniteGame.FortBuildingItemDefinition");

    std::vector<std::pair<FGuid, int>> GuidsAndCountsToRemove;
    const auto& ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;

    UFortWorldItem* PickaxeInstance = nullptr;
    for (int i = 0; i < ItemInstances.Num(); ++i)
    {
        auto ItemInstance = ItemInstances[i];

        if (ItemInstance->ItemEntry.ItemDefinition && ItemInstance->ItemEntry.ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
        {
            PickaxeInstance = ItemInstance;
            break;
        }
    }

    for (int i = 0; i < ItemInstances.Num(); ++i)
    {
        auto ItemInstance = ItemInstances[i];
        const auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemInstance->ItemEntry.ItemDefinition);

        if (!ItemDefinition->IsA(FortBuildingItemDefinitionClass)
            && !ItemDefinition->IsA(FortEditToolItemDefinitionClass)
            && ItemInstance != PickaxeInstance)
        {
            GuidsAndCountsToRemove.push_back({
                ItemInstance->ItemEntry.ItemGuid,
                ItemInstance->ItemEntry.Count
                });
        }
    }

    for (auto& [Guid, Count] : GuidsAndCountsToRemove)
    {
        RemoveItem(PlayerController, Guid, Count);
    }
}

bool FortInventory::RemoveItem(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count)
{
    auto WorldInventory = PlayerController->WorldInventory;
    
    for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
    {
        auto& ItemInstance = WorldInventory->Inventory.ItemInstances[i];

        if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
        {
            if (Count >= ItemInstance->ItemEntry.Count)
                WorldInventory->Inventory.ItemInstances.Remove(i);
            else
            {
                ItemInstance->ItemEntry.Count -= Count;
                WorldInventory->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
            }
            break;
        }
    }

    for (int i = 0; i < WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
    {
        auto& ReplicatedEntry = WorldInventory->Inventory.ReplicatedEntries[i];

        if (ReplicatedEntry.ItemGuid == ItemGuid)
        {
            if (Count >= ReplicatedEntry.Count)
                WorldInventory->Inventory.ReplicatedEntries.Remove(i);
            else
            {
                ReplicatedEntry.Count -= Count;
                WorldInventory->Inventory.MarkItemDirty(ReplicatedEntry);
            }
            break;
        }
    }

    WorldInventory->bRequiresLocalUpdate = true;
    WorldInventory->HandleInventoryLocalUpdate();
    WorldInventory->Inventory.MarkArrayDirty();

    return true;
}

bool FortInventory::ReplaceEntry(AFortPlayerController* PlayerController, FFortItemEntry& Entry)
{
    bool bSuccess = false;
    if (!PlayerController || !PlayerController->WorldInventory) 
        return bSuccess;

    if (Entry.Count <= 0)
    {
        RemoveItem(PlayerController, Entry.ItemGuid, Entry.Count);
        return bSuccess = true;
    }

    AFortInventory* Inventory = PlayerController->WorldInventory;

    for (int i = 0; i < Inventory->Inventory.ItemInstances.Num(); i++)
    {
        if (Inventory->Inventory.ItemInstances[i]->ItemEntry.ItemGuid == Entry.ItemGuid)
        {
            Inventory->Inventory.ItemInstances[i]->ItemEntry = Entry;
            
            bSuccess = true;
            break;
        }
    }

    for (int i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
    {
        if (Inventory->Inventory.ReplicatedEntries[i].ItemGuid == Entry.ItemGuid)
        {
            Inventory->Inventory.ReplicatedEntries[i] = Entry;
            Inventory->Inventory.MarkItemDirty(Entry);
            bSuccess = true;
            break;
        }
    }

    if (bSuccess)
    {
        Inventory->bRequiresLocalUpdate = true;
        Inventory->HandleInventoryLocalUpdate();
        Inventory->Inventory.MarkArrayDirty();
    }

    return bSuccess;
}

void FortInventory::SetLoadedAmmo(UFortWorldItem* Item, int LoadedAmmo)
{
    Item->ItemEntry.LoadedAmmo = LoadedAmmo;
    ReplaceEntry(Item->GetOwningController(), Item->ItemEntry);   
}

void FortInventory::Setup()
{
    UDetoursLibrary::InitializeDetour<UFortWorldItem, EveryVFT>(0x94, SetLoadedAmmo);
}
