#pragma once

#include "CoreGlobals.h"
#include "EngineTypes.h"

class World
{
public:
       static inline FLevelCollection* (*FindCollectionByType)(UWorld* self, const ELevelCollectionType InType);
public:
       enum ENetMode  
       {  
              NM_Standalone,  
              NM_DedicatedServer,  
              NM_ListenServer,  
              NM_Client,  
              NM_MAX,  
       };  
       
       static inline ENetMode GetNetMode(UWorld* World) { return ENetMode::NM_DedicatedServer; }
       
       static bool Listen(UWorld* InWorld);

       struct FActorSpawnParameters
       {
       public:
              FName Name;

              AActor* Template;
              AActor* Owner;
              APawn* Instigator;
              ULevel* OverrideLevel;
              ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

       private:
              uint8	bRemoteOwned : 1;
       public:
              uint8	bNoFail : 1;
              uint8	bDeferConstruction : 1;
              uint8	bAllowDuringConstructionScript : 1;
              EObjectFlags ObjectFlags;
       };

       template<class T>
       static inline T* SpawnActor(FVector Location, FRotator Rotation = FRotator{ 0, 0, 0 }, UClass* Class = T::StaticClass(), AActor* Owner = nullptr)
       {
              FTransform Transform{};

              Transform.Rotation = UKismetMathLibrary::Conv_RotatorToTransform(Rotation).Rotation;
              Transform.Scale3D = {1,1,1};
              Transform.Translation = Location;

              FActorSpawnParameters addr{};

              addr.Owner = Owner;
              addr.bDeferConstruction = false;
              addr.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

              return (T*)((AActor * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(InSDKUtils::GetImageBase() + 0x2ca9b50))(UWorld::GetWorld(), Class, &Transform, &addr);
       }
       
       template<class T>
       static inline T* SpawnActorOG(UClass* Class, FTransform& Transform, AActor* Owner = nullptr)
       {
              FActorSpawnParameters addr{};

              addr.Owner = Owner;
              addr.bDeferConstruction = false;
              addr.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

              return ((AActor * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(InSDKUtils::GetImageBase() + 0x2ca9b50))(UWorld::GetWorld(), Class, &Transform, &addr);
       }
public:
       static void Setup(); 
};
