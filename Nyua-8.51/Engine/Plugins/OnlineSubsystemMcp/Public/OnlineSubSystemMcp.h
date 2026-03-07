#pragma once

#include "CoreGlobals.h"

class FOnlineSubSystemMcp
{
public:
    static bool ProcessRequestAsUser(FOnlineSubSystemMcp* OnlineSubSystemMcp, void* Service, void* Id, void* Request, FString& Err);

    static void Setup();
};