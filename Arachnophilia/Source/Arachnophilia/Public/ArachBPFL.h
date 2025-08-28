

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Creature/FGCreatureSpawner.h"
#include "ArachBPFL.generated.h"

/**
 * 
 */
UCLASS()
class ARACHNOPHILIA_API UArachBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static void SpawnCreatures(AFGCreatureSpawner* spawner);

	UFUNCTION(BlueprintCallable)
	static void SpawnSingleCreature(AFGCreatureSpawner* spawner);
};
