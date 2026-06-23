


#include "ParadiseIsland_Subsystem.h"


AParadiseIsland_Subsystem::AParadiseIsland_Subsystem()
{
    ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer;
}

bool AParadiseIsland_Subsystem::ShouldSave_Implementation() const
{
    return true;
}


