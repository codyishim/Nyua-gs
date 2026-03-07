#include "Creative/ImproperIslandSaving.h"
#include <filesystem>
#include <fstream>
#include <mutex>

#include "Engine/World.h"
#include "Engine/Source/Runtime/CoreUObject/Public/Templates/Casts.h"
#include <thread>

namespace fs = std::filesystem;
using json = nlohmann::json;
static TArray<AActor*> Builder::GetActorsInVol(AFortVolume* Volume)
{
	TArray<AActor*> ret;
	TArray<AActor*> df;
	if (!Volume)
	{
		return ret;
	}
	static bool (*sub_1170BD0OG)(AFortVolume * AFortVolume, TArray<AActor*> *OutActors) = decltype(sub_1170BD0OG)(0x1170BD0 + __int64(GetModuleHandleW(0)));

	sub_1170BD0OG(Volume, &df);
	
	for (int i = 0; i < df.Num(); i++)
	{
		auto ac = df[i];
		if (ac)
		{
			if (ac->IsA(ABuildingActor::StaticClass()) || ac->IsA(AFortMinigameSettingsBuilding::StaticClass()))
				ret.Add(ac);
		}
	}
	return ret;
}

static std::string Builder::GetIslandName(AFortPlayerControllerAthena* Controller)
{
	return "1.island";
}

static std::string Builder::GetIslandPath(AFortPlayerControllerAthena* Controller)
{
	if (!Controller) return "";
	auto PS = Controller->PlayerState;
	if (!PS) return "";

	FString f = PS->GetPlayerName();
	std::string PlayerID = f.ToString();

	return "/creative/personal/" + PlayerID;
}

void Builder::LoadAllTraps(AFortPlayerControllerAthena* Controller, nlohmann::json& j, AFortVolume* Volume)
{
	if (!Controller) return;

	auto Pawn = Controller->MyFortPawn;
	if (!Pawn) return;

	auto Inventory = Controller->WorldInventory;
	if (!Inventory) return;
	
	if (!j.contains("Traps")) return;

	for (auto& trapJson : j["Traps"])
	{
		std::string classPath = trapJson["Class"];
		std::string AttachedTo = trapJson["AttachedTo"];
		auto DecoItemDefinition = GetTIDFromTrap(classPath);

		if (!DecoItemDefinition) continue;
		
		((AFortDecoTool_ContextTrap*)Pawn->CurrentWeapon)->ContextTrapItemDefinition = (UFortContextTrapItemDefinition*)DecoItemDefinition;

		auto VolumeLocation = Volume->K2_GetActorLocation();
		auto VolumeRotation = Volume->K2_GetActorRotation();
		
		FVector RelLoc(
			trapJson["RelativeLocation"]["x"],
			trapJson["RelativeLocation"]["y"],
			trapJson["RelativeLocation"]["z"]
		);

		FRotator RelRot(
			trapJson["Rotation"]["pitch"],
			trapJson["Rotation"]["yaw"],
			trapJson["Rotation"]["roll"]
		);

		FVector SpawnLoc = VolumeLocation + RelLoc;
		FRotator SpawnRot = FRotator(
			VolumeRotation.Pitch + RelRot.Pitch,
			VolumeRotation.Yaw + RelRot.Yaw,
			VolumeRotation.Roll + RelRot.Roll
		);
		std::wstring wAttachedTo(AttachedTo.begin(), AttachedTo.end());

		std::string attachedToStr(wAttachedTo.begin(), wAttachedTo.end());
		auto cls = StaticFindObject<UClass>(attachedToStr.c_str());
		if (!cls) continue;
		
		auto SpawnedActor = World::SpawnActor<ABuildingSMActor>(SpawnLoc, SpawnRot, cls);
		if (!SpawnedActor) continue;
		
		if (SpawnedActor->bIsPlayerBuildable)
			Cast<ABuildingSMActor>(SpawnedActor)->bPlayerPlaced = true;
		
		SpawnedActor->InitializeKismetSpawnedBuildingActor(SpawnedActor, nullptr, false);
		SpawnedActor->TeamIndex = ((AFortPlayerStateAthena*)Controller->PlayerState)->TeamIndex;

		((AFortDecoTool*)Pawn->CurrentWeapon)->ServerSpawnDeco(SpawnLoc, SpawnRot, SpawnedActor, EBuildingAttachmentType::ATTACH_Floor);
	}
}

inline FString JsonToFString(const std::string& input)
{
	if (input.empty()) return FString();

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, input.c_str(),
		(int)input.size(), nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(),
		(int)input.size(), &wstr[0], size_needed);

	return FString(wstr.c_str());
}

inline const FString StringToFString(const std::string str)
{
	std::wstring WStr = std::wstring(str.begin(), str.end()).c_str();
	FString FStr(WStr.c_str());
	return FStr;
}

struct FOptionResult
{
	FString HumanValue;
	UPlaylistUserOptionBase* Option = nullptr;
};

static FOptionResult HumanValueFromValue(const FString& Name, const FString& Val, UPlaylistUserOptions* AllOptions)
{
	if (!AllOptions)
	{
		std::cout << "AllOptions is null" << std::endl;
		return {};
	}

	for (UPlaylistUserOptionBase* Option : AllOptions->Options)
	{
		if (Option && Option->GetOptionKey(false) == Name)
		{
			FString HumanVal = Option->GetOptionValueNameFromValue(Val);
			return { HumanVal, Option };
		}
	}

	std::cout << "No matching user option for " << Name.ToString() << std::endl;
	return {};
}


static json GetNewOptions(UFortActorOptionsComponent* the)
{
	FPropertyOverrideData data = the->PlayerOptionData;
	json result = json::object();
	for (auto& d : data.PropertyOverrides)
	{
		if (!d.PropertyName.IsValid() || !d.PropertyData.IsValid()) continue;
		FOptionResult Ret = HumanValueFromValue(d.PropertyName, d.PropertyData, the->PlayerOptions);
		if (!Ret.Option) continue;

		std::string Key = Ret.Option->GetOptionKey(true).ToString();
		std::string Value = Ret.HumanValue.ToString();
		result[Key] = Value;
		
		
	}
	return result;
}

static TMap<FString, FString> JsonToTMap(const nlohmann::json& optionsJson)
{
	TMap<FString, FString> Result;

	for (auto& [key, val] : optionsJson.items())
	{
		FString KeyFS = JsonToFString(key);
		std::string ValStr = val.get<std::string>();
		FString ValFS = JsonToFString(ValStr);

		// Only add valid entries
		if (!KeyFS.IsValid() && !ValFS.IsValid())
		{
		/*	TPair<FString, FString> fishyisgay = { KeyFS, ValFS };
			
			Result.Elements.Add(fishyisgay);*/
		}
		else
		{
			std::cout << "Invalid entry: key=" << key << std::endl;
		}
	}

	return Result;
}

void Builder::LoadActorSettings(AActor* Actor, nlohmann::json optionsJson)
{
	static auto FortActorOptionsComponentClass =
		StaticFindObject<UClass>("/Game/Items/Traps/Blueprints/Toys/ToyOptionsComponent.ToyOptionsComponent_C");

	if (auto options = (UFortActorOptionsComponent*)Actor->GetComponentByClass(FortActorOptionsComponentClass))
	{
		auto map = JsonToTMap(optionsJson);
		options->SetPropertyOverrides(map);
	}
}

bool Builder::LoadIsland(AFortPlayerControllerAthena* Controller)
{
	if (!Controller || !Controller->GetCreativePlotLinkedVolume())
		return false;

	auto Volume = Controller->GetCreativePlotLinkedVolume();
	auto actors = GetActorsInVol(Volume);

	for (int i = 0; i < actors.Num(); i++)
	{
		auto actor = actors[i];
		if (!actor) continue;
		
		if (actor->IsA(ABuildingActor::StaticClass()))
			(Cast<ABuildingActor>(actor))->SilentDie();
	}
	auto VolumeLocation = Volume->K2_GetActorLocation();
	auto VolumeRotation = Volume->K2_GetActorRotation();

	std::string basePath = GetIslandPath(Controller);
	std::string fileName = GetIslandName(Controller);
	fs::path fullPath = fs::current_path() / basePath / fileName;

	if (!exists(fullPath))
	{
		static auto MinigameSettingsMachine = StaticLoadObject<UClass>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/MinigameSettingsMachine.MinigameSettingsMachine_C");
		auto OK = Volume->GetTransform();
		World::SpawnActorOG<AActor>(MinigameSettingsMachine, OK, Volume);
		return false;
	}

	std::ifstream file(fullPath);
	if (!file.is_open()) return false;

	nlohmann::json j;
	file >> j;
	file.close();

	if (!j.contains("Actors")) return false;
	
	for (auto& actorJson : j["Actors"])
	{
		std::string classPath = actorJson["Class"];
		if (classPath.contains("MinigameSettingsMachine"))
		{
			for (int i = 0; i < actors.Num(); i++)
			{
				auto actor = actors[i];
				if (!actor) continue;
				if (actor->GetFullName().contains("MinigameSettingsMachine"))
				{
					auto& optionsJson = actorJson["Options"];
					if (!optionsJson.is_null() && optionsJson.is_object())
					{
						LoadActorSettings(actor, optionsJson);
						break;
					}
				}
			}
			continue;
		}

		auto Class = StaticFindObject<UClass>(classPath.c_str());
		if (!Class) continue;

		FVector RelLoc(
			actorJson["RelativeLocation"]["x"],
			actorJson["RelativeLocation"]["y"],
			actorJson["RelativeLocation"]["z"]
		);

		FRotator RelRot(
			actorJson["Rotation"]["pitch"],
			actorJson["Rotation"]["yaw"],
			actorJson["Rotation"]["roll"]
		);

		FVector SpawnLoc = VolumeLocation + RelLoc;
		FRotator SpawnRot = FRotator(
			VolumeRotation.Pitch + RelRot.Pitch,
			VolumeRotation.Yaw + RelRot.Yaw,
			VolumeRotation.Roll + RelRot.Roll
		);
		
		auto SpawnedActor = World::SpawnActor<ABuildingActor>(SpawnLoc, SpawnRot, Class);
		if (!SpawnedActor) continue;
		
		if (SpawnedActor->bIsPlayerBuildable)
			Cast<ABuildingSMActor>(SpawnedActor)->bPlayerPlaced = true;
		
		SpawnedActor->InitializeKismetSpawnedBuildingActor(SpawnedActor, nullptr, false);
		if (actorJson.contains("TeamIndex"))
			SpawnedActor->TeamIndex = actorJson["TeamIndex"];

		if (actorJson.contains("Options"))
		{
			auto& optionsJson = actorJson["Options"];
			if (!optionsJson.is_null() && optionsJson.is_object())
			{
				LoadActorSettings(SpawnedActor, optionsJson);
			}
		}
	}
	
	return true;
}

std::string AfterColon(const std::string& input) {
	auto pos = input.find(':');
	if (pos == std::string::npos) return input;
	return input.substr(pos + 1);
}

UObject* GetUserOptionBase(std::string data, UObject* obj)
{
	UObject* idk = nullptr;
	if (obj->GetFullName().contains("MinigameSettingsMachine_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Athena/Items/Gameplay/MinigameSettingsControl/GameSettingsOptions.GameSettingsOptions");
	}
	if (obj->GetFullName().contains("MinigamePlayerStartPlate_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Toy_PlayerStart_UserKnobs.Toy_PlayerStart_UserKnobs");
	}
	if (obj->GetFullName().contains("ExplosiveBarrel_GBA_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/ExplosiveBarrel/Knobs/Device_ExplosiveBarrel_UserKnobsList.Device_ExplosiveBarrel_UserKnobsList");
	}
	if (obj->GetFullName().contains("BP_Creative_CaptureArea_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/CaptureArea/Knobs/Device_CaptureArea_UserKnobsList.Device_CaptureArea_UserKnobsList");
	}
	if (obj->GetFullName().contains("B_ShootingTarget_Master_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/ShootingTargets/Knobs/Device_ShootingTarget_UserKnobsList.Device_ShootingTarget_UserKnobsList");
	}
	if (obj->GetFullName().contains("BP_DestructionObject_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/Destruction_Object/Device_DestructionObject_UserKnobs.Device_DestructionObject_UserKnobs");
	}
	if (obj->GetFullName().contains("Prop_Creative_Coin_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/Creative_Coin/Device_CreativeCoin_UserOptions.Device_CreativeCoin_UserOptions");
	}
	if (obj->GetFullName().contains("BP_Creative_Billboard_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/Billboard/Knobs/Creative_Billboard_UserOptions.Creative_Billboard_UserOptions");
	}
	if (obj->GetFullName().contains("BP_PinballBumper_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/Pinball_Bumper/Device_PInballBumper_UserKnobs.Device_PInballBumper_UserKnobs");
	}
	if (obj->GetFullName().contains("BP_PinballFlipper_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/Pinball_Flipper/Device_PinballFlipper_UserKnobs.Device_PinballFlipper_UserKnobs");
	}
	if (obj->GetFullName().contains("BP_CreativeRadio_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/JukeBox/Device_Radio_UserKnobs.Device_Radio_UserKnobs");
	}
	if (obj->GetFullName().contains("Toy_Floor_Siren_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Toy_Siren_UserKnobs.Toy_Siren_UserKnobs");
	}
	if (obj->GetFullName().contains("Toy_Floor_SpeedBoost_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Toy_SpeedBoost_UserKnobs.Toy_SpeedBoost_UserKnobs");
	}
	if (obj->GetFullName().contains("Toy_Floor_SpeedIncrease_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Toy_SpeedAdjust_UserKnobs.Toy_SpeedAdjust_UserKnobs");
	}
	if (obj->GetFullName().contains("Device_Floor_PlayerCheckpoint_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Device_Checkpoint_UserKnobsList.Device_Checkpoint_UserKnobsList");
	}
	if (obj->GetFullName().contains("Trap_Floor_Minigame_Spawner_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Toy_ItemSpawner_UserKnobsList.Toy_ItemSpawner_UserKnobsList");
	}
	if (obj->GetFullName().contains("Trap_Floor_GameStartingInventory_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Creative/Devices/TeamSettings/Knobs/Device_TeamSettings_UserKnobsList.Device_TeamSettings_UserKnobsList");
	}
	if (obj->GetFullName().contains("Device_Floor_EliminationZone_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Device_EliminationZone_UserKnobs.Device_EliminationZone_UserKnobs");
	}
	if (obj->GetFullName().contains("Device_Floor_Barrier_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Device_Barrier_UserKnobs.Device_Barrier_UserKnobs");
	}
	if (obj->GetFullName().contains("Device_Floor_SweepTrigger_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Device_MusicSequencer_UserKnobsList.Device_MusicSequencer_UserKnobsList");
	}
	if (obj->GetFullName().contains("Trap_Floor_Backup_C"))
	{
		idk = StaticFindObject<UObject>("/Game/Items/Traps/Blueprints/Toys/UserKnobs/Device_AITurret_UserKnobs.Device_AITurret_UserKnobs");
	}

	if (!idk) return nullptr;
	TArray<UObject*> arr = *(TArray<UObject*>*)(__int64(idk) + 0x0030);
	for (int i = 0; i < arr.Num(); i++)
	{
		auto d = arr[i];
		auto l = *(FString*)(__int64(d) + 0x0058);
		if (l.ToString() == data)
		{
			return d;
		}
	}
	return nullptr;
}

struct PlaylistUserOptionBase_GetOptionValueNameFromValue final
{
public:
	class FString                                 OptionValue;                                       // 0x0000(0x0010)(Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	class FString                                 ReturnValue;                                       // 0x0010(0x0010)(Parm, OutParm, ZeroConstructor, ReturnParm, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

static inline void Builder::SaveActor(nlohmann::json& j, nlohmann::json& ja, AActor* actor, FVector VolumeLocation, FRotator VolumeRotation)
{
	ja["Trap"] = false;
	ja["Class"] = UKismetSystemLibrary::GetPathName(actor->Class).ToString();
	bool isIslandSettings = actor->GetFullName().contains("MinigameSettingsMachine");

	if (!isIslandSettings)
	{
		auto CurrentBuildingActor = Cast<ABuildingActor>(actor);
		auto Loc = CurrentBuildingActor->K2_GetActorLocation() - VolumeLocation;
		auto Rot = CurrentBuildingActor->K2_GetActorRotation();
		Rot.Pitch -= VolumeRotation.Pitch;
		Rot.Yaw -= VolumeRotation.Yaw;
		Rot.Roll -= VolumeRotation.Roll;

		ja["RelativeLocation"] = { {"x", Loc.X}, {"y", Loc.Y}, {"z", Loc.Z} };
		ja["Rotation"] = { {"roll", Rot.Roll}, {"pitch", Rot.Pitch}, {"yaw", Rot.Yaw} };
		ja["TeamIndex"] = CurrentBuildingActor->TeamIndex;
	}

	nlohmann::json optionsJson;
	static auto FortActorOptionsComponentClass =
		StaticFindObject<UClass>("/Game/Items/Traps/Blueprints/Toys/ToyOptionsComponent.ToyOptionsComponent_C");

	if (auto options = (UFortActorOptionsComponent*)actor->GetComponentByClass(FortActorOptionsComponentClass))
	{
		if (options)
		{
			auto PlayerOptionData = *(TArray<FPropertyOverrideMk2>*)(__int64(options) + 0x0170 + 0x0108);
			optionsJson = GetNewOptions(options);
		}
	}


	ja["Options"] = optionsJson;
	j["Actors"].push_back(ja);
}

bool Builder::SaveIsland(AFortPlayerControllerAthena* Controller)
{
	if (!Controller || !Controller->GetCreativePlotLinkedVolume()) return false;

	auto Volume = Controller->GetCreativePlotLinkedVolume();

	nlohmann::json j;
	j["Actors"] = nlohmann::json::array();
	j["Traps"] = nlohmann::json::array();
	auto VolumeLocation = Volume->K2_GetActorLocation();
	auto VolumeRotation = Volume->K2_GetActorRotation();

	auto actors = GetActorsInVol(Volume);

	for (int i = 0; i < actors.Num(); i++)
	{
		auto actor = actors[i];
		if (!actor) continue;

		nlohmann::json ja;
		if (actor->IsA(ABuildingTrap::StaticClass()))
		{
			auto d = Cast<ABuildingTrap>(actor)->GetBuildingAttachedTo();
		}

		SaveActor(j, ja, actor, VolumeLocation, VolumeRotation);
	}

	std::string jsonStr;
	try
	{
		jsonStr = j.dump(4);
	}
	catch (const std::exception& e)
	{
		return false;
	}

	std::string basePath = GetIslandPath(Controller);  
	std::string fileName = GetIslandName(Controller);  
	fs::path fullPath = fs::current_path() / basePath / fileName;

	if (!exists(fullPath.parent_path()))
	{
		create_directories(fullPath.parent_path());
	}

	std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
	if (!file.is_open())
	{
		return false;
	}

	file << jsonStr;
	file.close();

	return true;
}

bool Builder::ResetIsland(AFortPlayerControllerAthena* Controller)
{
	if (!Controller || !Controller->GetCreativePlotLinkedVolume())
	{
		return false;
	}

	auto Volume = Controller->GetCreativePlotLinkedVolume();
	auto actors = GetActorsInVol(Volume);
	for (int i = 0; i < actors.Num(); i++)
	{
		auto actor = actors[i];
		if (!actor) continue;
		if (actor->IsA(ABuildingActor::StaticClass()))
		{
			(Cast<ABuildingActor>(actor))->SilentDie();
		}
	}

	std::string basePath = GetIslandPath(Controller);
	std::string fileName = GetIslandName(Controller);
	fs::path fullPath = fs::current_path() / basePath / fileName;

	if (!exists(fullPath)) return false;

	std::ifstream file(fullPath);
	if (!file.is_open()) return false;

	nlohmann::json j;
	file >> j;
	file.close();

	if (!j.contains("Actors")) return false;

	SaveIsland(Controller);

	return true;
}

UFortDecoItemDefinition* Builder::GetTIDFromTrap(std::string trap)
{
	if (trap.contains("Device_Floor_Barrier_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_BarrierDevice.TID_Floor_BarrierDevice");
	}
	if (trap.contains("Device_Floor_EliminationZone_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_EliminationZone.TID_Floor_EliminationZone");
	}
	if (trap.contains("Trap_Floor_GameStartingInventory_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/GameInventoryToy/TID_Floor_GameStartingInventory.TID_Floor_GameStartingInventory");
	}
	if (trap.contains("Trap_Floor_Minigame_Spawner_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/MinigameSpawner/TID_Floor_Minigame_Spawner.TID_Floor_Minigame_Spawner");
	}
	if (trap.contains("Device_Floor_PlayerCheckpoint_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_PlayerCheckpoint.TID_Floor_PlayerCheckpoint");
	}
	if (trap.contains("MinigamePlayerStartPlate_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Player_Start_Plate_VR_T01.TID_Floor_Player_Start_Plate_VR_T01");
	}
	if (trap.contains("Toy_Floor_SpeedIncrease_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_SpeedIncrease.TID_Floor_SpeedIncrease");
	}
	if (trap.contains("Toy_Floor_SpeedBoost_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_SpeedBoost.TID_Floor_SpeedBoost");
	}
	if (trap.contains("Trap_Floor_Scoreboard_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Scoreboard.TID_Floor_Scoreboard");
	}
	if (trap.contains("Toy_Floor_Siren_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Toy_Siren.TID_Floor_Toy_Siren");
	}
	if (trap.contains("Device_Floor_SweepTrigger_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_SweepTrigger.TID_Floor_SweepTrigger");
	}
	if (trap.contains("Trap_Floor_Backup_C"))
	{
		return StaticFindObject<UFortDecoItemDefinition>("/Game/Athena/Items/Traps/TID_Floor_Backup.TID_Floor_Backup");
	}
	
	return nullptr; 
}
