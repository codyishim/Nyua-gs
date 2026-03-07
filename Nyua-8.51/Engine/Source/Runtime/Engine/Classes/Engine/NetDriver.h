#pragma once

#include "CoreGlobals.h"

class NetDriver
{
public:
    class Originals
    {
    public:
        static inline void (*TickFlush)(UNetDriver*);
    };
public:
    static inline bool (*InitListen)(UNetDriver* self, UWorld* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
    static inline void (*SetWorld)(UNetDriver* self, UWorld* InWorld);
public:
    static void TickFlush(UNetDriver* NetDriver);
public:
    static void Setup();
};
