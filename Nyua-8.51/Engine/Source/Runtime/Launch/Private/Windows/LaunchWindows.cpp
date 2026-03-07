#include "CoreGlobals.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/ReplicationDriver.h"
#include "Engine/World.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Plugins/MemoryLibary/Source/Public/MemoryLibrary.h"
#include "Engine/Plugins/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Public/AbilitySystemComponent.h"
#include <Creative/FortAthenaCreativePortal.h>
#include <Player/FortPlayerController.h>

#include "Building/BuildingSMActor.h"
#include "Components/FortControllerComponent_Interaction.h"
#include <Player/FortPlayerPawn.h>

#include "Creative/B_Prj_Athena_PlaysetGrenade.h"
#include "Engine/Plugins/OnlineSubsystemMcp/Public/OnlineSubSystemMcp.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include <Creative/FortMinigame.h>
#include <Athena/FortVehicleAthena.h>

#include "Building/BuildingContainer.h"
#include "Quests/FortQuestManager.h"

static inline char (*GetStringOG)(__int64 a1, const wchar_t* Section, const wchar_t* Key, __int64* Value, __int64* Filename)  = nullptr;
char GetStringINI(__int64 a1, const wchar_t* Section, const wchar_t* Key, __int64* Value, __int64* Filename) {
    if (Section && Key) {
        if (wcscmp(Section, L"OnlineSubsystemMcp.McpProfile") == 0 && wcslen(Key) == 0) {
            Key = L"McpDedicatedServerCommandUrl";
        }
    }
    
    return GetStringOG(a1, Section, Key, Value, Filename);
}

static void* (*ProcessEventOG)(UObject*, UFunction*, void*);
static std::vector<std::string> LoggedFunctions;
void* ProcessEvent(UObject* Obj, UFunction* Function, void* Params)
{
    if (Function)
    {
        std::string FunctionName = Function->GetName();

        if (FunctionName.contains("OnLoaded_3645F4484F4ECED813C69D92F55C7A1F"))
        {
            struct OnLoaded_3645F4484F4ECED813C69D92F55C7A1F final { class UObject* Loaded;  };
            auto Param = *(OnLoaded_3645F4484F4ECED813C69D92F55C7A1F*)Params;
            B_Prj_Athena_PlaysetGrenade::OnLoaded_3645F4484F4ECED813C69D92F55C7A1F((AB_Prj_Athena_PlaysetGrenade_C*)Obj, Param.Loaded);
        }
        
        if (std::find(LoggedFunctions.begin(), LoggedFunctions.end(), FunctionName) == LoggedFunctions.end()) {
            std::cout << "ProcessEvent: " << FunctionName << std::endl;
            LoggedFunctions.push_back(FunctionName);
        }
    }

    return ProcessEventOG(Obj, Function, Params);
}

DWORD WINAPI LaunchWindowsStartup(LPVOID)
{
    Sleep(5000);
    
    AllocConsole();

    FILE* OutIgnoredNewStreamPtr = nullptr;
    freopen_s(&OutIgnoredNewStreamPtr, "CONIN$", "r", stdin);
    freopen_s(&OutIgnoredNewStreamPtr, "CONOUT$", "w", stdout);
    freopen_s(&OutIgnoredNewStreamPtr, "CONOUT$", "w", stderr);

    SetConsoleTitleA("Nyua 8.51 | Setting Up");

    UDetoursLibrary::Setup();

    UMemoryLibrary::Null(0x654EC1D);
    UMemoryLibrary::Null(0x2815170);
    UMemoryLibrary::Null(0x12e7410);
    UMemoryLibrary::Null(0x1704270);
    UDetoursLibrary::InitializeDetour<AGameSession, EDetourType::MinHook>(0x2c03d20, UDetoursLibrary::ReturnTrueDetour); // AGameSession::KickPlayer

    UDetoursLibrary::InitializeDetour<UClass, MinHook>(Offsets::ProcessEvent, ProcessEvent, &ProcessEventOG);
    UDetoursLibrary::InitializeDetour<UClass, MinHook>(0x1C79650,  GetStringINI, &GetStringOG); // for proper endbattleroyalegame
    
    UMemoryLibrary::Patch<bool>(0x5940a13, false); // GIsClient
    UMemoryLibrary::Patch<bool>(0x5940A14, true); // GIsServer

    auto before = (uint8_t*)(InSDKUtils::GetImageBase() + 0x3012AEA);
    DWORD dwProtection;
    VirtualProtect((PVOID)before, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
    *before = 0x74;
    DWORD dwTemp;
    
    VirtualProtect((PVOID)before, 1, dwProtection, &dwTemp);

    Engine::Setup();
    ReplicationDriver::Setup();
    NetDriver::Setup();
    World::Setup();
    FortInventory::Setup();

    McpProfileGroup::Setup();
    FortPlayerPawn::Setup();
    
    FortGameModeAthena::Setup();
    FortPlayerControllerAthena::Setup();
    FortControllerComponent_Interaction::Setup();

    FortPlayerController::Setup();
    FortPickup::Setup();
    FortAthenaCreativePortal::Setup();
    FortVehicleAthena::Setup();
    FortQuestManager::Setup();
    FortMinigame::Setup();

    AbilitySystemComponent::Setup();
    BuildingSMActor::Setup();
    BuildingContainer::Setup();

   // FOnlineSubSystemMcp::Setup();
    
    UDetoursLibrary::Finalize();

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogFort VeryVerbose", nullptr);
    
    UFortEngine::GetEngine()->GameInstance->LocalPlayers.Remove(0);
    		
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);
    }
    
    UE_LOG(LogServer, Log, "Opened Level");
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(NULL, 0, LaunchWindowsStartup, NULL, 0, NULL);
	
    return TRUE;
}

