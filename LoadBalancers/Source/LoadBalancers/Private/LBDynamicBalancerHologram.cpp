

#include "LBDynamicBalancerHologram.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGClearanceComponent.h"

void ALBDynamicBalancerHologram::BeginPlay()
{
    //auto comp = Cast<UStaticMeshComponent>(this->GetComponentByClass(UStaticMeshComponent::StaticClass()));
    //if (comp)
    //{
    //    savedStaticMesh = comp->GetStaticMesh();
    //}
    Super::BeginPlay();
}

bool ALBDynamicBalancerHologram::DoMultiStepPlacement(bool isInputFromARelease)
{
    if (mBuildStep == EDynamicBalancerHologramBuildStep::DBHBS_PlacementAndRotation)
    {
        mBuildStep = EDynamicBalancerHologramBuildStep::DBHBS_AdjustLength;
        return false;
    }
    else
    {
        return true;
    }
}

void ALBDynamicBalancerHologram::SetHologramLocationAndRotation(const FHitResult& hitResult)
{
    if (mBuildStep == EDynamicBalancerHologramBuildStep::DBHBS_PlacementAndRotation)
    {
        Super::SetHologramLocationAndRotation(hitResult);
    }
}

void ALBDynamicBalancerHologram::ScrollRotate(int32 delta, int32 step)
{
    if (mBuildStep == EDynamicBalancerHologramBuildStep::DBHBS_AdjustLength)
    {
        if (delta == 1)
        {
            additionalChunks += delta;
            FVector newLocation = FVector(0, (200 * additionalChunks * delta) + 200, -50);
            FQuat newRotation = FQuat(0, 0, 180, 0);

            UFGColoredInstanceMeshProxy* newComponent = NewObject<UFGColoredInstanceMeshProxy>(this, UFGColoredInstanceMeshProxy::StaticClass());
            //Try using hologram children instead?
            //auto newHologram = this->SpawnChildHologramFromRecipe(this, savedRecipe, this, newLocation);

            //newHologram->SetActorRotation(newRotation);

            if (newComponent->AttachTo(this->GetRootComponent(), NAME_None))
            {
                newComponent->SetRelativeLocationAndRotation(newLocation, newRotation);
                /*auto comps = this->GetComponentsByClass(UFGColoredInstanceMeshProxy::StaticClass());
                for (auto c : comps)
                {

                    if (c->GetName() == "FGColoredInstanceMeshProxy")
                    {
                        auto ism = Cast<UFGColoredInstanceMeshProxy>(c);
                        if (ism)
                        {
                            savedStaticMesh = ism->GetStaticMesh();
                        }
                    }
                }*/
                newComponent->SetStaticMesh(savedStaticMesh);
                newComponent->ComponentTags.Add("HologramMesh");
                newComponent->RegisterComponent();
                //newComponent->OnComponentCreated();
                AddedComponents.Add(newComponent);

            }

        }
        else if (delta == -1 && AddedComponents.Num() > 0)
        {
            additionalChunks += delta;
            auto latestComponent = AddedComponents.Last();
            latestComponent->UnregisterComponent();
            //this->RemoveOwnedComponent(latestComponent);
            AddedComponents.RemoveSingle(latestComponent);
        }
    }
    else
    {
        Super::ScrollRotate(delta, step);
    }
}

int32 ALBDynamicBalancerHologram::GetRotationStep() const
{
    return Super::GetRotationStep();
}

void ALBDynamicBalancerHologram::ConfigureComponents(AFGBuildable* inBuildable) const
{
    //if (inBuildable)
    //{
    //    for (int i = 1; i <= additionalChunks; i++)
    //    {
    //        UFGColoredInstanceMeshProxy* newComponent = NewObject<UFGColoredInstanceMeshProxy>(inBuildable, UFGColoredInstanceMeshProxy::StaticClass());

    //        if (newComponent->AttachTo(inBuildable->GetRootComponent(), NAME_None))
    //        {
    //            FVector newLocation = FVector(0, (200 * i) + 200, -50);
    //            FQuat newRotation = FQuat(0, 0, 180, 0);

    //            newComponent->SetRelativeLocationAndRotation(newLocation, newRotation);
    //            newComponent->SetStaticMesh(savedStaticMesh);
    //            newComponent->RegisterComponent();

    //            //Add inputs and outputs

    //            UFGFactoryConnectionComponent* newInput = NewObject<UFGFactoryConnectionComponent>(inBuildable, UFGFactoryConnectionComponent::StaticClass());

    //            if (newInput->AttachTo(inBuildable->GetRootComponent(), NAME_None))
    //            {
    //                newInput->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
    //                newInput->SetConnectorClearance(100);
    //                newInput->SetForwardPeekAndGrabToBuildable(true);
    //                newInput->SetConnector(EFactoryConnectionConnector::FCC_CONVEYOR);
    //                FVector inputLoc = FVector(100, (200 * i) + 200, 0);
    //                FQuat inputRot = FQuat(0, 0, 0, 0);
    //                newInput->SetRelativeLocationAndRotation(inputLoc, inputRot);
    //                newInput->RegisterComponent();
    //            }

    //            UFGFactoryConnectionComponent* newOutput = NewObject<UFGFactoryConnectionComponent>(inBuildable, UFGFactoryConnectionComponent::StaticClass());

    //            if (newOutput->AttachTo(inBuildable->GetRootComponent(), NAME_None))
    //            {
    //                newOutput->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
    //                newOutput->SetConnectorClearance(100);
    //                newOutput->SetForwardPeekAndGrabToBuildable(true);
    //                newOutput->SetConnector(EFactoryConnectionConnector::FCC_CONVEYOR);
    //                FVector outputLoc = FVector(-100, (200 * i) + 200, 0);
    //                FQuat outputRot = FQuat(0, 0, 180, 0);
    //                newOutput->SetRelativeLocationAndRotation(outputLoc, outputRot);
    //                newOutput->RegisterComponent();
    //            }

    //            //Expand clearance box
    //            auto clearanceComp = Cast<UFGClearanceComponent>(this->GetComponentByClass(UFGClearanceComponent::StaticClass()));
    //            if (clearanceComp)
    //            {
    //                auto newLoc = clearanceComp->GetComponentLocation();
    //                newLoc.Y += 100;
    //                clearanceComp->SetRelativeLocation(newLoc);
    //                auto boxExt = clearanceComp->GetUnscaledBoxExtent();
    //                clearanceComp->SetBoxExtent(FVector(boxExt.X, boxExt.Y += 100, boxExt.Z));
    //            }
    //        }

    //    }

    //}
    Super::ConfigureComponents(inBuildable);

}

void ALBDynamicBalancerHologram::ConfigureActor(AFGBuildable* inBuildable) const
{
    int balancerSize = additionalChunks + 1;
}
