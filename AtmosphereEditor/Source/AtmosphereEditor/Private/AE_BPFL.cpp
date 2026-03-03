#include "AE_BPFL.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "GameFramework/Volume.h"
#include "FGSkySphere.h"
#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"
#include "Atmosphere/BiomeHelpers.h"

#pragma optimize("", off)



AActor* UAE_BPFL::CloneActor(UObject* WorldContextObject, AActor* ActorToClone, FTransform NewTransform, FName Tag)
{
    if (!ActorToClone || !WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;

    SpawnParams.Template = ActorToClone;

    SpawnParams.Owner = ActorToClone->GetOwner();
    SpawnParams.Instigator = ActorToClone->GetInstigator();

    auto NewActor = World->SpawnActor<AActor>(
        ActorToClone->GetClass(),
        NewTransform,
        SpawnParams
    );
    NewActor->Tags.Add(Tag);
    return NewActor;
}
#pragma optimize("", on)
