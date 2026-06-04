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

