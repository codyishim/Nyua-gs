#pragma once

#include "CoreGlobals.h"

class Engine
{
public:
    static inline UNetDriver* (*CreateNetDriver)(UEngine*, UWorld*, FName);
public:
    static void Setup();
};