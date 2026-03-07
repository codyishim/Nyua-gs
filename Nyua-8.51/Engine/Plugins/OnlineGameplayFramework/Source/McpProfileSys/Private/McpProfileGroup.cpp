#include "Engine/Plugins/OnlineGameplayFramework/Source/McpProfileSys/Public/McpProfileGroup.h"

#include "Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h"

void McpProfileGroup::SendRequestNow(UMcpProfileGroup* ProfileGroup, long long* HttpRequest, EContextCredentials ContextCredentials)
{
    return Originals::SendRequestNow(ProfileGroup, HttpRequest, CXC_Public);
}

void McpProfileGroup::Setup()
{
    UDetoursLibrary::InitializeDetour<UMcpProfileGroup, EDetourType::MinHook>(0xcf2e80, SendRequestNow, &Originals::SendRequestNow);
}
