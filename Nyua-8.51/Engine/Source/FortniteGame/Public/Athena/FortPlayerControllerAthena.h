#pragma once

#include "CoreGlobals.h"
#include <Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h>

#include "nlohmann.hpp"

struct PlayerData { std::string AccountID; };
inline std::unordered_map<class APlayerState*, PlayerData> PlayerMap;

inline std::string GetAccountID(AFortPlayerStateAthena* This)
{
	auto it = PlayerMap.find(This);
	if (it != PlayerMap.end())
	{
		return it->second.AccountID;
	}
        
	uint8* ReplicationBytesData = This->UniqueId.ReplicationBytes.GetData();
	uint8 AIDLen = *(uint8*)(ReplicationBytesData + 1);
	std::stringstream ss;

	for (int i = 0; i < AIDLen; i++)
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)*(uint8*)(ReplicationBytesData + 2 + i);

	PlayerData data;
	data.AccountID = ss.str();
	PlayerMap[This] = data;
        
	return data.AccountID;
}

class FortPlayerControllerAthena
{
private:
    class Originals
    {
    public:
       static inline void (*ClientOnPawnDied)(AFortPlayerControllerAthena *, FFortPlayerDeathReport &DeathReport);
    };
    
public:

	
    static void ServerAcknowledgePossession(AFortPlayerControllerAthena* PlayerController, APawn* P);
    static void ServerExecuteInventoryItem(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid);
    static void ServerPlayEmoteItem(AFortPlayerControllerAthena* PlayerController, UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber);
    static void ClientOnPawnDied(AFortPlayerControllerAthena *, FFortPlayerDeathReport& DeathReport);
    static void ServerGiveCreativeItem(AFortPlayerControllerAthena* PlayerController,FFortItemEntry* ItemEntry);
    static void ServerAttemptAircraftJump(AFortPlayerControllerAthena* PlayerController, FRotator& Rotation);
    static void ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* PlayerController);
    
    static void Setup();
};
