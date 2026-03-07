#include "Quests/FortQuestManager.h"
#include <map>

#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"

static void ProgressQuest(AFortPlayerControllerAthena* PlayerController, UFortQuestManager* QuestManager, UFortQuestItem* QuestItem, UFortQuestItemDefinition* QuestDef, FFortMcpQuestObjectiveInfo Objective, int32 IncrementCount, TArray<FFortQuestObjectiveCompletion>& OutObjectiveCompletions)
{
	static std::unordered_map<AFortPlayerControllerAthena*, std::vector<FFortMcpQuestObjectiveInfo>> CompletedObjectives;
    
	auto CurrentCount = QuestDef->GetPartialObjectiveCompletionCount();
	if (CurrentCount == -1) CurrentCount = 0;
	int32 NewCount = CurrentCount + IncrementCount;
	QuestDef->ObjectiveCompletionCount = NewCount;
    
	bool objectiveCompleted = NewCount >= Objective.Count;
	bool allObjectivesCompleted = objectiveCompleted && QuestDef->Objectives.Num() == 1;
    
	if (objectiveCompleted && QuestDef->Objectives.Num() > 1) {
		bool alreadyExists = false;
		for (auto& CompObj : CompletedObjectives[PlayerController]) {
			if (CompObj.BackendName == Objective.BackendName) {
				alreadyExists = true;
				break;
			}
		}
		if (!alreadyExists) CompletedObjectives[PlayerController].push_back(Objective);
        
		int32 CompletionCount = 0;
		for (auto& QuestObj : QuestDef->Objectives) {
			for (auto& CompObj : CompletedObjectives[PlayerController]) {
				if (QuestObj.BackendName == CompObj.BackendName) {
					CompletionCount++;
					break;
				}
			}
		}
		allObjectivesCompleted = (CompletionCount == QuestDef->Objectives.Num());
	}
    
	auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
	if (!PlayerState) return;

	for (const auto& TeamMember : PlayerState->PlayerTeam->TeamMembers) {
		auto TeamController = (AFortPlayerControllerAthena*)TeamMember;
        
		bool foundExisting = false;
		for (auto& UpdatedStat : TeamController->UpdatedObjectiveStats) {
			if (UpdatedStat.BackendName == Objective.BackendName) {
				UpdatedStat.Quest = QuestDef;
				UpdatedStat.StatDelta = IncrementCount;  
				UpdatedStat.StatValue = NewCount;  
				TeamController->OnRep_UpdatedObjectiveStats();
				foundExisting = true;
				TeamController->Client_DisplayQuestUpdate_Self(UpdatedStat);
				break;
			}
		}

		if (!foundExisting)
		{
			FFortUpdatedObjectiveStat NewStat{};
			NewStat.BackendName = Objective.BackendName;
			NewStat.Quest = QuestDef;
			NewStat.CurrentStage = 0; 
			NewStat.StatDelta = IncrementCount;  
			NewStat.StatValue = NewCount; 
			TeamController->UpdatedObjectiveStats.Add(NewStat);
			TeamController->OnRep_UpdatedObjectiveStats();
			TeamController->Client_DisplayQuestUpdate_Self(NewStat);
		}

		if (TeamController == PlayerController) {
			QuestManager->SelfCompletedUpdatedQuest(TeamController, QuestDef, Objective.BackendName, NewCount, IncrementCount, PlayerState, objectiveCompleted, allObjectivesCompleted);
		} else {
			QuestManager->AssistedPlayerUpdatedQuest(TeamController, QuestDef, Objective.BackendName, NewCount, IncrementCount, PlayerState, objectiveCompleted, allObjectivesCompleted);
		}
	}

	FFortQuestObjectiveCompletion Completion;
	Completion.StatName = UKismetStringLibrary::Conv_NameToString(Objective.BackendName);
	Completion.Count = NewCount;
	OutObjectiveCompletions.Add(Completion);

	if (allObjectivesCompleted) {
		CompletedObjectives[PlayerController].clear();
	}
}

void FortQuestManager::SendStatEventWithTags(UFortQuestManager* QuestManager, EFortQuestObjectiveStatEvent Type, UObject* TargetObj,
                                             FGameplayTagContainer& TargetTags, FGameplayTagContainer& SourceTags, FGameplayTagContainer& ContextTags, int32 Count)
{
    FGameplayTagContainer PSourceTags;
    FGameplayTagContainer PContextTags;
    QuestManager->GetSourceAndContextTags(&PSourceTags, &ContextTags);
    
    UFortPlaylistAthena* Playlist = nullptr;
    auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
    if (GameState)
        Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;

    ContextTags.AppendTags(PContextTags);
    ContextTags.AppendTags(Playlist->GameplayTagContainer);
    SourceTags.AppendTags(PSourceTags);

	TArray<FFortQuestObjectiveCompletion> AllObjectiveCompletions;
	AFortPlayerControllerAthena* PlayerController = nullptr;

    auto& CurrentQuests = QuestManager->CurrentQuests;
    for (auto& Quest : CurrentQuests)
    {
        if (!Quest || Quest->HasCompletedQuest()) continue;

        auto QuestDef = Quest->GetQuestDefinitionBP();
        if (!QuestDef) continue;
        if (QuestManager->HasCompletedQuest(QuestDef)) continue;

        auto Controller = (AFortPlayerControllerAthena*)Quest->GetOwningController();
        if (!Controller) continue;

		if (!PlayerController) PlayerController = Controller;

        auto& Objectives = QuestDef->Objectives;
        for (auto& Objective : Objectives)
        {
            if (QuestManager->HasCompletedObjectiveWithName(QuestDef, Objective.BackendName) ||
                QuestManager->HasCompletedObjective(QuestDef, Objective.ObjectiveStatHandle) ||
                Quest->HasCompletedObjectiveWithName(Objective.BackendName) ||
                Quest->HasCompletedObjective(Objective.ObjectiveStatHandle)) continue;

            auto StatTable = Objective.ObjectiveStatHandle.DataTable;
            auto& StatRowName = Objective.ObjectiveStatHandle.RowName;
            if (StatTable)
            {
                auto& RowMap = StatTable->RowMap;

                for (const auto& [RowName, RowPtr] : RowMap)
                {
                    if (RowName == StatRowName)
                    {
                        auto Row = (FFortQuestObjectiveStatTableRow*)RowPtr;
                        if (Row->Type != Type) continue;
						if (!TargetTags.HasAll(Row->TargetTagContainer)) continue;
                  //      if (!SourceTags.HasAll(Row->SourceTagContainer)) continue;
				//		if (!ContextTags.HasAll(Row->ContextTagContainer)) continue;

                        ProgressQuest(Controller, QuestManager, Quest, QuestDef, Objective, Count, AllObjectiveCompletions);
                    }
                }
            }
        }
    }

	if (AllObjectiveCompletions.Num() > 0)
		PlayerController->AthenaProfile->UpdateQuests(AllObjectiveCompletions, {});
}

void FortQuestManager::SendComplexCustomStatEvent(UFortQuestManager* QuestManager, UObject* TargetObj, FGameplayTagContainer& SourceTags, FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count)
{
	FGameplayTagContainer ContextTags;
	QuestManager->GetSourceAndContextTags(nullptr, &ContextTags);
    
	SendStatEventWithTags(QuestManager, EFortQuestObjectiveStatEvent::ComplexCustom, TargetObj, TargetTags, SourceTags,
						  ContextTags, Count);
	return Originals::SendComplexCustomStatEvent(QuestManager, TargetObj, SourceTags, TargetTags, QuestActive, QuestCompleted,
										Count);
}

void FortQuestManager::Setup()
{
	auto ImageBase = InSDKUtils::GetImageBase();
	std::vector<uint64_t> Addrs = {
		ImageBase + 0x11F01F5,
		ImageBase + 0x11F023A,
		ImageBase + 0xFE4DCE,
		ImageBase + 0x10593A5,
		ImageBase + 0x10405DE,
		ImageBase + 0x1030C6E,
		ImageBase + 0x14D0165,
		ImageBase + 0x174123F,
		ImageBase + 0x174126F,
	};

	for (auto Addr : Addrs)
	{
		void* PatchAddr = (void*)Addr;
		UDetoursLibrary::Rel32Swap(&PatchAddr, SendStatEventWithTags);
	}

	UDetoursLibrary::InitializeDetour<UFortQuestManager, MinHook>(0x17858F0, SendComplexCustomStatEvent, &Originals::SendComplexCustomStatEvent);
}