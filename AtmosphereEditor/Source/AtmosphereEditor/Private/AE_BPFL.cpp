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

    const bool bWasReplicated = ActorToClone->GetIsReplicated();
    ActorToClone->SetReplicates(true);

    FActorSpawnParameters SpawnParams;

    SpawnParams.Template = ActorToClone;

    SpawnParams.Owner = ActorToClone->GetOwner();
    SpawnParams.Instigator = ActorToClone->GetInstigator();

    auto NewActor = World->SpawnActor<AActor>(
        ActorToClone->GetClass(),
        NewTransform,
        SpawnParams
    );

    ActorToClone->SetReplicates(bWasReplicated);

    NewActor->Tags.Add(Tag);

    return NewActor;
}

AActor* UAE_BPFL::CloneActorLocal(UObject* WorldContextObject, AActor* ActorToClone, FTransform NewTransform, FName Tag)
{
    if (!ActorToClone || !WorldContextObject) return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;

    AActor* NewActor = NewObject<AActor>(World, ActorToClone->GetClass(),
        NAME_None, RF_NoFlags, ActorToClone);

    if (!NewActor) return nullptr;

    NewActor->SetActorTransform(NewTransform);
    NewActor->SetReplicates(false);
    NewActor->Tags.Add(Tag);
    //NewActor->RegisterAllComponents();
    //NewActor->PostInitializeComponents();
    NewActor->DispatchBeginPlay();

    return NewActor;
}

UObject* UAE_BPFL::GetCDO_NoLog(UClass* Class)
{
    if (IsValid(Class))
    {
        return Class->GetDefaultObject();
    }
    return nullptr;
}
#pragma optimize("", on)
