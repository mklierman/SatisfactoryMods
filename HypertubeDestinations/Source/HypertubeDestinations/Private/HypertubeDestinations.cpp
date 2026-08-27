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
using FJunctionEntryCallScope = TCallScope<EPipeHyperEnterResult (*)(AFGBuildablePipeHyperJunction*, AFGCharacterPlayer*, UFGPipeConnectionComponentBase*, FFGDynamicStruct&, const FFGDynamicStruct&)>;

using FJunctionTransitCallScope = TCallScope<UFGPipeConnectionComponentBase* (*)(AFGBuildablePipeHyperJunction*, AFGCharacterPlayer*, const FFGDynamicStruct&, float, float&)>;

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

void HandleJunctionEntry(FJunctionEntryCallScope& Scope, AFGBuildablePipeHyperJunction* HookSelf, AFGCharacterPlayer* Player, UFGPipeConnectionComponentBase* IncomingConnection, FFGDynamicStruct& OutPipeData,
						 const FFGDynamicStruct& PredictionPipeData)
{
	AFGBuildablePipeHyperJunction* Junction = nullptr;
	if (IsValid(IncomingConnection))
	{
		Junction = Cast<AFGBuildablePipeHyperJunction>(IncomingConnection->GetOwner());
	}
	UFGPipeConnectionComponentBase* PlannedOutgoingConnection = nullptr;
	const TWeakObjectPtr<AFGCharacterPlayer> PlayerKey(Player);
	PendingJunctionTransits.Remove(PlayerKey);

	if (IsValid(Player) && IsValid(Junction))
	{
		if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(Player))
		{
			Subsystem->PrepareJunctionRoute(Player, Junction, IncomingConnection, PlannedOutgoingConnection);
		}
	}

	Scope(HookSelf, Player, IncomingConnection, OutPipeData, PredictionPipeData);

	if (IsValid(PlannedOutgoingConnection))
	{
		bool bReversePath = false;
		if (ForceJunctionTravelRoute(Junction, IncomingConnection, PlannedOutgoingConnection, OutPipeData, bReversePath))
		{
			FPendingJunctionTransit& PendingTransit = PendingJunctionTransits.FindOrAdd(PlayerKey);
			PendingTransit.Junction = Junction;
			PendingTransit.IncomingConnection = IncomingConnection;
			PendingTransit.OutgoingConnection = PlannedOutgoingConnection;
			PendingTransit.bReversePath = bReversePath;
		}
	}
}

void HandleJunctionTransit(FJunctionTransitCallScope& Scope, AFGBuildablePipeHyperJunction* HookSelf, AFGCharacterPlayer* Player, const FFGDynamicStruct& PipeData, float Distance, float& OutExitOffset)
{
	UFGPipeConnectionComponentBase* VanillaExit = Scope(HookSelf, Player, PipeData, Distance, OutExitOffset);
	UFGPipeConnectionComponentBase* RoutedExit = VanillaExit;
	const TWeakObjectPtr<AFGCharacterPlayer> PlayerKey(Player);
	FPendingJunctionTransit* PendingTransit = PendingJunctionTransits.Find(PlayerKey);

	if (PendingTransit != nullptr && !PendingTransit->bRouteExitConsumed && VanillaExit == PendingTransit->IncomingConnection.Get())
	{
		AFGBuildablePipeHyperJunction* PendingJunction = PendingTransit->Junction.Get();
		float JunctionLength = 0.0f;
		if (IsValid(PendingJunction))
		{
			JunctionLength = PendingJunction->GetLengthAlongPipe(Player, PipeData);
		}

		float EntryDistance = 0.0f;
		if (PendingTransit->bReversePath)
		{
			EntryDistance = JunctionLength;
		}
		if (FMath::IsNearlyEqual(Distance, EntryDistance, 0.01f))
		{
			if (UFGCharacterMovementComponent* MovementComponent = Cast<UFGCharacterMovementComponent>(Player->GetCharacterMovement()))
			{
				FPlayerPipeHyperData& PlayerPipeData = MovementComponent->GetPipeHyperDataRef();
				float DesiredSign = 1.0f;
				if (PendingTransit->bReversePath)
				{
					DesiredSign = -1.0f;
				}
				PlayerPipeData.mPipeVelocityReal = FMath::Abs(PlayerPipeData.mPipeVelocityReal) * DesiredSign;
				PlayerPipeData.mPipeVelocity = FMath::Abs(PlayerPipeData.mPipeVelocity) * DesiredSign;
				PlayerPipeData.mPipeVelocityLast = FMath::Abs(PlayerPipeData.mPipeVelocityLast) * DesiredSign;
			}

			Scope.Override(static_cast<UFGPipeConnectionComponentBase*>(nullptr));
			return;
		}
	}

	AFGBuildablePipeHyperJunction* Junction = nullptr;
	if (IsValid(VanillaExit))
	{
		Junction = Cast<AFGBuildablePipeHyperJunction>(VanillaExit->GetOwner());
	}

	if (PendingTransit != nullptr && VanillaExit == PendingTransit->OutgoingConnection.Get())
	{
		PendingTransit->bRouteExitConsumed = true;
	}

	if (IsValid(Player) && IsValid(Junction))
	{
		if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(Player); Subsystem != nullptr && Subsystem->TryRouteJunctionExit(Player, Junction, VanillaExit, RoutedExit))
		{
			if (PendingTransit != nullptr)
			{
				PendingTransit->bRouteExitConsumed = true;
			}
			Scope.Override(RoutedExit);
		}
	}
}
} // namespace

void FHypertubeDestinationsModule::StartupModule()
{
#if !WITH_EDITOR
	auto junctionCDO = GetMutableDefault<AFGBuildablePipeHyperJunction>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildablePipeHyperJunction::OnPipeEnterReal, junctionCDO, HandleJunctionEntry);
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildablePipeHyperJunction::GetConnectionToTransitThrough, junctionCDO, HandleJunctionTransit);

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
