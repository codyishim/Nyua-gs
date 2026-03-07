#pragma once

#include "CoreGlobals.h"

enum EContextCredentials : int32
{
    CXC_Client                   = 0x0,
    CXC_DedicatedServer          = 0x1,
    CXC_Cheater                  = 0x2,
    CXC_Public                   = 0x3,
};

class McpProfileGroup
{
private:
    class Originals
    {
    public:
        static inline void (*SendRequestNow)(UMcpProfileGroup* ProfileGroup, __int64* HttpRequest, EContextCredentials ContextCredentials);
    };
public:
    static void SendRequestNow(UMcpProfileGroup* ProfileGroup, __int64* HttpRequest, EContextCredentials ContextCredentials);

    static void Setup();
};