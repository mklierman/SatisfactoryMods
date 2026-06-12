#include "InfiniteZoopSubsystem.h"
#include <Net/UnrealNetwork.h>
#include <Logging/StructuredLog.h>

AInfiniteZoopSubsystem::AInfiniteZoopSubsystem()
{
    ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
	csection = new FCriticalSection();
}

void AInfiniteZoopSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInfiniteZoopSubsystem, currentZoopAmount);
}

FCriticalSection ZoopAmountsMutex;

void AInfiniteZoopSubsystem::SetPublicZoopAmount(int x, int y, int z, bool foundation, bool verticalZoop, APawn* owner)
{
    if (!IsValid(owner))
    {
        return;
    }

    checkf(IsInGameThread(), TEXT("SetPublicZoopAmount called off game thread"));

    {
        FScopeLock Guard(&ZoopAmountsMutex);

        FZoopAmountStruct& zStruct = ZoopAmountStructs.FindOrAdd(owner);
        
        zStruct.isFoundation = foundation;

        if (foundation && verticalZoop)
        {
            zStruct.xAmount = -1;
            zStruct.yAmount = -1;
            zStruct.zAmount = -1;
            zStruct.needsUpdate = true;
        }
        else if (
            x == zStruct.xAmount &&
            y == zStruct.yAmount &&
            z == zStruct.zAmount &&
            foundation == zStruct.isFoundation)
        {
            zStruct.xAmount = -1;
            zStruct.yAmount = -1;
            zStruct.zAmount = -1;
            zStruct.needsUpdate = false;
        }
        else
        {
            zStruct.xAmount = x;
            zStruct.yAmount = y;
            zStruct.zAmount = z;
            zStruct.needsUpdate = true;
        }
    }

    ForceNetUpdate();
}
