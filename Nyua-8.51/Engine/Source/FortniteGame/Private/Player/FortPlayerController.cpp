#include "Player/FortPlayerController.h"

#include "Engine/World.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"
#include <Creative/FortPlaysetItemDefinition.h>

#include "FortKismetLibrary.h"
#include "Creative/ImproperIslandSaving.h"
#include "Engine/Plugins/MemoryLibary/Source/Public/MemoryLibrary.h"
#include "Vendor/Memcury.h"

// i reversed 1.8 - andrew
void FortPlayerController::ServerCreateBuildingActor(AFortPlayerController *PlayerController, FCreateBuildingActorData CreateBuildingData)
{
    AFortGameStateAthena *GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
    TArray<ABuildingActor *> BuildingActors;

    for (auto &BuildableClass : GameState->AllPlayerBuildableClassesIndexLookup)
    {
        if (BuildableClass.Value() == CreateBuildingData.BuildingClassHandle)
            CreateBuildingData.BuildingClassData.BuildingClass = BuildableClass.Key();
    }

    if (auto BuildingClass = CreateBuildingData.BuildingClassData.BuildingClass)
    {
        EFortBuildPreviewMarkerOptionalAdjustment Adjustment;

        EFortStructuralGridQueryResults QueryResult = GameState->StructuralSupportSystem->CanAddBuildingActorClassToGrid(
            UWorld::GetWorld(),
            BuildingClass,
            CreateBuildingData.BuildLoc,
            CreateBuildingData.BuildRot,
            CreateBuildingData.bMirrored,
            &BuildingActors,
            &Adjustment,
            false);

        if (QueryResult == EFortStructuralGridQueryResults::CanAdd)
        {
            for (int32 i = 0; i < BuildingActors.Num(); i++)
            {
                ABuildingActor *ExistingBuilding = BuildingActors[i];
                if (!ExistingBuilding)
                    continue;

                ExistingBuilding->K2_DestroyActor();
            }

            static void *(*CanAffordToPlaceBuildableClass)(AFortPlayerController *, FBuildingClassData *) = decltype(CanAffordToPlaceBuildableClass)(
                InSDKUtils::GetImageBase() + 0x16CFEE0);
            if (!CanAffordToPlaceBuildableClass(PlayerController, &CreateBuildingData.BuildingClassData))
                return;
            
            static void* (*PayBuildableClassPlacementCost)(AFortPlayerController*, FBuildingClassData*) = decltype(PayBuildableClassPlacementCost)(
                InSDKUtils::GetImageBase() + 0x16FB8E0);
            
            PayBuildableClassPlacementCost(PlayerController, &CreateBuildingData.BuildingClassData);

            ABuildingSMActor *BuildingSMActor = World::SpawnActor<ABuildingSMActor>(CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, BuildingClass);
            if (BuildingSMActor)
            {
                BuildingSMActor->CurrentBuildingLevel = CreateBuildingData.BuildingClassData.UpgradeLevel = 0;

                BuildingSMActor->InitializeKismetSpawnedBuildingActor(BuildingSMActor, PlayerController, true);
                BuildingSMActor->bPlayerPlaced = true;

                AFortPlayerStateAthena *PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
                if (PlayerState)
                {
                    BuildingSMActor->Team = EFortTeam(PlayerState->TeamIndex);
                    BuildingSMActor->OnRep_Team();
                    BuildingSMActor->SetTeam(PlayerState->TeamIndex);
                }
            }
        }
    }

    if (BuildingActors.IsValid())
        BuildingActors.Free();
}

void FortPlayerController::ServerBeginEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
    AFortPlayerStateZone* PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerController->PlayerState);
    if (!PlayerStateZone || !BuildingActorToEdit || !PlayerController->MyFortPawn)
        return;

    static void* (*SetEditingPlayer)(ABuildingSMActor*, AFortPlayerController*) = decltype(SetEditingPlayer)(InSDKUtils::GetImageBase() + 0x1127540);
    SetEditingPlayer(BuildingActorToEdit, PlayerController);

    static UFortGameData* GameData = nullptr;
    if (!GameData)
    {
        UFortAssetManager* AssetManager = (UFortAssetManager*)UFortEngine::GetEngine()->AssetManager;
        GameData = AssetManager->GameData;
    }
        
    static UFortEditToolItemDefinition* EditToolItemDefinition = GameData->EditToolItem.Get();
    if (!EditToolItemDefinition)
    {
        const FString& AssetPathName = UKismetStringLibrary::Conv_NameToString(GameData->EditToolItem.ObjectID.AssetPathName);
        EditToolItemDefinition = StaticLoadObject<UFortEditToolItemDefinition>(AssetPathName.ToString().c_str());
    }

    FFortItemEntry* ItemEntry = FortInventory::FindItem<FFortItemEntry>(PlayerController, (UFortItemDefinition*)EditToolItemDefinition);

    if (ItemEntry && 
        EditToolItemDefinition && 
        PlayerController->MyFortPawn->EquipWeaponDefinition(EditToolItemDefinition, ItemEntry->ItemGuid))
    {
        AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->CurrentWeapon);

        if (EditingTool)
        {
            EditingTool->EditActor = BuildingActorToEdit;
            EditingTool->OnRep_EditActor();
        }
    }
}

void FortPlayerController::ServerEditBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> BuildingClass, uint8 Rot, bool Mirrored)
{
    if (!PlayerController || !BuildingActorToEdit || !BuildingActorToEdit->IsA(ABuildingSMActor::StaticClass()) || !PlayerController->MyFortPawn)
        return;
    
    static void* (*SetEditingPlayer)(ABuildingSMActor*, AFortPlayerController*) = decltype(SetEditingPlayer)(InSDKUtils::GetImageBase() + 0x1127540);
    SetEditingPlayer(BuildingActorToEdit, nullptr);

    static auto ReplaceBuildingActor = (ABuildingSMActor * (*)(ABuildingSMActor*, unsigned int, UObject*, unsigned int, int, bool, AFortPlayerController*))(InSDKUtils::GetImageBase() + 0x11252b0);
    
    int32 CurrentBuildingLevel = BuildingActorToEdit->CurrentBuildingLevel;
    ABuildingSMActor* NewBuild = ReplaceBuildingActor(BuildingActorToEdit, 1, BuildingClass, CurrentBuildingLevel, Rot, Mirrored, PlayerController);
    if (NewBuild) NewBuild->bPlayerPlaced = true;
}

void FortPlayerController::ServerEndEditingBuildingActor(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
    if (!BuildingActorToEdit ||
        BuildingActorToEdit->EditingPlayer != PlayerController->PlayerState ||
        BuildingActorToEdit->bDestroyed || !PlayerController->MyFortPawn)
        return;

    static void* (*SetEditingPlayer)(ABuildingSMActor*, AFortPlayerController*) = decltype(SetEditingPlayer)(InSDKUtils::GetImageBase() + 0x1127540);
    SetEditingPlayer(BuildingActorToEdit, nullptr);

    AFortWeap_EditingTool* EditingTool = Cast<AFortWeap_EditingTool>(PlayerController->MyFortPawn->CurrentWeapon);
    if (EditingTool)
    {
        EditingTool->EditActor = BuildingActorToEdit;
        EditingTool->OnRep_EditActor();
    }
}

void FortPlayerController::GetPlayerViewPoint(AFortPlayerController* PlayerController, FVector& Loc, FRotator& Rot)
{
    static auto Spectating = UKismetStringLibrary::Conv_StringToName(L"Spectating");
    if (PlayerController->StateName == Spectating)
    {
        Loc = PlayerController->LastSpectatorSyncLocation;
        Rot = PlayerController->LastSpectatorSyncRotation;
    }
    else if (PlayerController->GetViewTarget())
    {
        Loc = PlayerController->GetViewTarget()->K2_GetActorLocation();
        Rot = PlayerController->GetControlRotation();
    }
    else return Originals::GetPlayerViewPoint(PlayerController, Loc, Rot);
}

void FortPlayerController::ServerLoadingScreenDropped(AFortPlayerController *PlayerController)
{
    auto *GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
    if (!GameMode) return Originals::ServerLoadingScreenDropped(PlayerController);

    auto *GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
    auto *PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
    auto *NewPlayer = Cast<AFortPlayerControllerAthena>(PlayerController);
    
    if (!GameState || !PlayerState || !NewPlayer)
        return Originals::ServerLoadingScreenDropped(PlayerController);
    
    if (bCreative)
    {
		auto CreativePortalManager = GameState->CreativePortalManager;
        GameState->CreativePortalManager->GetCreativePortalManager(UWorld::GetWorld(), &CreativePortalManager, nullptr);
        AFortAthenaCreativePortal *Portal = nullptr;

        if (CreativePortalManager->AvailablePortals.Num() > 0)
        {
            Portal = CreativePortalManager->AvailablePortals[0];
            CreativePortalManager->AvailablePortals.Remove(0);
            CreativePortalManager->UsedPortals.Add(Portal);
        }
		
        if (!Portal) return;

        Portal->GetLinkedVolume()->bShowPublishWatermark = false;
        Portal->GetLinkedVolume()->bNeverAllowSaving = false;
        Portal->GetLinkedVolume()->VolumeState = EVolumeState::Ready;
        Portal->GetLinkedVolume()->OnRep_VolumeState();

        Portal->OwningPlayer = PlayerState->UniqueId;
        Portal->OnRep_OwningPlayer();

        Portal->IslandInfo.CreatorName = PlayerState->GetPlayerName();
        Portal->IslandInfo.SupportCode = L"Nyua";
        Portal->IslandInfo.Version = 1.0f;
        Portal->OnRep_IslandInfo();

        Portal->bPortalOpen = true;
        Portal->OnRep_PortalOpen();

        Portal->PlayersReady.Add(PlayerState->UniqueId);
        Portal->OnRep_PlayersReady();

        Portal->bUserInitiatedLoad = true;
        Portal->bInErrorState = false;

        NewPlayer->OwnedPortal = Portal;

        auto *LevelSaveComponent = (UFortLevelSaveComponent *)Portal->GetLinkedVolume()->GetComponentByClass(UFortLevelSaveComponent::StaticClass());
        LevelSaveComponent->AccountIdOfOwner = PlayerState->UniqueId;
        LevelSaveComponent->bIsLoaded = true;
    	LevelSaveComponent->OnRep_IsActive();
		
        NewPlayer->CreativePlotLinkedVolume = Portal->GetLinkedVolume();
        NewPlayer->OnRep_CreativePlotLinkedVolume();

        static auto MinigameSettingsMachine = StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/MinigameSettingsMachine.MinigameSettingsMachine_C");
		auto OK = Portal->GetLinkedVolume()->GetTransform();
    	World::SpawnActorOG<AActor>(MinigameSettingsMachine, OK, Portal->LinkedVolume);

        static auto Playset = StaticLoadObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");
        FortPlaysetItemDefinition::LoadPlayset(Playset, Portal->GetLinkedVolume());

	// LoadFromDSSAsync    ((void (*)(UFortLevelSaveComponent*, bool)) (Memcury::Scanner::FindPattern("48 8B C4 88 50 ? 55 53 41 55").Get()))(LevelSaveComponent, true);
    }

    return Originals::ServerLoadingScreenDropped(PlayerController);
}

void FortPlayerController::ServerAttemptInventoryDrop(AFortPlayerController* PlayerController, const struct FGuid& ItemGuid, int32 Count)
{
    if (!PlayerController || !PlayerController->Pawn)
        return;

    auto ItemEntry = FortInventory::FindItem<FFortItemEntry>(PlayerController, ItemGuid);
    if (!ItemEntry || (ItemEntry->Count - Count) < 0)
        return;

    ItemEntry->Count -= Count;
    FortKismetLibrary::SpawnPickup(*ItemEntry, PlayerController->Pawn->K2_GetActorLocation() + PlayerController->Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, nullptr, true);
    if (ItemEntry->Count == 0)
        FortInventory::RemoveItem(PlayerController, ItemGuid, Count);
    else
        FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
}

bool FortPlayerController::ServerRemoveInventoryItem(AFortPlayerController* PlayerController, FGuid& ItemGuid, int Count, bool bForceRemoval, bool bForcePersistWhenEmpty)
{
    auto ItemEntry = FortInventory::FindItem<FFortItemEntry>(PlayerController, ItemGuid);
    if (!ItemEntry) return false;

    ItemEntry->Count -= Count;
    FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
    
    return true;
}

bool FortPlayerController::RemoveInventoryItem(void* InventoryOwner, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval)
{
    AFortPlayerController* PlayerController = FortInventory::GetPlayerControllerFromInventoryOwner(InventoryOwner);
    if (!PlayerController) return false;

    FGuid Ok = ItemGuid;
    ServerRemoveInventoryItem(PlayerController, Ok, Count, bForceRemoveFromQuickBars, bForceRemoval);
    
    return true;
}

std::vector<std::string> ParseCommand(const std::string& Input)
{
    std::vector<std::string> Result{};
    std::string Str = Input;
    while (Str.find(" ") != std::string::npos)
    {
        auto arg = Str.substr(0, Str.find(' '));
        Result.push_back(arg);
        Str.erase(0, Str.find(' ') + 1);
    }
    Result.push_back(Str);
    return Result;
}

static void ServerCheat(AFortPlayerControllerAthena* PC, FString FCommand)
{
    auto PS = Cast<AFortPlayerStateAthena>(PC->PlayerState);

    if (!PS)
    {
        return;
    }
   

    
    std::vector<std::string> Arguments = ParseCommand(FCommand.ToString());
    auto Command = Arguments[0];
    auto GS = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
    auto GM = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);

    if (Command.contains("loadisland"))
    {
        Builder::LoadIsland(PC);
    }
    else if(Command.contains("saveisland"))
    {
        Builder::SaveIsland(PC);
    }
}


void FortPlayerController::Setup()
{

    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x1BA, ServerCheat);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, MinHook>(0x16e2230, GetPlayerViewPoint, &Originals::GetPlayerViewPoint);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x261, ServerLoadingScreenDropped, &Originals::ServerLoadingScreenDropped);

    // building
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x223, ServerCreateBuildingActor);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x22A, ServerBeginEditingBuildingActor);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x225, ServerEditBuildingActor);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x228, ServerEndEditingBuildingActor);

    // inventory (proper!!) - andrew
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x1F7, ServerRemoveInventoryItem);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x210, ServerAttemptInventoryDrop);

    UObject* InventoryOwner = (UObject*)(__int64(AFortPlayerControllerAthena::GetDefaultObj()) + 0x598);

    DWORD oldProtect;
    VirtualProtect(&InventoryOwner->VTable[0x16], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect);
    InventoryOwner->VTable[0x16] = RemoveInventoryItem;
    VirtualProtect(&InventoryOwner->VTable[0x16], sizeof(void*), oldProtect, &oldProtect);
}