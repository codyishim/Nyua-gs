#pragma once

#include "CoreGlobals.h"

class AbilitySystemComponent
{
public:
    class Originals
    {
    public:
        static inline bool (*InternalTryActivateAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData);
        static inline void (*GiveAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle&, FGameplayAbilitySpec*);
        static inline void (*NewGameplayAbilitySpec)(FGameplayAbilitySpec*, UGameplayAbility*, int32, int32, UObject*);
    };
public:
    static void GivePlayerAbilities(UAbilitySystemComponent* AbilitySystemComponent);
    
    static void ConsumeAllReplicatedData(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey);
    static FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle);
    
    static void InternalServerTryActiveAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData);

    static void Setup();
};