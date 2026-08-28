#include "HypertubeDestinationSubsystem.h"

#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "Buildables/FGPipeHyperStart.h"
#include "Engine/World.h"
#include "FGCharacterPlayer.h"
#include "FGPipeConnectionComponent.h"
#include "HypertubeRoutePlanner.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/SubsystemActorManager.h"

namespace
{
	constexpr int32 HypertubeDestinationMaxNameLength = 64;
	const TCHAR* HypertubeDestinationDefaultName = TEXT("Unnamed Hypertube Entrance");

	bool PrimeVanillaJunctionSelection(AFGCharacterPlayer* Player, const FHypertubeRouteDecision& Decision)
	{
		if (!IsValid(Player) || !IsValid(Decision.IncomingConnection.Get()) || !IsValid(Decision.OutgoingConnection.Get()))
		{
			return false;
		}

		Player->Server_UpdateHyperJunctionOutputConnection(Decision.IncomingConnection.Get(), Decision.OutgoingConnection.Get());
		return true;
	}
}

AHypertubeDestinationSubsystem::AHypertubeDestinationSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
}

void AHypertubeDestinationSubsystem::Init()
{
	Super::Init();
}

AHypertubeDestinationSubsystem* AHypertubeDestinationSubsystem::Get(const UObject* WorldContext)
{
	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld()))
	{
		return nullptr;
	}

	if (USubsystemActorManager* Manager = WorldContext->GetWorld()->GetSubsystem<USubsystemActorManager>())
	{
		return Manager->GetSubsystemActor<AHypertubeDestinationSubsystem>();
	}

	return nullptr;
}

AHypertubeDestinationSubsystem* AHypertubeDestinationSubsystem::GetHypertubeDestinationSubsystem(const UObject* WorldContext)
{
	return Get(WorldContext);
}

FGuid AHypertubeDestinationSubsystem::RegisterEntrance(AFGPipeHyperStart* Entrance)
{
	if (!HasAuthority() || !IsValid(Entrance))
	{
		return {};
	}

	if (FHypertubeDestinationRecord* Existing = FindMutableDestination(Entrance))
	{
		return Existing->DestinationId;
	}

	FHypertubeDestinationRecord& Record = Destinations.AddDefaulted_GetRef();
	Record.DestinationId = FGuid::NewGuid();
	Record.DisplayName = HypertubeDestinationDefaultName;
	Record.Entrance = Entrance;
	ForceNetUpdate();
	OnDestinationsChanged.Broadcast();
	return Record.DestinationId;
}

void AHypertubeDestinationSubsystem::MarkEntranceUnavailable(AFGPipeHyperStart* Entrance)
{
	if (!HasAuthority() || !IsValid(Entrance))
	{
		return;
	}

	++TopologyRevision;
	ActiveRoutes.Empty();
}

bool AHypertubeDestinationSubsystem::RemoveEntrance(AFGPipeHyperStart* Entrance)
{
	if (!HasAuthority() || !IsValid(Entrance))
	{
		return false;
	}

	const int32 RemovedCount = Destinations.RemoveAll([Entrance](const FHypertubeDestinationRecord& Record)
	{
		return Record.Entrance == Entrance;
	});

	if (RemovedCount > 0)
	{
		++TopologyRevision;
		ActiveRoutes.Empty();
		ForceNetUpdate();
		OnDestinationsChanged.Broadcast();
	}
	return RemovedCount > 0;
}

bool AHypertubeDestinationSubsystem::SetDestinationName(AFGPipeHyperStart* Entrance, const FString& NewName)
{
	if (!HasAuthority() || !IsValid(Entrance))
	{
		return false;
	}

	FString SanitizedName = NewName.TrimStartAndEnd().Left(HypertubeDestinationMaxNameLength);
	if (SanitizedName.IsEmpty())
	{
		return false;
	}

	FHypertubeDestinationRecord* Record = FindMutableDestination(Entrance);
	if (Record == nullptr)
	{
		RegisterEntrance(Entrance);
		Record = FindMutableDestination(Entrance);
	}

	if (Record == nullptr || Record->DisplayName == SanitizedName)
	{
		return Record != nullptr;
	}

	Record->DisplayName = MoveTemp(SanitizedName);
	ForceNetUpdate();
	OnDestinationsChanged.Broadcast();
	return true;
}

TArray<FHypertubeDestinationRecord> AHypertubeDestinationSubsystem::GetAvailableDestinations() const
{
	TArray<FHypertubeDestinationRecord> Result;
	for (const FHypertubeDestinationRecord& Record : Destinations)
	{
		if (Record.DestinationId.IsValid() && IsValid(Record.Entrance.Get()))
		{
			Result.Add(Record);
		}
	}

	Result.Sort([](const FHypertubeDestinationRecord& Left, const FHypertubeDestinationRecord& Right)
	{
		return Left.DisplayName < Right.DisplayName;
	});
	return Result;
}

TArray<FHypertubeDestinationRecord> AHypertubeDestinationSubsystem::GetReachableDestinations(AFGCharacterPlayer* Player, AFGPipeHyperStart* Source) const
{
	TArray<FHypertubeDestinationRecord> Result;
	if (!IsValid(Player) || !IsValid(Source))
	{
		return Result;
	}

	for (const FHypertubeDestinationRecord& Record : Destinations)
	{
		AFGPipeHyperStart* Destination = Record.Entrance.Get();
		if (!Record.DestinationId.IsValid() || !IsValid(Destination) || Destination == Source)
		{
			continue;
		}

		FHypertubeRoutePlan Route;
		if (UHypertubeRoutePlanner::FindRoute(Player, Source, Destination, Route))
		{
			Result.Add(Record);
		}
	}

	Result.Sort([](const FHypertubeDestinationRecord& Left, const FHypertubeDestinationRecord& Right)
	{
		return Left.DisplayName < Right.DisplayName;
	});
	return Result;
}

bool AHypertubeDestinationSubsystem::GetDestinationForEntrance(AFGPipeHyperStart* Entrance, FHypertubeDestinationRecord& OutDestination) const
{
	if (const FHypertubeDestinationRecord* Record = FindDestination(Entrance))
	{
		OutDestination = *Record;
		return true;
	}
	return false;
}

bool AHypertubeDestinationSubsystem::SelectDestination(AFGCharacterPlayer* Player, AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination)
{
	if (!HasAuthority() || !IsValid(Player))
	{
		return false;
	}

	const FHypertubeDestinationRecord* SourceRecord = FindDestination(Source);
	const FHypertubeDestinationRecord* DestinationRecord = FindDestination(Destination);
	if (SourceRecord == nullptr || DestinationRecord == nullptr)
	{
		return false;
	}

	FHypertubeRoutePlan Route;
	if (!UHypertubeRoutePlanner::FindRoute(Player, Source, Destination, Route))
	{
		return false;
	}

	Route.SourceId = SourceRecord->DestinationId;
	Route.DestinationId = DestinationRecord->DestinationId;
	Route.TopologyRevision = TopologyRevision;
	FHypertubeRoutePlan& StoredRoute = ActiveRoutes.FindOrAdd(Player);
	StoredRoute = MoveTemp(Route);

	if (StoredRoute.Decisions.IsValidIndex(StoredRoute.CurrentDecisionIndex))
	{
		PrimeVanillaJunctionSelection(Player, StoredRoute.Decisions[StoredRoute.CurrentDecisionIndex]);
	}
	return true;
}

bool AHypertubeDestinationSubsystem::PrepareJunctionRoute(AFGCharacterPlayer* Player, AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* IncomingConnection,
														  UFGPipeConnectionComponentBase*& OutgoingConnection)
{
	OutgoingConnection = nullptr;
	if (!HasAuthority() || !IsValid(Player) || !IsValid(Junction) || !IsValid(IncomingConnection))
	{
		return false;
	}

	FHypertubeRoutePlan* Route = ActiveRoutes.Find(Player);
	if (Route == nullptr)
	{
		return false;
	}

	if (Route->TopologyRevision != TopologyRevision || !Route->Decisions.IsValidIndex(Route->CurrentDecisionIndex))
	{
		ActiveRoutes.Remove(Player);
		return false;
	}

	const FHypertubeRouteDecision& Decision = Route->Decisions[Route->CurrentDecisionIndex];
	if (Decision.Junction != Junction || Decision.IncomingConnection != IncomingConnection || !IsValid(Decision.OutgoingConnection.Get()))
	{
		if (Route->LastRoutedJunction.Get() == Junction)
		{
			ActiveRoutes.Remove(Player);
		}
		return false;
	}

	OutgoingConnection = Decision.OutgoingConnection.Get();
	return true;
}

bool AHypertubeDestinationSubsystem::TryRouteJunctionExit(AFGCharacterPlayer* Player, AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* VanillaExit, UFGPipeConnectionComponentBase*& OutExit)
{
	OutExit = VanillaExit;
	if (!HasAuthority() || !IsValid(Player) || !IsValid(Junction))
	{
		return false;
	}

	FHypertubeRoutePlan* Route = ActiveRoutes.Find(Player);
	if (Route == nullptr)
	{
		return false;
	}

	if (Route->LastRoutedJunction.Get() == Junction)
	{
		if (IsValid(Route->LastRoutedExit.Get()))
		{
			OutExit = Route->LastRoutedExit.Get();
			return true;
		}

		ActiveRoutes.Remove(Player);
		return false;
	}

	if (Route->TopologyRevision != TopologyRevision || !Route->Decisions.IsValidIndex(Route->CurrentDecisionIndex))
	{
		ActiveRoutes.Remove(Player);
		return false;
	}

	const FHypertubeRouteDecision& Decision = Route->Decisions[Route->CurrentDecisionIndex];
	if (Decision.Junction != Junction)
	{
		ActiveRoutes.Remove(Player);
		return false;
	}

	if (!IsValid(Decision.OutgoingConnection.Get()))
	{
		ActiveRoutes.Remove(Player);
		return false;
	}

	OutExit = Decision.OutgoingConnection;
	Route->LastRoutedJunction = Junction;
	Route->LastRoutedExit = OutExit;
	++Route->CurrentDecisionIndex;
	if (Route->Decisions.IsValidIndex(Route->CurrentDecisionIndex))
	{
		PrimeVanillaJunctionSelection(Player, Route->Decisions[Route->CurrentDecisionIndex]);
	}
	return true;
}

const FHypertubeRoutePlan* AHypertubeDestinationSubsystem::GetActiveRoute(AFGCharacterPlayer* Player) const
{
	return ActiveRoutes.Find(Player);
}

void AHypertubeDestinationSubsystem::ClearActiveRoute(AFGCharacterPlayer* Player)
{
	ActiveRoutes.Remove(Player);
}

void AHypertubeDestinationSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHypertubeDestinationSubsystem, Destinations);
}

void AHypertubeDestinationSubsystem::PostLoadGame_Implementation(int32, int32)
{
	Destinations.RemoveAll([](const FHypertubeDestinationRecord& Record)
	{
		return !Record.DestinationId.IsValid();
	});
}

void AHypertubeDestinationSubsystem::GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects)
{
	for (const FHypertubeDestinationRecord& Record : Destinations)
	{
		if (IsValid(Record.Entrance.Get()))
		{
			OutDependentObjects.Add(Record.Entrance);
		}
	}
}

bool AHypertubeDestinationSubsystem::ShouldSave_Implementation() const
{
	return HasAuthority();
}

bool AHypertubeDestinationSubsystem::NeedTransform_Implementation()
{
	return false;
}

void AHypertubeDestinationSubsystem::OnRep_Destinations()
{
	OnDestinationsChanged.Broadcast();
}

FHypertubeDestinationRecord* AHypertubeDestinationSubsystem::FindMutableDestination(AFGPipeHyperStart* Entrance)
{
	return Destinations.FindByPredicate([Entrance](const FHypertubeDestinationRecord& Record)
	{
		return Record.Entrance == Entrance;
	});
}

const FHypertubeDestinationRecord* AHypertubeDestinationSubsystem::FindDestination(AFGPipeHyperStart* Entrance) const
{
	return Destinations.FindByPredicate([Entrance](const FHypertubeDestinationRecord& Record)
	{
		return Record.Entrance == Entrance;
	});
}
