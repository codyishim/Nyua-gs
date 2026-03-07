#include "Components/FortControllerComponent_Interaction.h"

#include "Building/BuildingContainer.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Quests/FortQuestManager.h"

void FortControllerComponent_Interaction::ServerAttemptInteract(UFortControllerComponent_Interaction* InteractionComp, FFrame& Stack, void* Ret)
{
    AActor* ReceivingActor = *(AActor**)Stack.Locals;

    if (!ReceivingActor)
        return Originals::ServerAttemptInteract(InteractionComp, Stack, Ret); 
    
    AFortPlayerController* Controller = Cast<AFortPlayerController>(InteractionComp->GetOwner());

    if (!Controller)
        return Originals::ServerAttemptInteract(InteractionComp, Stack, Ret);

    AFortPlayerPawn* MyFortPawn = Controller->MyFortPawn;

    if (!MyFortPawn)
        return Originals::ServerAttemptInteract(InteractionComp, Stack, Ret);
    
    if (ABuildingContainer* BuildingContainer = Cast<ABuildingContainer>(ReceivingActor))
    {
        if (BuildingContainer->bAlreadySearched)
            return Originals::ServerAttemptInteract(InteractionComp, Stack, Ret);

        BuildingContainer::SpawnLoot(BuildingContainer, MyFortPawn);

        BuildingContainer->SearchBounceData.SearchAnimationCount++;
        BuildingContainer->BounceContainer();
        BuildingContainer->bAlreadySearched = true;
        BuildingContainer->OnRep_bAlreadySearched();
    }

    if (ReceivingActor->IsA(ABuildingActor::StaticClass()))
    {
        ABuildingActor* BuildingActor = (ABuildingActor*)ReceivingActor;
        if (!BuildingActor) return;
        FGameplayTagContainer SourceTags;
        FGameplayTagContainer TargetTags;
        FGameplayTagContainer ContextTags;
        UFortQuestManager* QuestManager = Controller->GetQuestManager(ESubGame::Athena);
        QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
        TargetTags.AppendTags(BuildingActor->StaticGameplayTags);
        
        FortQuestManager::SendStatEventWithTags(QuestManager, EFortQuestObjectiveStatEvent::Interact, nullptr, TargetTags, SourceTags, ContextTags, 1);
    }
    
    return Originals::ServerAttemptInteract(InteractionComp, Stack, Ret);
}

void FortControllerComponent_Interaction::Setup()
{
    UFunction* ServerAttemptInteractFn = StaticFindObject<UFunction>("/Script/FortniteGame.FortControllerComponent_Interaction:ServerAttemptInteract");
    UDetoursLibrary::InitializeDetour<UFortControllerComponent_Interaction, Exec>(ServerAttemptInteractFn, ServerAttemptInteract, &Originals::ServerAttemptInteract);
}
