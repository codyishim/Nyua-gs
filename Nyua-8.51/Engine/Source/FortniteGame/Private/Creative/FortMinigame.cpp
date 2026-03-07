#include "Creative/FortMinigame.h"
#include <Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h>
#include <Vendor/Memcury.h>
#include <thread>

#include "Creative/ImproperIslandSaving.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"

void FortMinigame::ChangeMinigameState(AFortMinigame* Minigame, EFortMinigameState State)
{
	UE_LOG(LogServer, Log, TEXT("AFortMinigame::ChangeMinigameState %d"), State);

	TArray<AFortPlayerState *> Players;
	Minigame->GetParticipatingPlayers(&Players);
	for (auto& It : Players)
	{
		auto Player = Cast<AFortPlayerStateAthena>(It);
		if (!Player) 
			continue;
		auto PC = Cast<AFortPlayerControllerAthena>(Player->GetOwner());
		if (!PC) 
			continue;
		Builder::SaveIsland(PC);
	}
	
	switch (State)
	{
	case EFortMinigameState::Transitioning:
		{
			for (int i = 0; i < Players.Num(); i++)
			{
				auto Player = Cast<AFortPlayerStateAthena>(Players[i]);
				auto PC = Cast<AFortPlayerControllerAthena>(Player->GetOwner());
				if (!PC) 
					continue;
				auto Pawn = PC->MyFortPawn;
				if (!Pawn)
					continue;

				FVector Loc{};
				FRotator Rot = Pawn->K2_GetActorRotation();
				Minigame->DetermineSpawnLocation(Player, &Loc, &Rot, nullptr);

				FortInventory::ClearInventory(PC);
				
				if (Minigame->NumTeams == 0)
					PC->ServerSetTeam(i + 3);
				else
					PC->ServerSetTeam((i % Minigame->NumTeams) + 3);
				
				PC->ClientStartRespawnPreparation(Loc, Rot, 0, Minigame->MinigameStartCameraBehavior, UKismetTextLibrary::Conv_StringToText(L"Starting Game "));
				Player->RespawnData.RespawnLocation = Loc;
				Player->RespawnData.RespawnRotation = Rot;
				Player->RespawnData.bClientIsReady = true;
				Player->RespawnData.bRespawnDataAvailable = true;
				Player->RespawnData.bServerIsReady = true;
				Pawn->K2_TeleportTo(Loc, Rot);
				PC->UnPossess();
				Pawn->K2_DestroyActor();
				Minigame->OnPlayerPawnPossessedDuringTransition(Pawn);
			}
	
			std::thread([Minigame, Players]()
			{
				bool AllRespawned = false;
				while (!AllRespawned)
				{
					AllRespawned = true;

					for (int i = 0; i < Players.Num(); i++)
					{
						auto Player = Cast<AFortPlayerStateAthena>(Players[i]);
						auto PC = Cast<AFortPlayerControllerAthena>(Player->GetOwner());
						if (!PC) continue;

						auto Pawn = PC->MyFortPawn;
						if (!Pawn)
						{
							AllRespawned = false;
							break;
						}

						if (Pawn->bIsRespawning)
						{
							AllRespawned = false;
							break;
						}
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
				Minigame->AdvanceState();
			}).detach();
		}
		break;
	case EFortMinigameState::WaitingForCameras:
		{
			for (int i = 0; i < Players.Num(); i++)
			{
				auto Player = Cast<AFortPlayerStateAthena>(Players[i]);
				auto PC = Cast<AFortPlayerControllerAthena>(Player->GetOwner());
				if (!PC) continue;
				auto Pawn = PC->MyFortPawn;
				if (!Pawn) continue;
				
				Minigame->OnClientFinishTeleportingForMinigame(Pawn);
			}
			std::thread([Minigame, State]()
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					Originals::ChangeMinigameState(Minigame, State);
				}).detach();
		}
		break;
	case EFortMinigameState::PostGameReset:
		{
			Originals::ChangeMinigameState(Minigame, State);
			for (int i = 0; i < Players.Num(); i++)
			{
				auto Player = Cast<AFortPlayerStateAthena>(Players[i]);
				auto PC = Cast<AFortPlayerControllerAthena>(Player->GetOwner());
				auto Pawn = PC->MyFortPawn;
				if (!Pawn) continue;

				FVector Loc{};
				FRotator Rot = Pawn->K2_GetActorRotation();
				Minigame->DetermineSpawnLocation(Player, &Loc, &Rot, nullptr);
			
				FortInventory::ClearInventory(PC);
				
				Player->RespawnData.bClientIsReady = true;
				Player->RespawnData.bRespawnDataAvailable = true;
				Player->RespawnData.bServerIsReady = true;
				PC->ClientStartRespawnPreparation(Loc, Rot, 0, Minigame->MinigameEndCameraBehavior, Minigame->ClientMinigameEndedText);
				Pawn->K2_TeleportTo(Loc, Rot);
				Pawn->K2_DestroyActor();
				Minigame->OnPlayerPawnPossessedDuringTransition(Pawn);
			}
			Minigame->CurrentState = EFortMinigameState::PreGame;
			Minigame->OnRep_CurrentState();
		}
	case EFortMinigameState::PostGameEnd:
		std::thread([Minigame](){
			while ((int)Minigame->CurrentState != 9)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			Minigame->CurrentState = EFortMinigameState::PostGameEnd;
			Minigame->OnRep_CurrentState();
		}).detach();
		break;
	}

	if (State != EFortMinigameState::WaitingForCameras && State != EFortMinigameState::PostGameReset)
		return Originals::ChangeMinigameState(Minigame, State);
}

void FortMinigame::Setup()
{
	UDetoursLibrary::InitializeDetour<AFortGameModeAthena, MinHook>(0x14A6170, ChangeMinigameState, &Originals::ChangeMinigameState);
}
