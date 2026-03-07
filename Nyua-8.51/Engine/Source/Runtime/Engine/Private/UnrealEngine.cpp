#include "Engine/Engine.h"

void Engine::Setup()
{
    CreateNetDriver = decltype(CreateNetDriver)(InSDKUtils::GetImageBase() + 0x2FBED30);
}
