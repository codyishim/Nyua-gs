#pragma once

#include "CoreGlobals.h"

class FortQuestManager
{
private:
    class Originals
    {
    public:
        static inline void (*SendComplexCustomStatEvent)(UFortQuestManager*, UObject*, FGameplayTagContainer&, FGameplayTagContainer&, bool*, bool*, int32);
    };
public:
    static void SendStatEventWithTags(UFortQuestManager* QuestManager,
        EFortQuestObjectiveStatEvent Type,
        UObject* TargetObj,
        FGameplayTagContainer &TargetTags,
        FGameplayTagContainer &SourceTags,
        FGameplayTagContainer &ContextTags,
        int32 Count);

    static void SendComplexCustomStatEvent(
     UFortQuestManager* QuestManager,
     UObject* TargetObj,
     FGameplayTagContainer &SourceTags,
     FGameplayTagContainer &TargetTags,
     bool *QuestActive, bool *QuestCompleted, int32 Count);
    
    static void Setup();
};