#include "SOBHologram.h"

void ASOBHologram::SetHologramLocationAndRotation(const FHitResult& hitResult)
{
    AActor* hitActor = hitResult.GetActor();
    ASOBBuildableStorage* cl = Cast <ASOBBuildableStorage>(hitActor);
    if (hitActor && cl)
    {
        FRotator addedRotation = FRotator(0, 0, 0);
        if (this->GetRotationStep() != 0)
        {
            addedRotation.Yaw = mRotationAmount;
        }
        if (hitResult.ImpactNormal.Z <= -0.75)
        {
            SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, -200), cl->GetActorRotation() + addedRotation);
            this->mSnappedBuilding = cl;
            this->CheckValidPlacement();
        }
        else if (hitResult.ImpactNormal.Z >= 0.75)
        {
            SetActorLocationAndRotation(cl->GetActorLocation() + FVector(0, 0, 200), cl->GetActorRotation() + addedRotation);
            this->mSnappedBuilding = cl;
            this->CheckValidPlacement();
        }
        else
        {
            Super::SetHologramLocationAndRotation(hitResult);
        }
    }
    else
    {
        Super::SetHologramLocationAndRotation(hitResult);
    }
}

bool ASOBHologram::IsValidHitResult(const FHitResult& hitResult) const
{
    AActor* hitActor = hitResult.GetActor();
    ASOBBuildableStorage* cl = Cast <ASOBBuildableStorage>(hitActor);
    if (hitActor && cl)
    {
        return true;
    }
    return Super::IsValidHitResult(hitResult);
}

void ASOBHologram::Scroll(int32 delta)
{
    //Super::SetScrollMode(EHologramScrollMode::HSM_ROTATE);
    if (GetSnappedBuilding() && Cast<ASOBBuildableStorage>(GetSnappedBuilding()))
    {
        auto pContr = this->GetNetOwningPlayer()->GetPlayerController(GetWorld());
        if (pContr && pContr->IsInputKeyDown(EKeys::LeftControl))
        {
            mRotationAmount += delta * 5;
            SetScrollRotateValue(GetScrollRotateValue() + delta * 5);
        }
        else
        {
            mRotationAmount += delta * 45;
            Super::Scroll(delta);
        }
    }
    else
    {
        Super::Scroll(delta);
    }
}

void ASOBHologram::ConfigureComponents(AFGBuildable* inBuildable) const
{
    Super::ConfigureComponents(inBuildable);

    if (mSnappedConveyor && inBuildable)
    {
        auto cl = Cast< ASOBBuildableStorage>(inBuildable);
        if (cl)
        {
            auto beltCustomizationData = mSnappedConveyor->Execute_GetCustomizationData(mSnappedConveyor);
            TArray< AFGBuildableConveyorBelt* > Belts = AFGBuildableConveyorBelt::Split(mSnappedConveyor, mSnappedConveyorOffset, false);
            if (Belts.Num() > 0)
            {
                for (auto Belt : Belts)
                {
                    if (Belt->GetConnection0() && !Belt->GetConnection0()->IsConnected())
                    {
                        if (Belt->GetConnection0()->GetDirection() == EFactoryConnectionDirection::FCD_INPUT && !cl->outputConnection->GetConnection())
                        {
                            Belt->GetConnection0()->SetConnection(cl->outputConnection);
                        }
                        else if (Belt->GetConnection0()->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT && !cl->inputConnection->GetConnection())
                        {
                            Belt->GetConnection0()->SetConnection(cl->inputConnection);
                        }
                    }
                    else if (Belt->GetConnection1() && !Belt->GetConnection1()->IsConnected() && !cl->outputConnection->GetConnection())
                    {
                        if (Belt->GetConnection1()->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
                        {
                            Belt->GetConnection1()->SetConnection(cl->outputConnection);
                        }
                        else if (Belt->GetConnection1()->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT && !cl->inputConnection->GetConnection())
                        {
                            Belt->GetConnection1()->SetConnection(cl->inputConnection);
                        }
                    }
                    Belt->Execute_SetCustomizationData(Belt, beltCustomizationData);
                }
            }
        }
    }
}