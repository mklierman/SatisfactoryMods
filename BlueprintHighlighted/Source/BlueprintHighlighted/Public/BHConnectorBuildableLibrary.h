#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildables/FGBuildableFactory.h"
#include "BHConnectorBuildableLibrary.generated.h"

class AFGBuildable;
class AFGBuildableWire;
class UFGCircuitConnectionComponent;

/**
 * Spawns connector-type buildables (belts, pipes, hypertube, rail, lifts) that SpawnBuildableFromClass
 * cannot handle, since those need spline/connection data copied from the source, not just a Transform.
 * See conversation notes: BH_Mixin_BuildGun's SpawnNewBuildable/HandleIgnoredBuildables blocklist these
 * types entirely because of this gap.
 */
UCLASS()
class BLUEPRINTHIGHLIGHTED_API UBHConnectorBuildableLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Duplicates BuildableToCopy at Transform, correctly carrying over spline/connection data for
	 * spline buildables (belt, pipe, hypertube, rail) and using the native duplicate path for lifts.
	 * Returns nullptr on failure - caller should treat that the same as a SpawnBuildableFromClass failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueprintHighlighted", meta = (WorldContext = "WorldContextObject"))
	static AFGBuildable* SpawnConnectorBuildableCopy(
		UObject* WorldContextObject,
		AFGBuildable* BuildableToCopy,
		TSubclassOf<AFGBuildable> BuildableClass,
		const FTransform& Transform);

	/**
	 * Second-pass helper for wires: wires aren't spline buildables, they're just a connection between
	 * two other buildables' connectors, so they can only be reconstructed AFTER both endpoint buildables
	 * have already been duplicated. Call this once per original wire, passing the NEW copies of the wire's
	 * two endpoint buildables (in the same order the wire's own connectors were on the originals).
	 * Returns nullptr on failure (e.g. if the matching connector couldn't be found on the new buildable).
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueprintHighlighted", meta = (WorldContext = "WorldContextObject"))
	static AFGBuildableWire* DuplicateWireBetweenNewBuildables(
		UObject* WorldContextObject,
		AFGBuildableWire* WireToCopy,
		AFGBuildable* NewFirstBuildable,
		AFGBuildable* NewSecondBuildable);

	/**
	 * Call once per item right after SpawnConnectorBuildableCopy, inside the same loop that duplicates
	 * the whole selection. Records OriginalBuildable -> NewBuildable in OldToNewBuildables, then looks
	 * at every belt/pipe/rail connector on OriginalBuildable: if the neighbor it was connected to has
	 * ALREADY been duplicated (i.e. is already a key in the map), links the matching new connectors
	 * together. Because it checks every connection (not just "forward" ones), it doesn't matter which
	 * order the selection is processed in - each real connection gets created exactly once, whichever
	 * of its two buildables is processed second.
	 *
	 * This exists because AFGBuildableConveyorBelt/PipeBase::SetupConnections() is a no-op stub in this
	 * SDK build (empty body, does not search by proximity or anything else) - it cannot be used to wire
	 * up a spawned copy, so connections have to be reconstructed explicitly from the original topology,
	 * the same way DuplicateWireBetweenNewBuildables already does for wires.
	 */
	UFUNCTION(BlueprintCallable, Category = "BlueprintHighlighted")
	static void ReconnectSpawnedBuildable(
		AFGBuildable* OriginalBuildable,
		AFGBuildable* NewBuildable,
		UPARAM(ref) TMap<AFGBuildable*, AFGBuildable*>& OldToNewBuildables);


	UFUNCTION(BlueprintCallable, Category = "BlueprintHighlighted")
	static void SetPotential(AFGBuildableFactory* building, float newPotential);
};
