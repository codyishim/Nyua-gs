#pragma once

#include "CoreGlobals.h"

class FortPlayerPawn
{
private:
    class Originals
    {
    public:
    };

public:
  
    static void Setup();
   
    static void ServerHandlePickup(AFortPlayerPawnAthena* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);
    static void ServerSendZiplineState(AFortPlayerPawn* Pawn, FZiplinePawnState& InState);
    static void TryToAutoPickup(AFortPlayerPawn* Pawn, AFortPickup *Pickup);
};