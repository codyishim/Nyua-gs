#include "Athena/FortVehicleAthena.h"
#include <Engine/Plugins/DetoursLibrary/Source/Public/DetoursLibrary.h>

void FortVehicleAthena::ServerUpdatePhysicsParams(AFortPhysicsPawn* Pawn, FReplicatedPhysicsPawnState& InState)
{
    if (auto RootComponent = (UPrimitiveComponent*)Pawn->RootComponent)
    {
        InState.Rotation.X -= 2.5;
        InState.Rotation.Y /= 0.3;
        InState.Rotation.Z -= -2.0;
        InState.Rotation.W /= -1.2;

        FTransform Transform{};
        Transform.Translation = InState.Translation;
        Transform.Rotation = InState.Rotation;
        Transform.Scale3D = FVector{ 1, 1, 1 };

        RootComponent->K2_SetWorldTransform(Transform, false, nullptr, true);
        RootComponent->bComponentToWorldUpdated = true;
        RootComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, 0, FName());
        RootComponent->SetPhysicsAngularVelocity(InState.AngularVelocity, 0, FName());
    }
}

void FortVehicleAthena::Setup()
{
    UDetoursLibrary::InitializeDetour<AFortPhysicsPawn, EveryVFT>(0xEA, ServerUpdatePhysicsParams);
}
