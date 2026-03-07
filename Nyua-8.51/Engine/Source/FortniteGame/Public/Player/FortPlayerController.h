#pragma once

#include "CoreGlobals.h"

class FortPlayerController
{
private:
    class Originals
    {
    public:
        static inline void (*ServerLoadingScreenDropped)(AFortPlayerController*);
        static inline void (*GetPlayerViewPoint)(AFortPlayerController*, FVector&, FRotator&);
    };
    
public:
    static void ServerLoadingScreenDropped(AFortPlayerController* PlayerController);
    static void GetPlayerViewPoint(AFortPlayerController* PlayerController, FVector& Loc, FRotator& Rot);

    // building
    static void ServerCreateBuildingActor(AFortPlayerController* PlayerController, FCreateBuildingActorData CreateBuildingData);
    static void ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
    static void ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* Building, TSubclassOf<ABuildingSMActor> BuildingClass, uint8 Rot, bool Mirrored);
    static void ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);

    // inventory
    static bool ServerRemoveInventoryItem(AFortPlayerController* PlayerController, FGuid& ItemGuid, int Count, bool bForceRemoval,bool bForcePersistWhenEmpty);
    static bool RemoveInventoryItem(void* InventoryOwner, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval);
    static void ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, const struct FGuid& ItemGuid, int32 Count);

    static void Setup();
};