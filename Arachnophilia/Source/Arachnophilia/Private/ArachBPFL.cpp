


#include "ArachBPFL.h"

void UArachBPFL::SpawnCreatures(AFGCreatureSpawner* spawner)
{
	spawner->SpawnCreatures();
	spawner->PopulateSpawnData();
}

void UArachBPFL::SpawnSingleCreature(AFGCreatureSpawner* spawner)
{
	spawner->SpawnSingleCreature();
	spawner->PopulateSpawnData();
}
