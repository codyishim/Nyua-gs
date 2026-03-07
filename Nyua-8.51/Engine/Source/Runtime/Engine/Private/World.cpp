#include "Engine/World.h"

#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "UObject/UnrealNames.h"

bool World::Listen(UWorld* InWorld)
{
    if (bProd) Port = UKismetMathLibrary::RandomIntegerInRange(7777,8888);
    
    FURL InURL{};
    InURL.Port = Port;
    
    UE_LOG(LogNet, Log, "UWorld::Listen");
    
    if (InWorld->NetDriver)
    {
        UE_LOG(LogNet, Error, "UEngine::BroadcastNetworkFailure: NetDriverAlreadyExists");
        return false;
    }

    UEngine* Engine = UFortEngine::GetEngine();

    if (!Engine)
    {
        UE_LOG(LogNet, Error, "UEngine::GetEngine()");
        return false;
    }
    
    if (auto NetDriver = Engine::CreateNetDriver(Engine, InWorld, NAME_GameNetDriver))
    {
        NetDriver->World = InWorld;
        NetDriver->NetDriverName = NAME_GameNetDriver;
        
        FString Error;
        if (!NetDriver::InitListen(NetDriver, InWorld, InURL, true, Error))
        {
            UE_LOG(LogWorld, Log, "Failed to listen: %s", Error.ToString().c_str());

            return false;
        }

        NetDriver::SetWorld(NetDriver, InWorld);
        
        for (auto& LevelCollection : InWorld->LevelCollections)
        {
            LevelCollection.NetDriver = NetDriver;
        }

        InWorld->NetDriver = NetDriver;
    }
    else
    {
        return false;
    }
    
    SetConsoleTitleA(std::format("Nyua 8.51 | Listening on Port {} | Joinable = False", Port).c_str());

    return true;
}

void World::Setup()
{
    UDetoursLibrary::InitializeDetour<UWorld, MinHook>(0x30100a0, GetNetMode);
    
    auto Addr = InSDKUtils::GetImageBase() + 0x2FC8C96;
    UDetoursLibrary::Rel32Swap((void**)&Addr, Listen);
}

