#include "Player/FortPlayerPawn.h"
#include <Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h>
#include <Items/FortInventory.h>
#include <FortKismetLibrary.h>

#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"

void FortPlayerPawn::ServerHandlePickup(AFortPlayerPawnAthena* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
{
	if (!Pawn || !Pickup || Pickup->bPickedUp)
		return;

	Pawn->IncomingPickups.Add(Pickup);
	
	auto& LocData = Pickup->PickupLocationData;
	LocData.PickupTarget = Pawn;
	LocData.FlyTime = 0.4f;
	LocData.ItemOwner = Pawn;
	Pickup->OnRep_PickupLocationData();
	
	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void FortPlayerPawn::ServerSendZiplineState(AFortPlayerPawn* Pawn, FZiplinePawnState& InState)
{
	if (Pawn)
	{
		static void (*OnRep_ZiplineState)(void* FortPawn) = decltype(OnRep_ZiplineState)(InSDKUtils::GetImageBase() + 0x16A2800);
		OnRep_ZiplineState(Pawn);
		
		Pawn->ZiplineState = InState;
		
		if (InState.bJumped)
		{
			auto Velocity = Pawn->CharacterMovement->Velocity;
			auto VelocityX = Velocity.X * -0.5f;
			auto VelocityY = Velocity.Y * -0.5f;
			Pawn->LaunchCharacterJump({ VelocityX >= -750 ? fminf(VelocityX, 750) : -750, VelocityY >= -750 ? fminf(VelocityY, 750) : -750, 1200 }, false, false, true, true);
		}
	}
}

void FortPlayerPawn::TryToAutoPickup(AFortPlayerPawn* Pawn, AFortPickup* Pickup)
{
	if (!Pawn || !Pawn->GetController()) return;
	if (!Pickup || !Pickup->PrimaryPickupItemEntry.ItemDefinition) return;
	
	auto MaxStack = Pickup->PrimaryPickupItemEntry.ItemDefinition->MaxStackSize;
	auto ItemEntry = FortInventory::FindItem<FFortItemEntry>(((AFortPlayerControllerAthena*)Pawn->Controller), (UFortItemDefinition*)Pickup->PrimaryPickupItemEntry.ItemDefinition);
	
	if (Pickup && Pickup->PawnWhoDroppedPickup != Pawn)
	{
		if ((!ItemEntry && !FortInventory::IsPrimaryQuickBar(Pickup->PrimaryPickupItemEntry.ItemDefinition)) || (ItemEntry && ItemEntry->Count < MaxStack))
			Pawn->ServerHandlePickup(Pickup, 0.4f, FVector(), true);
	}
}

void FortPlayerPawn::Setup()
{
	UDetoursLibrary::InitializeDetour<AFortPlayerPawnAthena, VFT>(0x1BA, ServerHandlePickup);
	UDetoursLibrary::InitializeDetour<AFortPlayerPawnAthena, VFT>(0x1C5, ServerSendZiplineState);
	UDetoursLibrary::InitializeDetour<AFortPlayerPawn, MinHook>(0x16C1480, TryToAutoPickup);
}