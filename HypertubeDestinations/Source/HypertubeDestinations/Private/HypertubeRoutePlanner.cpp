#include "HypertubeRoutePlanner.h"

#include "Algo/Reverse.h"
#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "Buildables/FGPipeHyperStart.h"
#include "FGPipeConnectionComponent.h"
#include "FGPipeHyperInterface.h"

namespace
{
struct FSearchNode
{
	TObjectPtr<UFGPipeConnectionComponentBase> EnteredConnection = nullptr;
	int32 ParentIndex = INDEX_NONE;
	FHypertubeRouteDecision Decision;
};

bool IsDestinationConnection(const UFGPipeConnectionComponentBase* Connection, const AFGPipeHyperStart* Destination)
{
	return IsValid(Connection) && IsValid(Destination) && (Connection == Destination->mConnection0 || Connection->GetOwner() == Destination);
}
}

bool UHypertubeRoutePlanner::FindRoute(AFGCharacterPlayer* Player, AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination, FHypertubeRoutePlan& OutRoute)
{
	OutRoute = {};

	if (!Player || !IsValid(Source) || !IsValid(Destination) || Source == Destination || !IsValid(Source->mConnection0.Get()) || !IsValid(Destination->mConnection0.Get()))
	{
		return false;
	}

	UFGPipeConnectionComponentBase* FirstConnection = Source->mConnection0->GetConnection();
	if (!IsValid(FirstConnection))
	{
		return false;
	}

	TArray<FSearchNode> SearchNodes;
	SearchNodes.Reserve(64);
	SearchNodes.Add({FirstConnection, INDEX_NONE, {}});

	TSet<TObjectPtr<UFGPipeConnectionComponentBase>> Visited;
	Visited.Add(FirstConnection);

	int32 FoundNodeIndex = INDEX_NONE;
	for (int32 QueueIndex = 0; QueueIndex < SearchNodes.Num() && FoundNodeIndex == INDEX_NONE; ++QueueIndex)
	{
		const FSearchNode& CurrentNode = SearchNodes[QueueIndex];
		UFGPipeConnectionComponentBase* EnteredConnection = CurrentNode.EnteredConnection;
		AActor* PipeActor = nullptr;
		if (IsValid(EnteredConnection))
		{
			PipeActor = EnteredConnection->GetOwner();
		}
		IFGPipeHyperInterface* HyperInterface = Cast<IFGPipeHyperInterface>(PipeActor);
		if (HyperInterface == nullptr)
		{
			continue;
		}

		const TArray<TPair<UFGPipeConnectionComponentBase*, float>> PossibleConnections = HyperInterface->GetPossibleConnectionsToTransitionThrough(Player, EnteredConnection);

		for (const TPair<UFGPipeConnectionComponentBase*, float>& PossibleConnection : PossibleConnections)
		{
			UFGPipeConnectionComponentBase* OutgoingConnection = PossibleConnection.Key;
			if (!IsValid(OutgoingConnection))
			{
				continue;
			}

			UFGPipeConnectionComponentBase* NextEnteredConnection = OutgoingConnection->GetConnection();
			if (IsDestinationConnection(NextEnteredConnection, Destination))
			{
				FSearchNode DestinationNode;
				DestinationNode.EnteredConnection = NextEnteredConnection;
				DestinationNode.ParentIndex = QueueIndex;
				if (AFGBuildablePipeHyperJunction* Junction = Cast<AFGBuildablePipeHyperJunction>(PipeActor))
				{
					DestinationNode.Decision = {Junction, EnteredConnection, OutgoingConnection};
				}
				FoundNodeIndex = SearchNodes.Add(MoveTemp(DestinationNode));
				break;
			}

			if (!IsValid(NextEnteredConnection) || Visited.Contains(NextEnteredConnection))
			{
				continue;
			}

			Visited.Add(NextEnteredConnection);
			FSearchNode NextNode;
			NextNode.EnteredConnection = NextEnteredConnection;
			NextNode.ParentIndex = QueueIndex;
			if (AFGBuildablePipeHyperJunction* Junction = Cast<AFGBuildablePipeHyperJunction>(PipeActor))
			{
				NextNode.Decision = {Junction, EnteredConnection, OutgoingConnection};
			}
			SearchNodes.Add(MoveTemp(NextNode));
		}
	}

	if (FoundNodeIndex == INDEX_NONE)
	{
		return false;
	}

	TArray<FHypertubeRouteDecision> ReverseDecisions;
	for (int32 NodeIndex = FoundNodeIndex; NodeIndex != INDEX_NONE; NodeIndex = SearchNodes[NodeIndex].ParentIndex)
	{
		const FHypertubeRouteDecision& Decision = SearchNodes[NodeIndex].Decision;
		if (IsValid(Decision.Junction.Get()))
		{
			ReverseDecisions.Add(Decision);
		}
	}

	Algo::Reverse(ReverseDecisions);
	OutRoute.Decisions = MoveTemp(ReverseDecisions);
	return true;
}
