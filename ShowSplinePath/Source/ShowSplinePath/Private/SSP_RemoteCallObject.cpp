#include "SSP_RemoteCallObject.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "Subsystem/ModSubsystem.h"
#include "SSP_Subsystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include <Subsystem/SubsystemActorManager.h>

USSP_RemoteCallObject::USSP_RemoteCallObject() : Super()
{
    DummyReplicatedField = 0;
}

void USSP_RemoteCallObject::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USSP_RemoteCallObject, DummyReplicatedField);
}

void USSP_RemoteCallObject::Client_SetPathVisible_Implementation(AFGDrivingTargetList* targetList, bool show)
{
    if (!targetList)
    {
        return;
    }
    targetList->OnRep_IsPathVisible();
}