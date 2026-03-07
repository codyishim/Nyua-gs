#include "Items/FortPickup.h"

#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"

void FortPickup::OnAboutToEnterBackpack(AFortPickupAthena* Pickup)
{
    auto Controller = Cast<AFortPlayerControllerAthena>(Pickup->PickupLocationData.ItemOwner->Controller);

    if (!Controller)
        return Originals::OnAboutToEnterBackpack(Pickup);

    bool bWasSwapped = false;
    
    auto NewItem = FortInventory::GiveItem(Controller, Pickup->PrimaryPickupItemEntry.ItemDefinition, Pickup->PrimaryPickupItemEntry.Count, Pickup->PrimaryPickupItemEntry.LoadedAmmo, 0, bWasSwapped);

    if (bWasSwapped && NewItem)
        Controller->ClientEquipItem(NewItem->ItemEntry.ItemGuid, false);
    
    return Originals::OnAboutToEnterBackpack(Pickup);        
}

void FortPickup::Setup()
{
    UDetoursLibrary::InitializeDetour<AFortPickupAthena, MinHook>(0x1453B50, OnAboutToEnterBackpack, &Originals::OnAboutToEnterBackpack);
}
