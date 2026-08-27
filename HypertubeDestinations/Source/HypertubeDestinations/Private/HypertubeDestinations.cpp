// Copyright Epic Games, Inc. All Rights Reserved.

#include "HypertubeDestinations.h"

#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "Buildables/FGPipeHyperStart.h"
#include "FGCharacterMovementComponent.h"
#include "FGCharacterPlayer.h"
#include "FGPipeConnectionComponent.h"
#include "HypertubeDestinationSubsystem.h"
#include "Patching/NativeHookManager.h"
#include "UObject/UnrealType.h"

#define LOCTEXT_NAMESPACE "FHypertubeDestinationsModule"

namespace
{
struct FPendingJunctionTransit
{
	TWeakObjectPtr<AFGBuildablePipeHyperJunction> Junction;
	TWeakObjectPtr<UFGPipeConnectionComponentBase> IncomingConnection;
	TWeakObjectPtr<UFGPipeConnectionComponentBase> OutgoingConnection;
	bool bReversePath = false;
	bool bRouteExitConsumed = false;
};

TMap<TWeakObjectPtr<AFGCharacterPlayer>, FPendingJunctionTransit> PendingJunctionTransits;

bool ForceJunctionTravelRoute(AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* IncomingConnection, UFGPipeConnectionComponentBase* OutgoingConnection, FFGDynamicStruct& PipeData,
							  bool& OutReversePath)
{
	OutReversePath = false;

	if (!IsValid(Junction) || !IsValid(IncomingConnection) || !IsValid(OutgoingConnection))
	{
		return false;
	}

	FArrayProperty* PathsProperty = FindFProperty<FArrayProperty>(AFGBuildablePipeHyperJunction::StaticClass(), TEXT("JunctionPaths"));
	if (PathsProperty == nullptr || CastField<FStructProperty>(PathsProperty->Inner) == nullptr)
	{
		return false;
	}

	void* PathsValue = PathsProperty->ContainerPtrToValuePtr<void>(Junction);
	FScriptArrayHelper Paths(PathsProperty, PathsValue);
	for (int32 PathIndex = 0; PathIndex < Paths.Num(); ++PathIndex)
	{
		const FFGPipeHyperJunctionPath* Path = reinterpret_cast<const FFGPipeHyperJunctionPath*>(Paths.GetRawPtr(PathIndex));
		const bool bMatchesForward = Path->Connection0 == IncomingConnection && Path->Connection1 == OutgoingConnection;
		const bool bMatchesReverse = Path->Connection1 == IncomingConnection && Path->Connection0 == OutgoingConnection;
		if (!bMatchesForward && !bMatchesReverse)
		{
			continue;
		}

		if (FFGPipeHyperJunctionPipeData* JunctionPipeData = PipeData.GetValuePtr<FFGPipeHyperJunctionPipeData>())
		{
			JunctionPipeData->TravelRouteIndex = PathIndex;
			OutReversePath = bMatchesReverse;
			return true;
		}
		return false;
	}

	return false;
}

}

void FHypertubeDestinationsModule::StartupModule()
{
#if !WITH_EDITOR
	auto junctionCDO = GetMutableDefault<AFGBuildablePipeHyperJunction>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildablePipeHyperJunction::OnPipeEnterReal, junctionCDO, [](auto& scope, AFGBuildablePipeHyperJunction* hookSelf, AFGCharacterPlayer* player, UFGPipeConnectionComponentBase* incomingConnection, FFGDynamicStruct& outPipeData, const FFGDynamicStruct& predictionPipeData) {
		AFGBuildablePipeHyperJunction* junction = nullptr;
		if (IsValid(incomingConnection))
		{
			junction = Cast<AFGBuildablePipeHyperJunction>(incomingConnection->GetOwner());
		}

		UFGPipeConnectionComponentBase* plannedOutgoingConnection = nullptr;
		const TWeakObjectPtr<AFGCharacterPlayer> playerKey(player);
		PendingJunctionTransits.Remove(playerKey);

		if (IsValid(player) && IsValid(junction))
		{
			if (AHypertubeDestinationSubsystem* subsystem = AHypertubeDestinationSubsystem::Get(player))
			{
				subsystem->PrepareJunctionRoute(player, junction, incomingConnection, plannedOutgoingConnection);
			}
		}

		scope(hookSelf, player, incomingConnection, outPipeData, predictionPipeData);

		if (IsValid(plannedOutgoingConnection))
		{
			bool reversePath = false;
			if (ForceJunctionTravelRoute(junction, incomingConnection, plannedOutgoingConnection, outPipeData, reversePath))
			{
				FPendingJunctionTransit& pendingTransit = PendingJunctionTransits.FindOrAdd(playerKey);
				pendingTransit.Junction = junction;
				pendingTransit.IncomingConnection = incomingConnection;
				pendingTransit.OutgoingConnection = plannedOutgoingConnection;
				pendingTransit.bReversePath = reversePath;
			}
		}
	});

	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildablePipeHyperJunction::GetConnectionToTransitThrough, junctionCDO, [](auto& scope, AFGBuildablePipeHyperJunction* hookSelf, AFGCharacterPlayer* player, const FFGDynamicStruct& pipeData, float distance, float& outExitOffset) {
		UFGPipeConnectionComponentBase* vanillaExit = scope(hookSelf, player, pipeData, distance, outExitOffset);
		UFGPipeConnectionComponentBase* routedExit = vanillaExit;
		const TWeakObjectPtr<AFGCharacterPlayer> playerKey(player);
		FPendingJunctionTransit* pendingTransit = PendingJunctionTransits.Find(playerKey);

		if (pendingTransit != nullptr && !pendingTransit->bRouteExitConsumed && vanillaExit == pendingTransit->IncomingConnection.Get())
		{
			AFGBuildablePipeHyperJunction* pendingJunction = pendingTransit->Junction.Get();
			float junctionLength = 0.0f;
			if (IsValid(pendingJunction))
			{
				junctionLength = pendingJunction->GetLengthAlongPipe(player, pipeData);
			}

			float entryDistance = 0.0f;
			if (pendingTransit->bReversePath)
			{
				entryDistance = junctionLength;
			}
			if (FMath::IsNearlyEqual(distance, entryDistance, 0.01f))
			{
				if (UFGCharacterMovementComponent* movementComponent = Cast<UFGCharacterMovementComponent>(player->GetCharacterMovement()))
				{
					FPlayerPipeHyperData& playerPipeData = movementComponent->GetPipeHyperDataRef();
					float desiredSign = 1.0f;
					if (pendingTransit->bReversePath)
					{
						desiredSign = -1.0f;
					}
					playerPipeData.mPipeVelocityReal = FMath::Abs(playerPipeData.mPipeVelocityReal) * desiredSign;
					playerPipeData.mPipeVelocity = FMath::Abs(playerPipeData.mPipeVelocity) * desiredSign;
					playerPipeData.mPipeVelocityLast = FMath::Abs(playerPipeData.mPipeVelocityLast) * desiredSign;
				}

				scope.Override(static_cast<UFGPipeConnectionComponentBase*>(nullptr));
				return;
			}
		}

		AFGBuildablePipeHyperJunction* junction = nullptr;
		if (IsValid(vanillaExit))
		{
			junction = Cast<AFGBuildablePipeHyperJunction>(vanillaExit->GetOwner());
		}

		if (pendingTransit != nullptr && vanillaExit == pendingTransit->OutgoingConnection.Get())
		{
			pendingTransit->bRouteExitConsumed = true;
		}

		if (IsValid(player) && IsValid(junction))
		{
			if (AHypertubeDestinationSubsystem* subsystem = AHypertubeDestinationSubsystem::Get(player); subsystem != nullptr && subsystem->TryRouteJunctionExit(player, junction, vanillaExit, routedExit))
			{
				if (pendingTransit != nullptr)
				{
					pendingTransit->bRouteExitConsumed = true;
				}
				scope.Override(routedExit);
			}
		}
	});

	auto entranceCDO = GetMutableDefault<AFGPipeHyperStart>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::Dismantle_Implementation, entranceCDO, [](auto& Scope, AFGBuildable* Buildable) {
		if (AFGPipeHyperStart* Entrance = Cast<AFGPipeHyperStart>(Buildable); Entrance != nullptr && Entrance->HasAuthority())
		{
			if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(Entrance))
			{
				Subsystem->RemoveEntrance(Entrance);
			}
		}

		Scope(Buildable);
	});
#endif
}

void FHypertubeDestinationsModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHypertubeDestinationsModule, HypertubeDestinations)
