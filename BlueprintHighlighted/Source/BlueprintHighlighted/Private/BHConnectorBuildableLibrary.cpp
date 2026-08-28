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

	// Wires are AFGBuildable themselves, so they show up in the dismantle-mode selection alongside
	// everything else - but they only make sense once both endpoint buildables already have copies,
	// which this generic path has no way to know. DuplicateWireBetweenNewBuildables is the only thing
	// that should ever create a wire copy; spawning one here too would leave a second, connectionless
	// wire actor sitting on top of it.
	if (BuildableToCopy->IsA<AFGBuildableWire>())
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

	// KNOWN LIMITATION: mergers/splitters (including vertical variants added by other mods, e.g.
	// VerticalLogisticsQoL) are not currently supported by this copy path and are always skipped below.
	//
	// They derive their input/output roles per-connector via SetDirection(), normally called by the
	// hologram's ConfigureComponents() between spawn and FinishSpawning - a step our raw copy path
	// doesn't go through. Without it, a vertical attachment's two lift-facing connectors keep whatever
	// direction they default to (neither Input nor Output), leaving it with zero outputs, which trips a
	// hard check() in AFGBuildableConveyorAttachment::BeginPlay and crashes the game outright.
	//
	// The block below tries to replicate the hologram's effect by copying each connector's already-correct
	// direction from the original buildable (matched by component name) before BeginPlay can run - but in
	// practice this buildable's connector components don't exist yet at this point (they're only created
	// once FinishSpawning runs the construction script, which is the same call that triggers BeginPlay),
	// so the copy is always a no-op and this always falls through to the bail-out below. Left in rather
	// than deleted because it's harmless, and it's the right place to pick this back up if a way is found
	// to configure connectors before BeginPlay runs (e.g. writing AFGBuildableConveyorAttachment's
	// SaveGame mSavedDirections directly would need a new Access Transformer Friend= grant).
	if (AFGBuildableConveyorAttachment* OriginalAttachment = Cast<AFGBuildableConveyorAttachment>(BuildableToCopy))
	{
		AFGBuildableConveyorAttachment* NewAttachment = CastChecked<AFGBuildableConveyorAttachment>(NewBuildable);

		TArray<UFGFactoryConnectionComponent*> OriginalConnections;
		OriginalAttachment->GetComponents(OriginalConnections);

		bool bHasOutputConnection = false;
		for (UFGFactoryConnectionComponent* OriginalConnection : OriginalConnections)
		{
			if (UFGFactoryConnectionComponent* NewConnection = FindMatchingComponentByName(NewAttachment, OriginalConnection))
			{
				NewConnection->SetDirection(OriginalConnection->GetDirection());
				bHasOutputConnection |= NewConnection->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT;
			}
		}

		// Always taken in practice (see comment above) - bail out gracefully rather than let BeginPlay
		// crash. The caller already treats a nullptr return the same as a SpawnBuildableFromClass failure.
		if (!bHasOutputConnection)
		{
			NewBuildable->Destroy();
			return nullptr;
		}

		// SetDirection() only updates the live per-component direction. Visual/mesh state is driven from
		// mSavedDirections instead (see AFGBuildableConveyorAttachment::SaveCurrentDirections), which we
		// haven't touched yet - without this the buildable is functionally fine but renders as invisible.
		NewAttachment->SaveCurrentDirections();
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
