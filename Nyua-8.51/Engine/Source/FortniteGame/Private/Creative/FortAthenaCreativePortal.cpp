#include "Creative/FortAthenaCreativePortal.h"
#include <Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h>

#include "Creative/ImproperIslandSaving.h"

void FortAthenaCreativePortal::TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Portal, FFrame& Stack, void* Ret)
{
    AFortPlayerPawnAthena* PlayerPawn;
    Stack.StepCompiledIn(&PlayerPawn);
    
	if (!PlayerPawn) return;

    auto Location = Portal->GetLinkedVolume()->K2_GetActorLocation();
    Location.Z = 15000.f;

    PlayerPawn->K2_TeleportTo(Location, FRotator());
    PlayerPawn->BeginSkydiving(false);

    

    Builder::LoadIsland((AFortPlayerControllerAthena*)PlayerPawn->GetController());

    return Originals::TeleportPlayerToLinkedVolume(Portal, Stack, Ret);
}

void FortAthenaCreativePortal::Setup()
{
    UDetoursLibrary::InitializeDetour<AFortAthenaCreativePortal, Exec>(StaticFindObject<UFunction>("/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume"), TeleportPlayerToLinkedVolume, &Originals::TeleportPlayerToLinkedVolume);
}