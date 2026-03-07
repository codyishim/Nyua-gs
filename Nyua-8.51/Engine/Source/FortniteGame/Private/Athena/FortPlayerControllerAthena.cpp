#include "Athena/FortPlayerControllerAthena.h"

#include <algorithm>

#include "FortKismetLibrary.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include "Items/FortInventory.h"
#include <thread>

#include "nlohmann.hpp"
#include "Quests/FortQuestManager.h"

void FortPlayerControllerAthena::ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* P)
{
    PlayerController->AcknowledgedPawn = P;

    auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

    if (!PlayerState)
        return;

    auto Pawn = Cast<AFortPlayerPawn>(P);

    if (!Pawn)
        return;
    
    static void (*ApplyCharacterCustomization)(AFortPlayerState*, AFortPlayerPawn*) = decltype(ApplyCharacterCustomization)(InSDKUtils::GetImageBase() + 0x175fe30);

    ApplyCharacterCustomization(PlayerState, Pawn);

    if (!PlayerController->bHasInitializedWorldInventory)
    {
        PlayerController->bHasInitializedWorldInventory = true;

        if (auto NewItem = FortInventory::GiveItem(PlayerController, PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition))
        {
            PlayerController->ServerExecuteInventoryItem(NewItem->ItemEntry.ItemGuid);   
        }
    }
}

std::vector<FItemAndCount> GetLategameLoadout()
{
    static std::vector<FItemAndCount> Shotguns{
        FItemAndCount(1, {},    StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03")), // pump 
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03")), // pump 
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03")), // pump 
    };

    static std::vector<FItemAndCount> AssaultRifles{
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03")), // scar epic
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03")), // scar gold 
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_VR_Ore_T03.WID_Assault_Suppressed_Athena_VR_Ore_T03")), // suppresed epic 
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Suppressed_Athena_SR_Ore_T03.WID_Assault_Suppressed_Athena_SR_Ore_T03")), // suppresed gold
    };

    static std::vector<FItemAndCount> Snipers{
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_SR_Ore_T03.WID_Sniper_Heavy_Athena_SR_Ore_T03")), // heavy sniper
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_Heavy_Athena_VR_Ore_T03.WID_Sniper_Heavy_Athena_VR_Ore_T03")), // heavy sniper
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/LTM/WID_Sniper_NoScope_Athena_R_Ore_T03.WID_Sniper_NoScope_Athena_R_Ore_T03")), // hunting
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03")), // bolt sniper
        FItemAndCount(1, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03")), // bolt sniper
    };

    static std::vector<FItemAndCount> Heals{
        FItemAndCount(3, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/Shields/Athena_Shields.Athena_Shields")), // big pots
        FItemAndCount(6, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall")), // minis
        FItemAndCount(6, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ShockwaveGrenade/Athena_ShockGrenade.Athena_ShockGrenade")), // shockwave
        FItemAndCount(2, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff")), // Slurp Juice
        FItemAndCount(4, {}, StaticFindObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Consumables/ChillBronco/Athena_ChillBronco.Athena_ChillBronco")), // Slurp Juice
     };

    int f = rand() % Heals.size();
    int s;
    do {
        s = rand() % Heals.size();
    } while (s == f && Heals.size() > 1);

    std::vector<FItemAndCount> Loadout;
    do {
        int f = rand() % Heals.size();
        int s;
        do {
            s = rand() % Heals.size();
        } while (s == f && Heals.size() > 1);
        
        Loadout = {
            Shotguns[rand() % Shotguns.size()],
            AssaultRifles[rand() % AssaultRifles.size()],
            Snipers[rand() % Snipers.size()],
            Heals[f],
            Heals[s]
        };
    } while (std::any_of(Loadout.begin(), Loadout.end(), [](const FItemAndCount& Item) { return !Item.Item; }));

    return Loadout;
}

void FortPlayerControllerAthena::ServerAttemptAircraftJump(AFortPlayerControllerAthena* PlayerController, FRotator& Rotation)
{
    auto GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
    auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);

    if (GameMode)
    {
        AFortInventory* WorldInventory = PlayerController->WorldInventory;

        if (WorldInventory)
        {
            FFortItemList& Inventory = WorldInventory->Inventory;
            for (int i = Inventory.ItemInstances.Num() - 1; i >= 0; --i)
            {
                UFortWorldItem* ItemInstance = Inventory.ItemInstances[i];
                if (!ItemInstance || !ItemInstance->CanBeDropped()) continue;

                FortInventory::RemoveItem(PlayerController, ItemInstance->ItemEntry.ItemGuid, ItemInstance->ItemEntry.Count);
            }
        }
        
        GameMode->RestartPlayer(PlayerController);
        PlayerController->ClientSetRotation(Rotation, false);
        
        if (bLategame)
        {
            auto SpawnLocation = GameState->Aircrafts[0]->K2_GetActorLocation();
            SpawnLocation.Z -= UKismetMathLibrary::RandomIntegerInRange(370, 700);
            SpawnLocation.X += UKismetMathLibrary::RandomIntegerInRange(200, 750);
            SpawnLocation.X -= UKismetMathLibrary::RandomIntegerInRange(200, 400);

            if (PlayerController->MyFortPawn) PlayerController->MyFortPawn->K2_TeleportTo(SpawnLocation, {});

            GameState->GamePhase = EAthenaGamePhase::SafeZones;
            GameState->OnRep_GamePhase(EAthenaGamePhase::Count);
            
            auto Loadout = GetLategameLoadout();
            for (auto Item : Loadout)
            {
                if (!Item.Item) continue;
                FortInventory::GiveItem(PlayerController, Item.Item, Item.Count, Item.Item->IsA(UFortWeaponItemDefinition::StaticClass()) ?
                    FortKismetLibrary::GetClipSize((UFortWeaponItemDefinition*)Item.Item) : 0, 1);
            }

            static auto Assault = StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
            static auto Shotgun = StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
            static auto Submachine = StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
            static auto Rocket = StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
            static auto Sniper = StaticFindObject<UFortAmmoItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");

            FortInventory::GiveItem(PlayerController, Assault, 120, 120, 1);
            FortInventory::GiveItem(PlayerController, Shotgun, 30, 30, 1);
            FortInventory::GiveItem(PlayerController, Submachine, 180, 180, 1);
            FortInventory::GiveItem(PlayerController, Rocket, 6, 6, 1);
            FortInventory::GiveItem(PlayerController, Sniper, 18, 18, 1);

            FortInventory::GiveItem(PlayerController, UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Wood), 500, 500, 1);
            FortInventory::GiveItem(PlayerController, UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Stone), 500, 500, 1);
            FortInventory::GiveItem(PlayerController, UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Metal), 500, 500, 1);

            if (PlayerController->MyFortPawn) PlayerController->MyFortPawn->SetShield(100);
        }

        if (PlayerController->MyFortPawn) PlayerController->MyFortPawn->BeginSkydiving(true);
    }
}

void FortPlayerControllerAthena::ServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
{
    if (!PlayerController->MyFortPawn)
        return;
    
    FFortItemEntry* ItemEntry = FortInventory::FindItem<FFortItemEntry>(PlayerController, ItemGuid);

    if (!ItemEntry)
        return;

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry->ItemDefinition))
    {
        PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemEntry->ItemGuid);
    }
}

void FortPlayerControllerAthena::ServerGiveCreativeItem(AFortPlayerControllerAthena* PlayerController, FFortItemEntry* CreativeItemEntry)
{
    auto ItemDef = CreativeItemEntry->ItemDefinition;
    if (!ItemDef || !PlayerController)
        return;

    TArray<FFortItemEntry>& ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;
            
    FFortItemEntry* ItemEntry = nullptr;
    for (auto& Item : ReplicatedEntries) 
    {
        auto ItemEntryDef = Item.ItemDefinition;
        if (!ItemEntryDef)
            continue;
            
        if (ItemEntryDef->IsA(UFortResourceItemDefinition::StaticClass()) || 
            ItemEntryDef->IsA(UFortAmmoItemDefinition::StaticClass())) 
        {
            if (ItemEntryDef == ItemDef)
            {
                ItemEntry = &Item;
                break;
            }
        }
    }

    if (ItemEntry)
    {
        ItemEntry->Count = ItemEntry->Count + CreativeItemEntry->Count == 0 ? 1 : CreativeItemEntry->Count;
        FortInventory::ReplaceEntry(PlayerController, *ItemEntry);
        return;
    }

    int32 LoadedAmmo = 0;
    if (ItemDef->IsA(UFortWeaponItemDefinition::StaticClass()))
        LoadedAmmo = FortKismetLibrary::GetClipSize((UFortWeaponItemDefinition*)ItemDef);
    else 
        LoadedAmmo = CreativeItemEntry->LoadedAmmo == 0 ? 1 : CreativeItemEntry->LoadedAmmo;

    FortInventory::GiveItem(PlayerController, ItemDef, CreativeItemEntry->Count == 0 ? 1 : CreativeItemEntry->Count, LoadedAmmo, 1);
}

void ApplySiphon(AFortPawn* Pawn, int SiphonAmount, int MaxHealth, int MaxShield)
{
    if (!Pawn) return;

    int Health = Pawn->GetHealth();
    int Shield = Pawn->GetShield();

    if (Health > MaxHealth) Health = MaxHealth;
    if (Shield > MaxShield) Shield = MaxShield;

    if (Health == MaxHealth)
    {
        if (Shield + SiphonAmount > MaxShield)
            Shield = MaxShield;
        else
            Shield += SiphonAmount;
    }
    else if (Health + SiphonAmount > MaxHealth)
    {
        int Overflow = (Health + SiphonAmount) - MaxHealth;
        Health = MaxHealth;

        if (Shield + Overflow > MaxShield)
            Shield = MaxShield;
        else
            Shield += Overflow;
    }
    else
    {
        Health += SiphonAmount;
    }

    Pawn->SetHealth(Health);
    Pawn->SetShield(Shield);
}

void FortPlayerControllerAthena::ClientOnPawnDied(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport& DeathReport)
{
    auto GameMode = Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
    auto GameState = Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
    auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
    auto KillerPlayerState = (AFortPlayerStateAthena*)DeathReport.KillerPlayerState;
    auto KillerPawn = DeathReport.KillerPawn;
    auto VictimPawn = PlayerController->MyFortPawn;
    bool bRespawningAllowed = GameState && PlayerState ? GameState->IsRespawningAllowed(PlayerState) : false;
    if (bRespawningAllowed) return Originals::ClientOnPawnDied(PlayerController, DeathReport);

    FVector DeathLocation = VictimPawn ? VictimPawn->K2_GetActorLocation() : FVector(0,0,0);
    if (!KillerPlayerState && VictimPawn)
        KillerPlayerState = (AFortPlayerStateAthena*)((AFortPlayerControllerAthena*)VictimPawn->GetController())->PlayerState;
    FAthenaRewardResult& RewardResult = PlayerController->MatchReport->EndOfMatchResults;
    FAthenaMatchStats& MatchStats = PlayerController->MatchReport->MatchStats;
    FAthenaMatchTeamStats& TeamStats = PlayerController->MatchReport->TeamStats;

    auto DeathTags = DeathReport.Tags;
    EDeathCause DeathCause = PlayerState->ToDeathCause(DeathTags, false);

    PlayerState->PawnDeathLocation = DeathLocation;
    PlayerState->DeathInfo.bDBNO = VictimPawn->IsDBNO();
    PlayerState->DeathInfo.DeathCause = DeathCause;
    PlayerState->DeathInfo.DeathTags = DeathTags;
    PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState;
    PlayerState->DeathInfo.bInitialized = true;
    PlayerState->DeathInfo.Distance = KillerPawn ? KillerPawn->GetDistanceTo(VictimPawn) : 0.0f;
    PlayerState->OnRep_DeathInfo();

    if (!KillerPlayerState) KillerPlayerState = PlayerState;
    if (!KillerPawn) KillerPawn = VictimPawn;

    if (KillerPlayerState && KillerPawn && KillerPawn->GetController() != PlayerController)
    {
        int32 KillerScore = KillerPlayerState->KillScore + 1;
        int32 TeamScore = KillerPlayerState->TeamKillScore + 1;
		
        KillerPlayerState->KillScore = KillerScore;
        KillerPlayerState->TeamKillScore = TeamScore;
        KillerPlayerState->ClientReportTeamKill(TeamScore);
        KillerPlayerState->ClientReportKill(PlayerState);

        auto Team = KillerPlayerState->PlayerTeam;
        if (Team) {
            for (int32 i = 0; i < Team->TeamMembers.Num(); ++i) {
                auto MemberPlayerState = (AFortPlayerStateAthena*)Team->TeamMembers[i]->PlayerState;
                if (MemberPlayerState != KillerPlayerState) {
                    MemberPlayerState->TeamKillScore = TeamScore;
                    KillerPlayerState->ClientReportTeamKill(TeamScore);
                }
            }
        }

        FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;
        if (Inventory.ItemInstances.Num() != 0)
        {
            for (int i = Inventory.ItemInstances.Num() - 1; i >= 0; --i)
            {
                UFortWorldItem* ItemInstance = Inventory.ItemInstances[i];
                if (!ItemInstance || !ItemInstance->CanBeDropped()) continue;
                FortKismetLibrary::SpawnPickup(
                    ItemInstance->ItemEntry.ItemDefinition,
                    PlayerController->MyFortPawn->K2_GetActorLocation(),
                    ItemInstance->ItemEntry.Count,
                    EFortPickupSourceTypeFlag::Player,
                    EFortPickupSpawnSource::Unset,
                    PlayerController->MyFortPawn,
                    true
                );
            }
        }

        if (bLategame)
        {
            int MaxHealth = KillerPawn->GetMaxHealth();
            int MaxShield = KillerPawn->GetMaxShield();
		
            ApplySiphon(KillerPawn, 75, MaxHealth, MaxShield);

            static FGameplayTag EarnedElim = { UKismetStringLibrary::Conv_StringToName(L"Event.EarnedElimination") };
            FGameplayEventData Data{};
            Data.EventTag = EarnedElim;
            Data.ContextHandle = KillerPlayerState->AbilitySystemComponent->MakeEffectContext();
            Data.Instigator = KillerPlayerState->GetOwner();
            Data.Target = PlayerState;
            Data.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(PlayerState);

            UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(((AFortPlayerController*)KillerPawn->GetController())->MyFortPawn, EarnedElim, Data);
        }
    }

    bool bRebooting = false; // will need later on ✌️ - andrew
    if (!KillerPawn->GetController()->IsA(AFortAthenaAIBotController::StaticClass()))
    {
        auto PlayerTeam = PlayerState->PlayerTeam;

        if (PlayerTeam && PlayerTeam->TeamMembers.Num() > 1) {
            bRebooting = true;
            for (auto& MemberController : PlayerTeam->TeamMembers) {
                auto MemberAthenaController = (AFortPlayerControllerAthena*)MemberController;
                if (MemberAthenaController && MemberAthenaController != PlayerController) {
                    if (!MemberAthenaController->bMarkedAlive) {
                        bRebooting = false;
                        break;
                    }
                }
            }
        }
    }

    UFortWeaponItemDefinition* ItemDef = nullptr;
    if (DeathReport.DamageCauser)
    {
        if (DeathReport.DamageCauser->IsA(AFortProjectileBase::StaticClass()))
        {
            auto Owner = (AFortWeapon*)(DeathReport.DamageCauser->GetOwner());
            ItemDef = Owner->WeaponData; 
        }

        if (auto WeaponDef = (AFortWeapon*)(DeathReport.DamageCauser))
        {
            ItemDef = WeaponDef->WeaponData;
        } 
    }

    if (!bRespawningAllowed)
    {
        static void (*RemoveFromAlivePlayers)(
            AFortGameModeAthena*, AFortPlayerController*, AFortPlayerStateAthena*, AFortPawn*, UFortWeaponItemDefinition*, EDeathCause, char
            ) = decltype(RemoveFromAlivePlayers)(InSDKUtils::GetImageBase() + 0xFAE8C0);
        RemoveFromAlivePlayers(GameMode, PlayerController,
            KillerPlayerState == PlayerState ? nullptr : KillerPlayerState, KillerPawn, ItemDef, PlayerState->DeathInfo.DeathCause, 0);

        for (auto& Controller : GameMode->AlivePlayers)
        {
            FGameplayTagContainer SourceTags;
            FGameplayTagContainer TargetTags;
            FGameplayTagContainer ContextTags;
            UFortQuestManager* QuestManager = Controller->GetQuestManager(ESubGame::Athena);
            QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
            FortQuestManager::SendStatEventWithTags(QuestManager, EFortQuestObjectiveStatEvent::AthenaOutlive, nullptr, TargetTags, SourceTags, ContextTags, 1);
        }
        
        int32 AliveCount = GameMode->AlivePlayers.Num();
        int32 KillScore = KillerPlayerState == PlayerState ? 0 : KillerPlayerState->KillScore;
        PlayerState->Place = AliveCount + 1;

        int32 TotalXpEarned = 0;
        int32 TotalScoreEarned = 0;
        RewardResult.TotalSeasonXpGained = 100;

        static auto StartingPlayerCount = GameMode->AlivePlayers.Num() + 1;
        TeamStats.Place = PlayerState->Place;
        TeamStats.TotalPlayers = StartingPlayerCount;
        
        MatchStats.Stats[0] = PlayerState->Place; // idk
        MatchStats.Stats[1] = 2; // idk
        MatchStats.Stats[2] = 3; // idk
        MatchStats.Stats[3] = KillScore;
        MatchStats.Stats[4] = 5; // idk
        MatchStats.Stats[5] = 6; // idk
        MatchStats.Stats[6] = 0; // assists
        MatchStats.Stats[7] = 0; // revives
        MatchStats.Stats[8] = PlayerState->TeamAverageDamageDealt;
        MatchStats.Stats[9] = 10; // distance traveled 
        MatchStats.Stats[10] = 11; // idk
        MatchStats.Stats[11] = 12; // idk 
        MatchStats.Stats[12] = 13;// idk
        MatchStats.Stats[13] = 14; // idk
        MatchStats.Stats[14] = 15; // idk
        MatchStats.Stats[15] = 16; // idk
        MatchStats.Stats[16] = 17; // idk
        MatchStats.Stats[17] = 18; // then again idk
        MatchStats.Stats[18] = StartingPlayerCount; 
        
        std::wstring TimeStr = std::to_wstring(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        MatchStats.MatchEndTime = FString(TimeStr.c_str());
        MatchStats.MatchID = GameState->GameSessionId;
        MatchStats.MatchPlatform = PlayerState->GetPlatform();
        
        if (KillScore > 0)
        {
            TotalXpEarned += 100;
            TotalScoreEarned += 50;

            FAthenaAwardGroup FirstKill;
            FirstKill.RewardSource = ERewardSource::FirstKill;
            FirstKill.BookXp = 100;
            FirstKill.Score = 50;
        }

        if (KillScore >= 2)
        {
            KillScore--;

            TotalXpEarned += KillScore * 50;
            TotalScoreEarned += KillScore * 15;

            FAthenaAwardGroup FirstKill;
            FirstKill.RewardSource = ERewardSource::TeamKills;
            FirstKill.BookXp = KillScore * 50;
            FirstKill.Score = KillScore * 15;
        }   

        RewardResult.TotalSeasonXpGained = RewardResult.TotalSeasonXpGained + TotalXpEarned;
        MatchStats.Stats[19] = RewardResult.TotalSeasonXpGained; // still dk

        PlayerController->ClientSendMatchStatsForPlayer(MatchStats);
        PlayerController->ClientSendEndBattleRoyaleMatchForPlayer(true, RewardResult);
        PlayerController->ClientSendTeamStatsForPlayer(TeamStats);

        if (AliveCount <= GameState->CurrentPlaylistInfo.BasePlaylist->MaxTeamSize)
        {
            AFortTeamInfo* LastAliveTeam = nullptr;
    
            for (int32 i = 0; i < GameMode->AlivePlayers.Num(); ++i) {
                if (GameMode->AlivePlayers[i] && GameMode->AlivePlayers[i]->PlayerState) {
                    auto AlivePlayerState = Cast<AFortPlayerStateAthena>(GameMode->AlivePlayers[i]->PlayerState);
                    if (AlivePlayerState && AlivePlayerState->PlayerTeam) {
                        LastAliveTeam = AlivePlayerState->PlayerTeam;
                        break;
                    }
                }
            }
    
            if (LastAliveTeam) {
                for (int32 i = 0; i < LastAliveTeam->TeamMembers.Num(); ++i) {
                    auto WinningController = Cast<AFortPlayerControllerAthena>(LastAliveTeam->TeamMembers[i]);
                    if (WinningController) {
                        auto WinningPlayerState = Cast<AFortPlayerStateAthena>(WinningController->PlayerState);

                        if (WinningPlayerState)
                        {
                            WinningPlayerState->Place = 1;
                            WinningPlayerState->OnRep_Place();

                            auto WinnerMatchReport = WinningController->MatchReport;
                            if (WinnerMatchReport)
                            {
                                WinnerMatchReport->MatchStats.Stats[0] = PlayerState->Place; // idk
                                WinnerMatchReport->MatchStats.Stats[1] = 2; // idk
                                WinnerMatchReport->MatchStats.Stats[2] = 3; // idk
                                WinnerMatchReport->MatchStats.Stats[3] = KillScore;
                                WinnerMatchReport->MatchStats.Stats[4] = 5; // idk
                                WinnerMatchReport->MatchStats.Stats[5] = 6; // idk
                                WinnerMatchReport->MatchStats.Stats[6] = 0; // assists
                                WinnerMatchReport->MatchStats.Stats[7] = 0; // revives
                                WinnerMatchReport->MatchStats.Stats[8] = PlayerState->TeamAverageDamageDealt;
                                WinnerMatchReport->MatchStats.Stats[9] = 10; // distance traveled 
                                WinnerMatchReport->MatchStats.Stats[10] = 11; // idk
                                WinnerMatchReport->MatchStats.Stats[11] = 12; // idk 
                                WinnerMatchReport->MatchStats.Stats[12] = 13;// idk
                                WinnerMatchReport->MatchStats.Stats[13] = 14; // idk
                                WinnerMatchReport->MatchStats.Stats[14] = 15; // idk
                                WinnerMatchReport->MatchStats.Stats[15] = 16; // idk
                                WinnerMatchReport->MatchStats.Stats[16] = 17; // idk
                                WinnerMatchReport->MatchStats.Stats[17] = 18; // then again idk
                                WinnerMatchReport->MatchStats.Stats[18] = StartingPlayerCount; // uh something 
                                WinnerMatchReport->MatchStats.Stats[19] = RewardResult.TotalSeasonXpGained + TotalXpEarned; // still dk
                                WinnerMatchReport->MatchStats.MatchEndTime = FString(TimeStr.c_str());
                                WinnerMatchReport->MatchStats.MatchID = GameState->GameSessionId;
                                WinnerMatchReport->MatchStats.MatchPlatform = PlayerState->GetPlatform();
                                
                                WinningController->ClientSendMatchStatsForPlayer(WinnerMatchReport->MatchStats);
                                WinningController->ClientSendTeamStatsForPlayer(WinnerMatchReport->TeamStats);
                                WinningController->ClientSendEndBattleRoyaleMatchForPlayer(true, WinnerMatchReport->EndOfMatchResults);
                                GameState->WinningTeam = PlayerState->TeamIndex;
                                GameState->WinningPlayerState = WinningPlayerState;
                                GameState->OnRep_WinningTeam();
                                GameState->OnRep_WinningPlayerState();
                            }
                        }
                    }
                }
            }
        }
    }

    static auto PlaylistId = GameState->CurrentPlaylistInfo.BasePlaylist->PlaylistName;
    TArray<FFortQuestObjectiveCompletion> Objectives;
    PlayerController->AthenaProfile->EndBattleRoyaleGame(
        Objectives,
        UKismetStringLibrary::Conv_NameToString(PlaylistId),
        MatchStats,
        0,
        0,
        1.f,
        true,
        true,
        {},
        {});

    FGameplayTagContainer SourceTags;
    FGameplayTagContainer TargetTags;
    FGameplayTagContainer ContextTags;
    UFortQuestManager* QuestManager = PlayerController->GetQuestManager(ESubGame::Athena);
    if (!QuestManager) return;
	
    QuestManager->GetSourceAndContextTags(&SourceTags, &ContextTags);
    FortQuestManager::SendStatEventWithTags(QuestManager, EFortQuestObjectiveStatEvent::AthenaRank, nullptr, TargetTags, SourceTags, ContextTags, 1);
    
    return Originals::ClientOnPawnDied(PlayerController, DeathReport);
}

void FortPlayerControllerAthena::ServerPlayEmoteItem(AFortPlayerControllerAthena* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber)
{
    if (!PlayerController || !EmoteAsset)
        return;

    static UClass *DanceAbility = StaticLoadObject<UClass>("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
    static UClass *SprayAbility = StaticLoadObject<UClass>("/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");

    if (!DanceAbility || !SprayAbility)
        return;

    auto EmoteDef = (UAthenaDanceItemDefinition *)EmoteAsset;
    if (!EmoteDef)
        return;

    PlayerController->MyFortPawn->bMovingEmote = EmoteDef->bMovingEmote;
    PlayerController->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;
    PlayerController->MyFortPawn->bMovingEmoteForwardOnly = EmoteDef->bMoveForwardOnly;
    PlayerController->MyFortPawn->EmoteWalkSpeed = EmoteDef->WalkForwardSpeed;

    FGameplayAbilitySpec Spec{};
    UGameplayAbility *Ability = nullptr;

    if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
    {
        Ability = (UGameplayAbility *)SprayAbility->DefaultObject;
    }

    if (Ability == nullptr)
    {
        Ability = (UGameplayAbility *)DanceAbility->DefaultObject;
    }

    static void (*AbilitySpecConstructor)(FGameplayAbilitySpec *, UGameplayAbility *, int, int, UObject *) = decltype(AbilitySpecConstructor)(InSDKUtils::GetImageBase() + 0x868290);
    static FGameplayAbilitySpecHandle (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent *ASC, FGameplayAbilitySpecHandle *, FGameplayAbilitySpec, void *) = decltype(GiveAbilityAndActivateOnce)(InSDKUtils::GetImageBase() + 0x843f10);
    AbilitySpecConstructor(&Spec, Ability, 1, -1, EmoteDef);
    GiveAbilityAndActivateOnce(((AFortPlayerStateAthena *)PlayerController->PlayerState)->AbilitySystemComponent, &Spec.Handle, Spec, nullptr);
}

void FortPlayerControllerAthena::ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* PlayerController)
{
    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
	auto GameState = (AFortGameStateAthena*)UWorld::GetWorld()->GameState;
	auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
	AFortPawn* OldPawn = PlayerController->MyFortPawn;
	PlayerState->RespawnData.bRespawnDataAvailable = true;
	PlayerState->RespawnData.bServerIsReady = true;
    
	if (GameState->IsRespawningAllowed(PlayerState) || bCreative)
	{
		if (PlayerState->RespawnData.bServerIsReady && PlayerState->RespawnData.bRespawnDataAvailable)
		{
			PlayerState->RespawnData.bClientIsReady = true;
				
			FTransform SpawnTransform{};
			SpawnTransform.Translation = PlayerState->RespawnData.RespawnLocation;
		    SpawnTransform.Rotation = UKismetMathLibrary::Conv_RotatorToTransform(PlayerState->RespawnData.RespawnRotation).Rotation;
		    SpawnTransform.Scale3D = { 1,1,1 };
		    
			AFortPlayerPawn* Pawn = (AFortPlayerPawn*)GameMode->SpawnDefaultPawnAtTransform(PlayerController, SpawnTransform); 
			if (!Pawn) return;

			PlayerController->Possess(Pawn);
			auto vol = PlayerController->GetCurrentVolume();
			if (!vol) return;
			auto cmpnt = (UFortMutatorListComponent*)vol->GetComponentByClass(UFortMutatorListComponent::StaticClass());
			auto muts = cmpnt->Mutators;
			static auto HealthAndShieldMut = AFortAthenaMutator_HealthAndShield::StaticClass();
			for (int32 i = 0; i < muts.Num(); i++)
			{
				auto mut = muts[i];
				if (mut->IsA(HealthAndShieldMut))
				{
					auto castmut = Cast<AFortAthenaMutator_HealthAndShield>(mut);

					float MaxHealth = castmut->MaxHealth;
					float MaxShield = castmut->MaxShield;
					float StartingHealth = MaxHealth * (castmut->StartingHealth / 100);
					float StartingShield = MaxShield * (castmut->StartingShield / 100);

					Pawn->SetMaxHealth(MaxHealth);
					Pawn->SetHealth(StartingHealth);
					Pawn->SetMaxShield(MaxShield);
					Pawn->SetShield(StartingShield);
					break;
				}
			}
			
			PlayerController->RespawnPlayerAfterDeath(true);
		}
	}
    
	if (OldPawn)
		OldPawn->K2_DestroyActor();
}

void FortPlayerControllerAthena::Setup()
{
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x108, ServerAcknowledgePossession);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x1FD, ServerExecuteInventoryItem);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x1BC, ServerPlayEmoteItem);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x433, ServerAttemptAircraftJump);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x422, ServerGiveCreativeItem);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, VFT>(0x457, ServerClientIsReadyToRespawn);
    UDetoursLibrary::InitializeDetour<AFortPlayerControllerAthena, MinHook>(0x1aec9a0, ClientOnPawnDied, &Originals::ClientOnPawnDied);
}

