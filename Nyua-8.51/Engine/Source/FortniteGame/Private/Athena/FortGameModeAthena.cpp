#include "Athena/FortGameModeAthena.h"

#include <algorithm>

#include "Engine/World.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include <Items/FortInventory.h>
#include <Creative/FortPlaysetItemDefinition.h>

#include "FortKismetLibrary.h"
#include <chrono>

#include "nlohmann.hpp"
#include "Engine/Plugins/MemoryLibary/Source/Public/MemoryLibrary.h"

static void ShowFoundation(ABuildingFoundation* Foundation)
{
	if (!Foundation) return;

	Foundation->StreamingData.BoundingBox = Foundation->StreamingBoundingBox;
	Foundation->StreamingData.FoundationLocation = Foundation->GetTransform().Translation;
	Foundation->SetDynamicFoundationEnabled(true);
}

bool FortGameModeAthena::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
    auto GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
    
    if (!GameState) return false;
    if (!GameState->MapInfo) return false;
    
    if (!GameMode->bWorldIsReady)
    {
        UFortPlaylistAthena* Playlist = bCreative
            ? UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_PlaygroundV2.Playlist_PlaygroundV2")
            : UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");

    	if (bProd)
    	{
    		auto it = std::find_if(args.begin(), args.end(), [](const std::wstring& s) { 
				return s.rfind(L"-playlist=", 0) == 0; 
			});

    		if (it != args.end())
    		{
    			std::wstring PlaylistID = it->substr(10);
    			std::transform(PlaylistID.begin(), PlaylistID.end(), PlaylistID.begin(), ::towlower);

    			for (auto& It : GetObjectsOfClass(UFortPlaylistAthena::StaticClass()))
    			{
    				auto Ittt = (UFortPlaylistAthena*)It;
    				if (Ittt)
    				{
    					std::string playlistStr = Ittt->PlaylistName.ToString();
                
    					std::wstring playlistName(playlistStr.begin(), playlistStr.end());
    					std::transform(playlistName.begin(), playlistName.end(), playlistName.begin(), ::towlower);

    					if (wcsstr(playlistName.c_str(), PlaylistID.c_str()) != NULL)
    					{
    						Playlist = Ittt;
    						break;
    					}
    				}
    			}
    		}
    	}
    	
        GameMode->WarmupRequiredPlayerCount = 1;
        
        GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
        GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
        GameState->CurrentPlaylistInfo.MarkArrayDirty();
        GameState->OnRep_CurrentPlaylistInfo();
        
        GameState->CurrentPlaylistId = Playlist->PlaylistId;
        GameState->OnRep_CurrentPlaylistId();
        
        GameMode->CurrentPlaylistName = Playlist->PlaylistName;
        GameMode->CurrentPlaylistId = Playlist->PlaylistId;

        GameMode->GameSession->MaxPlayers = 100;

    	for (auto& Level : Playlist->AdditionalLevels)
        {
            bool bSuccess = false;
        	auto LevelName = Level.ObjectID.AssetPathName;
            ULevelStreamingDynamic::LoadLevelInstance(UWorld::GetWorld(), UKismetStringLibrary::Conv_NameToString(LevelName), {}, {}, &bSuccess);
        	if (bSuccess) GameState->AdditionalPlaylistLevelsStreamed.Add(LevelName);
        }

        GameState->OnRep_AdditionalPlaylistLevelsStreamed();
        GameState->OnRep_AdditionalPlaylistLevelsStreamed();
		
		auto blocky = StaticFindObject<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.SLAB_2");
		auto VolcanoFoundy = StaticFindObject<ABuildingFoundation>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_50x53_Volcano");
		ShowFoundation(VolcanoFoundy);
		ShowFoundation(blocky);

     	auto SpawnIsland_FloorLoot = StaticFindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
		auto BRIsland_FloorLoot = StaticFindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

		TArray<AActor*> SpawnIsland_FloorLoot_Actors;
		UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), SpawnIsland_FloorLoot, &SpawnIsland_FloorLoot_Actors);

		TArray<AActor*> BRIsland_FloorLoot_Actors;
		UGameplayStatics::GetDefaultObj()->GetAllActorsOfClass(UWorld::GetWorld(), BRIsland_FloorLoot, &BRIsland_FloorLoot_Actors);

		auto SpawnIslandTierGroup = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");
		auto BRIslandTierGroup = UKismetStringLibrary::GetDefaultObj()->Conv_StringToName(L"Loot_AthenaFloorLoot");

		EFortPickupSourceTypeFlag SpawnFlag = EFortPickupSourceTypeFlag::FloorLoot;

		for (int i = 0; i < SpawnIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)SpawnIsland_FloorLoot_Actors[i];

			auto Location = CurrentActor->K2_GetActorLocation() + FVector(0, 0, 100);

			TArray<FFortItemEntry> LootDrops = FortKismetLibrary::PickLootDrops(SpawnIslandTierGroup);

			if (LootDrops.Num())
			{
				for (auto& LootDrop : LootDrops)
					FortKismetLibrary::SpawnPickup(LootDrop, Location, SpawnFlag);
			}
		}

		for (int i = 0; i < BRIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)BRIsland_FloorLoot_Actors[i];

			auto Location = CurrentActor->K2_GetActorLocation() + FVector(0, 0, 100);

			TArray<FFortItemEntry> LootDrops = FortKismetLibrary::PickLootDrops(BRIslandTierGroup);

			if (LootDrops.Num())
			{
				for (auto& LootDrop : LootDrops)
					FortKismetLibrary::SpawnPickup(LootDrop, Location, SpawnFlag);
			}
		}

    	if (bCreative)
    	{
    		UDetoursLibrary::InitializeDetour<AGameModeBase, MinHook>(0xFAABC0, UDetoursLibrary::ReturnFalseDetour); // prelogin
    	}
    	
        GameMode->bWorldIsReady = true;
        
        UE_LOG(LogFort, Log, "Set Playlist and World is Ready, you may join!");

        SetConsoleTitleA(std::format("Nyua 8.51 | Listening on Port {} | Joinable = True", Port).c_str());
    }
    
    return Originals::ReadyToStartMatch(GameMode);
}

void FortGameModeAthena::HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer)
{
    AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
    if (!GameState) return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);

    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);
    if (!PlayerState) return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);

    AbilitySystemComponent::GivePlayerAbilities(PlayerState->AbilitySystemComponent);

    for (auto& StartingItem : GameMode->StartingItems)
    {
        if (!StartingItem.Item)
            continue;
        
        if (StartingItem.Item->GetName().contains("Smart"))
            continue;

        FortInventory::GiveItem(NewPlayer, StartingItem.Item, StartingItem.Count);
    }

	NewPlayer->MatchReport = (UAthenaPlayerMatchReport*)UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), NewPlayer);

	FGameMemberInfo Member;
	Member.MostRecentArrayReplicationKey = -1;
	Member.ReplicationID = -1;
	Member.ReplicationKey = -1;
	Member.TeamIndex = PlayerState->TeamIndex; 
	Member.SquadId = PlayerState->SquadId;
	Member.MemberUniqueId = PlayerState->UniqueId;

	GameState->GameMemberInfoArray.Members.Add(Member);
	GameState->GameMemberInfoArray.MarkItemDirty(Member);
	
    return Originals::HandleStartingNewPlayer(GameMode, NewPlayer);    
}

APawn* FortGameModeAthena::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	auto Pawn = GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
    
    return Pawn;
}

static float precision(float f, float places)
{
	float n = powf(10.0f, places);
	return round(f * n) / n;
}

inline FVector_NetQuantize100 Quantize100(FVector p)
{
	FVector_NetQuantize100 ret;
	ret.X = precision(p.X, 2);
	ret.Y = precision(p.Y, 2);
	ret.Z = precision(p.Z, 2);
	return ret;
}

static inline DWORD WINAPI StartLGThread(LPVOID)
{
	if (bLategame)
	{
		auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
		auto LocalAircraft = GameState->Aircrafts[0];

		while (GameState->GamePhase == EAthenaGamePhase::Warmup)
		{
			Sleep(1000 / 30);
		}

		while (GameState->GamePhase != EAthenaGamePhase::Aircraft)
		{
			Sleep(1000 / 30);
		}

		float time = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());

		auto start = std::chrono::high_resolution_clock::now();

		while (GameState->GamePhase == EAthenaGamePhase::Aircraft && time < LocalAircraft->GetDropEndTime() + 1) {
			Sleep(1000 / 30);

			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = now - start;
			time += elapsed.count();
			start = now;
		}
	
		GameState->GamePhase = EAthenaGamePhase::SafeZones;
		GameState->OnRep_GamePhase(GameState->GamePhase);
		GameState->GamePhaseStep = EAthenaGamePhaseStep::StormForming;
		GameState->OnRep_GamePhase(GameState->GamePhase);
	} 

	return 0;
}

void FortGameModeAthena::StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int NewSafeZonePhase)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
	if (!GameState) return Originals::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);

	static bool bZoneReversing = false;
    static bool bEnableReverseZone = false;
    static int32 NewLateGameSafeZonePhase = 1;
    static const int32 EndReverseZonePhase = 5;
    static const int32 StartReverseZonePhase = 7;

    AFortSafeZoneIndicator* SafeZoneIndicator = GameMode->SafeZoneIndicator;
    if (!SafeZoneIndicator) return Originals::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);
	
    if (bLategame)
    {
        GameMode->SafeZonePhase = NewLateGameSafeZonePhase;
        GameState->SafeZonePhase = NewLateGameSafeZonePhase;
        
    	Originals::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);
        if (NewLateGameSafeZonePhase == EndReverseZonePhase) bZoneReversing = false;
        if (NewLateGameSafeZonePhase >= StartReverseZonePhase) bZoneReversing = false;
        NewLateGameSafeZonePhase = (bZoneReversing && bEnableReverseZone) ? NewLateGameSafeZonePhase - 1 : NewLateGameSafeZonePhase + 1;
    }
    else 
        Originals::StartNewSafeZonePhase(GameMode, NewSafeZonePhase);
	
    if (bLategame)
    {
    	const float FixedInitialZoneSize = 5000.0f;
        if (GameMode->SafeZonePhase == 3)
            SafeZoneIndicator->Radius = FixedInitialZoneSize;

        if (GameMode->SafeZonePhase == 2 || GameMode->SafeZonePhase == 3)
        {
            if (SafeZoneIndicator)
            {
                SafeZoneIndicator->SafeZoneStartShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld());
                SafeZoneIndicator->SafeZoneFinishShrinkTime = UGameplayStatics::GetTimeSeconds(UWorld::GetWorld()) + 0.2;
            }

            UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"skipsafezone", nullptr);
        }
    }
}

void FortGameModeAthena::StartAircraftPhase(AFortGameModeAthena* GameMode, char a2)
{
	
	auto GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
	bLategame = GameMode->AlivePlayers.Num() < 30 ? true : false;
	static auto PlaylistName = GameState->CurrentPlaylistInfo.BasePlaylist->PlaylistName.ToString();
	std::transform(PlaylistName.begin(), PlaylistName.end(), PlaylistName.begin(), ::tolower);
	if (!PlaylistName.contains("showdownalt"))
		bLategame = false;

	if (bLategame)
	{
		GameState->CachedSafeZoneStartUp = ESafeZoneStartUp::StartsWithWarmUp;
		GameState->AirCraftBehavior = EAirCraftBehavior::FlyTowardFirstCircleCenter;
		Originals::StartAircraftPhase(GameMode, 0);
	} else
	{
		return Originals::StartAircraftPhase(GameMode, 0);
	}

	if (bLategame)
	{
		GameMode->bSafeZoneActive = true;
		((void* (*)(AFortGameModeAthena * GameMode, bool a))(InSDKUtils::GetImageBase() + 0xFB5580))(GameMode, false);
		StartNewSafeZonePhase(GameMode, -1);
		StartNewSafeZonePhase(GameMode, -1);
		
		GameState->DefaultParachuteDeployTraceForGroundDistance = 3500.0f;
		auto LocalAircraft = GameState->Aircrafts[0];

		const FVector ZoneCenterLocation = GameMode->SafeZoneLocations[3];
		FVector LocationToStartAircraft1 = ZoneCenterLocation;
		LocationToStartAircraft1.Z += 25000;
		
		FVector CurrentLocation = LocationToStartAircraft1;
		FRotator CurrentRotation = UKismetMathLibrary::FindLookAtRotation(ZoneCenterLocation, GameMode->SafeZoneLocations[4]);

		FVector ForwardVector = LocalAircraft->GetActorForwardVector();
		LocalAircraft->CameraInitialRotation = CurrentRotation;
		
		float Distance = 30000.0f;
		FVector LocationToStartAircraft = CurrentLocation - (ForwardVector * Distance);
		LocalAircraft->K2_TeleportTo(LocationToStartAircraft, CurrentRotation);
		FRotator CurrentRotationAfter = UKismetMathLibrary::FindLookAtRotation(LocalAircraft->K2_GetActorLocation(), ZoneCenterLocation);
		CurrentRotationAfter.Pitch = 0;
		
		float FlightSpeed = 2500.0f;
		LocalAircraft->K2_SetActorRotation(CurrentRotationAfter, true);
		FAircraftFlightInfo& FlightInfo = LocalAircraft->FlightInfo;
		FlightInfo.FlightSpeed = FlightSpeed;
		FlightInfo.FlightStartLocation = Quantize100(LocationToStartAircraft);
		FlightInfo.FlightStartRotation = CurrentRotationAfter;
		LocalAircraft->DropEndTime = LocalAircraft->GetDropEndTime() - (8 * 2 + 5);
		FlightInfo.TimeTillDropStart -= 3;
		LocalAircraft->DropStartTime = LocalAircraft->GetDropStartTime() - 3;
		FlightInfo.TimeTillDropEnd -= (8 * 2 + 5);
		CreateThread(0, 0, StartLGThread, 0, 0, 0);
	}
}

void FortGameModeAthena::OnAircraftExitedDropZone(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft)
{
	if (bLategame)
	{
		for (auto& Player : GameMode->AlivePlayers)
		{
			if (!Player) continue;
			if (Player->IsInAircraft())
				Player->ServerAttemptAircraftJump({});
		}
	}
	
	return Originals::OnAircraftExitedDropZone(GameMode, Aircraft);
}

EFortTeam FortGameModeAthena::PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller)
{	
    static uint8_t Next = 3;
    uint8_t Teams = Next;

    static uint8_t Players = 0;

    auto GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
    if (!GameState || !GameState->CurrentPlaylistInfo.BasePlaylist) return EFortTeam(0);
    
    Players++;

    if (Players >= GameState->CurrentPlaylistInfo.BasePlaylist->MaxSquadSize)
    {
        Next++;
        Players = 0;
    }

    return EFortTeam(Teams);
}

void FortGameModeAthena::Setup()
{
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, EDetourType::VFT>(0xFC, ReadyToStartMatch, &Originals::ReadyToStartMatch);
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, EDetourType::VFT>(0xC3, SpawnDefaultPawnFor);
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, EDetourType::VFT>(0xC9, HandleStartingNewPlayer, &Originals::HandleStartingNewPlayer);
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xfb8070, StartAircraftPhase, &Originals::StartAircraftPhase);
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xFA70C0, OnAircraftExitedDropZone, &Originals::OnAircraftExitedDropZone);
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xfb9830, StartNewSafeZonePhase, &Originals::StartNewSafeZonePhase);
	//if (bProd)
	//	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0xfa9b20, PickTeam);
}