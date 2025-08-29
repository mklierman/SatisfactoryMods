


#include "ArachBPFL.h"

void UArachBPFL::SpawnCreatures(AFGCreatureSpawner* spawner)
{
	if (!spawner->IsNearBase())
	{
		spawner->SpawnCreatures();
		spawner->PopulateSpawnData();
	}
}

void UArachBPFL::SpawnSingleCreature(AFGCreatureSpawner* spawner)
{
	if (!spawner->IsNearBase())
	{
		spawner->SpawnSingleCreature();
		spawner->PopulateSpawnData();
	}
}
