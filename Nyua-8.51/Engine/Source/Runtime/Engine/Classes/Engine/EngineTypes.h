#pragma once

#include "SDK/UnrealContainers.hpp"

enum class ELevelCollectionType : UC::uint8
{
    /**
     * The dynamic levels that are used for normal gameplay and the source for any duplicated collections.
     * Will contain a world's persistent level and any streaming levels that contain dynamic or replicated gameplay actors.
     */
    DynamicSourceLevels,

    /** Gameplay relevant levels that have been duplicated from DynamicSourceLevels if requested by the game. */
    DynamicDuplicatedLevels,

    /**
     * These levels are shared between the source levels and the duplicated levels, and should contain
     * only static geometry and other visuals that are not replicated or affected by gameplay.
     * These will not be duplicated in order to save memory.
     */
    StaticLevels,

    MAX
};
