#include "../Public/OnlineSubSystemMcp.h"
#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"
#include "Engine/Plugins/MemoryLibary/Source/Public/MemoryLibrary.h"

bool FOnlineSubSystemMcp::ProcessRequestAsUser(FOnlineSubSystemMcp* OnlineSubSystemMcp, void* Service, void* Id, void* Request, FString& Err)
{
    auto Function = reinterpret_cast<bool (*)(FOnlineSubSystemMcp*, void*, FString&, void*, FString&)>(InSDKUtils::GetImageBase() + 0x5FAE60);
    FString ID = L"007c0bfe154c4f5396648f013c641dcf";
    return Function(OnlineSubSystemMcp, Service, ID, Request, Err);
}

struct FServicePermissionsMcp {
public:
    char Unk_0[0x10];
    class FString Id;
};

FServicePermissionsMcp* MatchmakingServicePerms(int64, int64)
{
    static FServicePermissionsMcp* Perms = new FServicePermissionsMcp{ .Id = L"ec684b8c687f479fadea3cb2ad83f5c6" };
    return Perms;
}

void FOnlineSubSystemMcp::Setup()
{
    UDetoursLibrary::InitializeDetour<UClass, MinHook>(0x5FB150, ProcessRequestAsUser);
    UDetoursLibrary::InitializeDetour<UClass, MinHook>(0x5ED200, MatchmakingServicePerms);

    UMemoryLibrary::Null(0x1347DE0);
    UMemoryLibrary::Null(0x1CED030);
    UMemoryLibrary::Null(0x1BE4B30);
}
