#pragma once

#include "CoreGlobals.h"
#include <type_traits>

class FortMinigame {
private:
    class Originals
    {
    public:
       static inline void (*ChangeMinigameState)(AFortMinigame*, EFortMinigameState);
    };
    
public:
    static void ChangeMinigameState(AFortMinigame* Minigame, EFortMinigameState State);
    static void Setup();
};
