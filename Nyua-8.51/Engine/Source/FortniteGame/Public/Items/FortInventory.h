#pragma once

#include "CoreGlobals.h"
#include <type_traits>

class FortInventory
{
public:
    template <typename T, typename Key>
        static inline T* FindItem(AFortPlayerController* PlayerController, Key Identifier)
    {
        if constexpr (std::is_same_v<T, FFortItemEntry>)
        {
            if constexpr (std::is_same_v<Key, UFortItemDefinition*>)
            {
                for (FFortItemEntry& ItemEntry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
                {
                    if (ItemEntry.ItemDefinition == Identifier)
                        return &ItemEntry;
                }
            }
            else if constexpr (std::is_same_v<Key, FGuid>)
            {
                for (FFortItemEntry& ItemEntry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
                {
                    if (ItemEntry.ItemGuid == Identifier)
                        return &ItemEntry;
                }
            }
        }
            
        else if constexpr (std::is_same_v<T, UFortWorldItem>)
        {
            if constexpr (std::is_same_v<Key, UFortItemDefinition*>)
            {
                for (UFortWorldItem* WorldItem : PlayerController->WorldInventory->Inventory.ItemInstances)
                {
                    if (WorldItem->ItemEntry.ItemDefinition == Identifier)
                        return WorldItem;
                }
            }
            else if constexpr (std::is_same_v<Key, FGuid>)
            {
                for (UFortWorldItem* WorldItem : PlayerController->WorldInventory->Inventory.ItemInstances)
                {
                    if (WorldItem->ItemEntry.ItemGuid == Identifier)
                        return WorldItem;
                }
            }
        }

        return nullptr;
    }

    static inline bool IsPrimaryQuickBar(UFortItemDefinition* ItemDefinition)
    {
        return
            !ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) &&
            !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
            !ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) &&
            !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
            !ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) &&
            !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass());
    }

    static inline bool IsQuickBarFull(AFortPlayerController* PlayerController, bool bSecondary = false)
    {
        int SlotsFilled = 0;

        for (auto& Item : PlayerController->WorldInventory->Inventory.ItemInstances)
        {
            if ((IsPrimaryQuickBar(Item->ItemEntry.ItemDefinition) && !bSecondary) ||
                (!IsPrimaryQuickBar(Item->ItemEntry.ItemDefinition) && bSecondary))
            {
                if (++SlotsFilled >= 5)
                    return true;
            }
        }

        return false;
    }
    
    static UFortWorldItem* GiveItem(AFortPlayerController* PlayerController, UFortItemDefinition* Definition, int Count = 1, int LoadedAmmo = 0, int Level = 0, bool& bWasSwapped = *new bool);
    static bool RemoveItem(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count);
    static bool ReplaceEntry(AFortPlayerController* PlayerController, FFortItemEntry& Entry);
    static void SetLoadedAmmo(UFortWorldItem* Item, int LoadedAmmo);
    static void ClearInventory(AFortPlayerController* PlayerController);
    
    static class AFortPlayerController* GetPlayerControllerFromInventoryOwner(void* InventoryOwner)
    {
        if (!InventoryOwner) return nullptr;
        return (AFortPlayerController*)(__int64(InventoryOwner) - 0x598);
    }
    
    static void Setup();
};