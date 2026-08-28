#include "BHConnectorBuildableLibrary.h"

#include "FGBuildableSubsystem.h"
#include "FGSplineBuildableInterface.h"
#include "FGCircuitConnectionComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "Buildables/FGBuildableConveyorLift.h"
#include "Buildables/FGBuildableWire.h"

namespace
{
	// Components are created from the same construction script when spawning from the same class,
	// so matching by component name finds the equivalent connector on a freshly duplicated buildable.
	template<typename TConnection>
	TConnection* FindMatchingComponentByName(AFGBuildable* NewBuildable, TConnection* OriginalComponent)
	{
		if (!NewBuildable || !OriginalComponent)
		{
			return nullptr;
		}

		const FName TargetName = OriginalComponent->GetFName();

		TArray<TConnection*> Candidates;
		NewBuildable->GetComponents<TConnection>(Candidates);

		for (TConnection* Candidate : Candidates)
		{
			if (Candidate->GetFName() == TargetName)
			{
				return Candidate;
			}
		}

		// Fall back to the first free connector of the same class if the name didn't match -
		// better than failing outright when a subclass renames its components.
		return Candidates.Num() > 0 ? Candidates[0] : nullptr;
	}

	UFGCircuitConnectionComponent* FindMatchingCircuitConnection(AFGBuildable* NewBuildable, UFGCircuitConnectionComponent* OriginalConnection)
	{
		return FindMatchingComponentByName(NewBuildable, OriginalConnection);
	}
}

AFGBuildable* UBHConnectorBuildableLibrary::SpawnConnectorBuildableCopy(
	UObject* WorldContextObject,
	AFGBuildable* BuildableToCopy,
	TSubclassOf<AFGBuildable> BuildableClass,
	const FTransform& Transform)
{
	if (!BuildableToCopy || !BuildableClass)
	{
		return nullptr;
	}

	// Lifts already ship a native "duplicate this exact instance, including its connections" function.
	if (AFGBuildableConveyorLift* LiftToCopy = Cast<AFGBuildableConveyorLift>(BuildableToCopy))
	{
		return AFGBuildableConveyorLift::DuplicateLift(LiftToCopy, /*dismantleOriginalLift=*/false);
	}

	// Mergers/splitters populate their input/output connector caches from hologram-side setup
	// (AFGBuildableHologram::ConfigureActor/ConfigureComponents) that runs between spawn and BeginPlay
	// during normal placement. Spawning them here skips that setup entirely, and for some variants
	// (e.g. vertical mergers built on a lift) that leaves zero output connectors, which trips a hard
	// check() in AFGBuildableAttachmentMerger::BeginPlay and crashes the game outright - not a
	// recoverable failure, so it must never be reached. Bail out before FinishSpawning instead.
	if (BuildableToCopy->IsA<AFGBuildableConveyorAttachment>())
	{
		return nullptr;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContextObject);
	if (!BuildableSubsystem)
	{
		return nullptr;
	}

	AFGBuildable* NewBuildable = BuildableSubsystem->BeginSpawnBuildable(BuildableClass, Transform);
	if (!NewBuildable)
	{
		return nullptr;
	}

	// Belts/pipes/hypertube/rail: copy the spline data before FinishSpawning (BeginPlay flushes it into
	// the spline component and it must not be touched afterwards - see FGSplineBuildableInterface.h).
	IFGSplineBuildableInterface* SourceSpline = Cast<IFGSplineBuildableInterface>(BuildableToCopy);
	IFGSplineBuildableInterface* NewSpline = Cast<IFGSplineBuildableInterface>(NewBuildable);
	if (SourceSpline && NewSpline)
	{
		if (TArray<FSplinePointData>* MutableSplineData = NewSpline->GetMutableSplinePointData())
		{
			*MutableSplineData = SourceSpline->GetSplinePointData();
		}
	}

	NewBuildable->FinishSpawning(Transform);

	// This is the same hook the blueprint subsystem calls after loading a saved blueprint - it
	// cleans up any stale connection references left over from spawning/serialization. It does
	// NOT create new connections; that happens separately in ReconnectSpawnedBuildable once all
	// buildables in the selection have been duplicated.
	NewBuildable->PostSerializedFromBlueprint(/*isBlueprintWorld=*/true);

	return NewBuildable;
}

AFGBuildableWire* UBHConnectorBuildableLibrary::DuplicateWireBetweenNewBuildables(
	UObject* WorldContextObject,
	AFGBuildableWire* WireToCopy,
	AFGBuildable* NewFirstBuildable,
	AFGBuildable* NewSecondBuildable)
{
	if (!WireToCopy || !NewFirstBuildable || !NewSecondBuildable)
	{
		return nullptr;
	}

	UFGCircuitConnectionComponent* OriginalConnection0 = WireToCopy->GetConnection(0);
	UFGCircuitConnectionComponent* OriginalConnection1 = WireToCopy->GetConnection(1);
	if (!OriginalConnection0 || !OriginalConnection1)
	{
		return nullptr;
	}

	UFGCircuitConnectionComponent* NewConnection0 = FindMatchingCircuitConnection(NewFirstBuildable, OriginalConnection0);
	UFGCircuitConnectionComponent* NewConnection1 = FindMatchingCircuitConnection(NewSecondBuildable, OriginalConnection1);
	if (!NewConnection0 || !NewConnection1)
	{
		return nullptr;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContextObject);
	if (!BuildableSubsystem)
	{
		return nullptr;
	}

	AFGBuildable* NewWireBuildable = BuildableSubsystem->BeginSpawnBuildable(WireToCopy->GetClass(), WireToCopy->GetActorTransform());
	AFGBuildableWire* NewWire = Cast<AFGBuildableWire>(NewWireBuildable);
	if (!NewWire)
	{
		return nullptr;
	}

	NewWire->FinishSpawning(WireToCopy->GetActorTransform());
	NewWire->Connect(NewConnection0, NewConnection1);

	return NewWire;
}

void UBHConnectorBuildableLibrary::ReconnectSpawnedBuildable(
	AFGBuildable* OriginalBuildable,
	AFGBuildable* NewBuildable,
	TMap<AFGBuildable*, AFGBuildable*>& OldToNewBuildables)
{
	if (!OriginalBuildable || !NewBuildable)
	{
		return;
	}

	OldToNewBuildables.Add(OriginalBuildable, NewBuildable);

	// Belts (factory connections). Checking every connector - not just ones pointing at
	// already-processed neighbors - means each real connection gets made exactly once,
	// whichever of its two buildables happens to be duplicated second.
	TArray<UFGFactoryConnectionComponent*> FactoryConnections;
	OriginalBuildable->GetComponents<UFGFactoryConnectionComponent>(FactoryConnections);
	for (UFGFactoryConnectionComponent* OriginalConnection : FactoryConnections)
	{
		if (!OriginalConnection->IsConnected())
		{
			continue;
		}

		UFGFactoryConnectionComponent* OriginalNeighborConnection = OriginalConnection->GetConnection();
		AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
		AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
		if (!NewNeighbor)
		{
			continue;
		}

		UFGFactoryConnectionComponent* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
		UFGFactoryConnectionComponent* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
		if (NewConnection && NewNeighborConnection && !NewConnection->IsConnected() && !NewNeighborConnection->IsConnected())
		{
			NewConnection->SetConnection(NewNeighborConnection);
		}
	}

	// Pipes and hypertubes (both derive from UFGPipeConnectionComponentBase).
	TArray<UFGPipeConnectionComponentBase*> PipeConnections;
	OriginalBuildable->GetComponents<UFGPipeConnectionComponentBase>(PipeConnections);
	for (UFGPipeConnectionComponentBase* OriginalConnection : PipeConnections)
	{
		if (!OriginalConnection->IsConnected())
		{
			continue;
		}

		UFGPipeConnectionComponentBase* OriginalNeighborConnection = OriginalConnection->GetConnection();
		AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
		AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
		if (!NewNeighbor)
		{
			continue;
		}

		UFGPipeConnectionComponentBase* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
		UFGPipeConnectionComponentBase* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
		if (NewConnection && NewNeighborConnection && !NewConnection->IsConnected() && !NewNeighborConnection->IsConnected())
		{
			NewConnection->SetConnection(NewNeighborConnection);
		}
	}

	// Rail. Track connections can hold more than one link (switches), so walk all of them.
	TArray<UFGRailroadTrackConnectionComponent*> RailConnections;
	OriginalBuildable->GetComponents<UFGRailroadTrackConnectionComponent>(RailConnections);
	for (UFGRailroadTrackConnectionComponent* OriginalConnection : RailConnections)
	{
		for (UFGRailroadTrackConnectionComponent* OriginalNeighborConnection : OriginalConnection->GetConnections())
		{
			if (!OriginalNeighborConnection)
			{
				continue;
			}

			AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
			AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
			if (!NewNeighbor)
			{
				continue;
			}

			UFGRailroadTrackConnectionComponent* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
			UFGRailroadTrackConnectionComponent* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
			if (NewConnection && NewNeighborConnection && !NewConnection->GetConnections().Contains(NewNeighborConnection))
			{
				NewConnection->AddConnection(NewNeighborConnection);
			}
		}
	}
}
