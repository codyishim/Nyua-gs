#include "Engine/NetDriver.h"

#include "Engine/ReplicationDriver.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include <thread>

void NetDriver::TickFlush(UNetDriver* NetDriver)
{
    if (auto ReplicationDriver = NetDriver->ReplicationDriver)
    {
        ReplicationDriver::ServerReplicateActors(ReplicationDriver);   
    }

    static bool bStartedBus = false;
    if (GetAsyncKeyState(VK_F5) & 0x1 && !bStartedBus) {
        bStartedBus = true;
        UKismetSystemLibrary::ExecuteConsoleCommand(NetDriver->World, L"startaircraft", nullptr);
    }
    
    static bool bPlayerJoined = false;
    static bool bTerminating = false;
    if (!bPlayerJoined && NetDriver->ClientConnections.Num() > 0)
        bPlayerJoined = true;
    
    if (bPlayerJoined && !bTerminating && NetDriver->ClientConnections.Num() == 0)
    {
        bTerminating = true;
        std::thread([](){ 
            Sleep(7000);  
            TerminateProcess(GetCurrentProcess(), 0);
        }).detach();
    }

    return Originals::TickFlush(NetDriver);
}

void NetDriver::Setup()
{
    InitListen = decltype(InitListen)(InSDKUtils::GetImageBase() + 0x634c10);
    SetWorld = decltype(SetWorld)(InSDKUtils::GetImageBase() + 0x2d38590);

    UDetoursLibrary::InitializeDetour<UNetDriver, MinHook>(0x2d39300, TickFlush, &Originals::TickFlush);
}
