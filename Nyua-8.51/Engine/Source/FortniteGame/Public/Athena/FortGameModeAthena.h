#pragma once

#include "CoreGlobals.h"

class FortGameModeAthena
{
public:
    class Originals
    {
    public:
        static inline bool (*ReadyToStartMatch)(AFortGameModeAthena*);
        static inline void (*HandleStartingNewPlayer)(AFortGameModeAthena*, AFortPlayerControllerAthena*);
        static inline void (*StartAircraftPhase)(AFortGameModeAthena*, char);
        static inline void (*StartNewSafeZonePhase)(AFortGameModeAthena*, int);
        static inline void (*OnAircraftExitedDropZone)(AFortGameModeAthena*, AFortAthenaAircraft*);
    };
public:
    static bool ReadyToStartMatch(AFortGameModeAthena* GameMode);
    static void HandleStartingNewPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer);
    static APawn* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AController* NewPlayer, AActor* StartSpot); 
    static void StartAircraftPhase(AFortGameModeAthena* GameMode,  char a2);
    static void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int NewSafeZonePhase);
    static void OnAircraftExitedDropZone(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft);
    static EFortTeam PickTeam(AFortGameModeAthena* GameMode, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller);

    static void Setup();
};