#pragma once

#include "CoreGlobals.h"

class ReplicationDriver
{
public:
    static inline int32 (*ServerReplicateActors)(UReplicationDriver*);
public:
    static void Setup()
    {
        ServerReplicateActors = decltype(ServerReplicateActors)(InSDKUtils::GetImageBase() + 0x95EDB0);
    }
};