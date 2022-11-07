


#include "CP_Subsystem.h"

ACP_Subsystem::ACP_Subsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
}

bool ACP_Subsystem::ShouldSave_Implementation() const
{
	return true;
}