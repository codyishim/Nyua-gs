#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Public/AbilitySystemComponent.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"

void AbilitySystemComponent::GivePlayerAbilities(UAbilitySystemComponent* AbilitySystemComponent)
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogAbilitySystem, Error, "GivePlayerAbilities: Invalid AbilitySystemComponent provided!");
        return;
    }
    
    static UFortAbilitySet* AbilitySet = StaticLoadObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
    
    for (auto& GameplayAbility : AbilitySet->GameplayAbilities)
    {
        FGameplayAbilitySpec Spec{};
        Originals::NewGameplayAbilitySpec(&Spec, (UGameplayAbility*)GameplayAbility->DefaultObject, 1, -1, nullptr);

        if (!Spec.Ability)
        {
            UE_LOG(LogAbilitySystem, Error, "GivePlayerAbilities: Spec doesnt have an ability?");
            continue;
        }
        
        Originals::GiveAbility(AbilitySystemComponent, Spec.Handle, &Spec);
    }

    auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
    
    for (auto& Modifier : GameState->CurrentPlaylistInfo.BasePlaylist->ModifierList)
    {
        if (!Modifier.Get()) continue;
        for (auto& PersistentAbilitySet : Modifier.Get()->PersistentAbilitySets)
        {
            for (auto& PlaylistAbilitySet : PersistentAbilitySet.AbilitySets)
            {
                if (!PlaylistAbilitySet.Get()) continue;
                
                for (auto& GameplayAbility : PlaylistAbilitySet->GameplayAbilities)
                {
                    FGameplayAbilitySpec Spec{};
                    Originals::NewGameplayAbilitySpec(&Spec, (UGameplayAbility*)GameplayAbility->DefaultObject, 1, -1, nullptr);
        
                    Originals::GiveAbility(AbilitySystemComponent, Spec.Handle, &Spec);
                }
            }
        }
    }
}

// Maybe ill add this later idk
void AbilitySystemComponent::ConsumeAllReplicatedData(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey)
{
    
}

FGameplayAbilitySpec* AbilitySystemComponent::FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle)
{
    for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->ActivatableAbilities.Items)
    {
        if (Spec.Handle == Handle)
        {
            return &Spec;
        }
    }

    return nullptr;
}

void AbilitySystemComponent::InternalServerTryActiveAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilitySystemComponent, Handle);
    if (!Spec)
    {
        UE_LOG(LogAbilitySystem, Log, "InternalServerTryActiveAbility. Rejecting ClientActivation of ability with invalid SpecHandle!");
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }
    
    ConsumeAllReplicatedData(AbilitySystemComponent, Handle, PredictionKey);

    const UGameplayAbility* AbilityToActivate = Spec->Ability;

    if (!AbilityToActivate)
    {
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (Originals::InternalTryActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        
    }
    else
    {
        UE_LOG(LogAbilitySystem, Log, "InternalServerTryActiveAbility. Rejecting ClientActivation of %s. InternalTryActivateAbility failed: %s", Spec->Ability->GetName().c_str());
        AbilitySystemComponent->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;

        AbilitySystemComponent->ActivatableAbilities.MarkItemDirty(*Spec);
    }
}

void AbilitySystemComponent::Setup()
{
    Originals::GiveAbility = decltype(Originals::GiveAbility)(InSDKUtils::GetImageBase() + 0x843df0);
    Originals::InternalTryActivateAbility = decltype(Originals::InternalTryActivateAbility)(InSDKUtils::GetImageBase() + 0x8455d0);
    Originals::NewGameplayAbilitySpec = decltype(Originals::NewGameplayAbilitySpec)(InSDKUtils::GetImageBase() + 0x868290);
    
    UDetoursLibrary::InitializeDetour<UFortAbilitySystemComponentAthena, VFT>(0xF4, InternalServerTryActiveAbility);
}
