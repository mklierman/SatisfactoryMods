#include "HoverPackPoleRangeModule.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Buildables/FGBuildableRailroadTrack.h"
#include "Equipment/FGHoverPack.h"
#include "FGBuildableSubsystem.h"
#include "FGPowerConnectionComponent.h"
#include "Patching/NativeHookManager.h"
#include "SessionSettings/SessionSettingsManager.h"

static constexpr int32 Mk2ConnectionCount = 7;
static constexpr int32 Mk3ConnectionCount = 10;

static const FString Mk1SettingId = TEXT("HoverPackPoleRange.Mk1");
static const FString Mk2SettingId = TEXT("HoverPackPoleRange.Mk2");
static const FString Mk3SettingId = TEXT("HoverPackPoleRange.Mk3");
static const FString RailSettingId = TEXT("HoverPackPoleRange.Rail");
static const FString ElseSettingId = TEXT("HoverPackPoleRange.Else");

float FHoverPackPoleRangeModule::GetScaledRange(AFGHoverPack* hoverPack, UFGPowerConnectionComponent* powerConnection, AFGBuildableRailroadTrack* railroadTrack, const float nativeSearchRadius) const
{
	const FString* settingId = &Mk1SettingId;

	if (IsValid(railroadTrack))
	{
		settingId = &RailSettingId;
	}
	else if (IsValid(powerConnection))
	{
		if (const AFGBuildablePowerPole* powerPole = Cast<AFGBuildablePowerPole>(powerConnection->GetOwner()))
		{
			if (powerPole->GetPowerPoleType() == EPowerPoleType::PPT_TOWER)
			{
				settingId = &ElseSettingId;
			}
			else if (powerConnection->GetMaxNumConnections() >= Mk3ConnectionCount)
			{
				settingId = &Mk3SettingId;
			}
			else if (powerConnection->GetMaxNumConnections() >= Mk2ConnectionCount)
			{
				settingId = &Mk2SettingId;
			}
		}
		else
		{
			settingId = &ElseSettingId;
		}
	}

	const UWorld* world = hoverPack->GetWorld();
	const USessionSettingsManager* sessionSettings = nullptr;
	if (IsValid(world))
	{
		sessionSettings = world->GetSubsystem<USessionSettingsManager>();
	}

	const bool bSettingRegistered = IsValid(sessionSettings) && sessionSettings->FindSessionSetting(*settingId) != nullptr;
	float percentageIncrease = 0.0f;
	if (bSettingRegistered)
	{
		percentageIncrease = FMath::Max(0.0f, sessionSettings->GetFloatOptionValue(*settingId));
	}

	const float rangeMultiplier = 1.0f + percentageIncrease / 100.0f;
	return nativeSearchRadius * rangeMultiplier;
}

void FHoverPackPoleRangeModule::UpdateSearchRadius(AFGHoverPack* hoverPack)
{
	if (!IsValid(hoverPack))
	{
		return;
	}

	const float previousRadius = hoverPack->mPowerConnectionSearchRadius;
	const TWeakObjectPtr<AFGHoverPack> hoverPackKey(hoverPack);
	FHoverPackRangeState* state = mRangeStates.Find(hoverPackKey);
	if (!state)
	{
		FHoverPackRangeState initialState;
		initialState.nativeSearchRadius = previousRadius;
		initialState.lastAppliedSearchRadius = previousRadius;
		state = &mRangeStates.Add(hoverPackKey, initialState);
	}
	else if (!FMath::IsNearlyEqual(previousRadius, state->lastAppliedSearchRadius))
	{
		state->nativeSearchRadius = previousRadius;
	}

	const float newRadius = GetScaledRange(
		hoverPack,
		hoverPack->GetCurrentPowerConnection(),
		hoverPack->GetCurrentRailroadTrack(),
		state->nativeSearchRadius);
	hoverPack->mPowerConnectionSearchRadius = newRadius;
	state->lastAppliedSearchRadius = newRadius;
}

bool FHoverPackPoleRangeModule::TryConnectToBestPowerConnection(AFGHoverPack* hoverPack)
{
	if (!IsValid(hoverPack))
	{
		return false;
	}

	const TWeakObjectPtr<AFGHoverPack> hoverPackKey(hoverPack);
	const FHoverPackRangeState* state = mRangeStates.Find(hoverPackKey);
	if (!state || state->nativeSearchRadius <= 0.0f)
	{
		return false;
	}

	const FVector hoverPackLocation = hoverPack->GetActorLocation();
	auto getTrackDistance = [&hoverPackLocation](AFGBuildableRailroadTrack* track)
		{
			const FRailroadTrackPosition trackPosition = track->FindTrackPositionClosestToWorldLocation(hoverPackLocation);
			FVector trackLocation;
			FVector trackDirection;
			track->GetWorldLocationAndDirectionAtPosition(trackPosition, trackLocation, trackDirection);
			return FVector::Distance(hoverPackLocation, trackLocation);
		};

	UFGPowerConnectionComponent* currentConnection = hoverPack->GetCurrentPowerConnection();
	AFGBuildableRailroadTrack* currentTrack = hoverPack->GetCurrentRailroadTrack();
	if (IsValid(currentTrack))
	{
		UFGPowerConnectionComponent* thirdRail = currentTrack->GetThirdRail();
		const float currentRange = GetScaledRange(hoverPack, thirdRail, currentTrack, state->nativeSearchRadius);
		if (IsValid(thirdRail) && thirdRail->HasPower() && getTrackDistance(currentTrack) <= currentRange)
		{
			return true;
		}
	}
	else if (IsValid(currentConnection))
	{
		const float currentRange = GetScaledRange(hoverPack, currentConnection, nullptr, state->nativeSearchRadius);
		const float currentDistance = FVector::Distance(hoverPackLocation, currentConnection->GetComponentLocation());
		if (currentConnection->HasPower() && currentDistance <= currentRange)
		{
			return true;
		}
	}

	const UWorld* world = hoverPack->GetWorld();
	const USessionSettingsManager* sessionSettings = nullptr;
	if (IsValid(world))
	{
		sessionSettings = world->GetSubsystem<USessionSettingsManager>();
	}

	if (!IsValid(sessionSettings))
	{
		return false;
	}

	float maximumPercentageIncrease = 0.0f;
	for (const FString* settingId : {&Mk1SettingId, &Mk2SettingId, &Mk3SettingId, &RailSettingId, &ElseSettingId})
	{
		if (sessionSettings->FindSessionSetting(*settingId))
		{
			maximumPercentageIncrease = FMath::Max(
				maximumPercentageIncrease,
				FMath::Max(0.0f, sessionSettings->GetFloatOptionValue(*settingId)));
		}
	}
	const float maximumSearchRadius = state->nativeSearchRadius * (1.0f + maximumPercentageIncrease / 100.0f);

	AFGBuildableSubsystem* buildableSubsystem = AFGBuildableSubsystem::Get(hoverPack);
	if (!IsValid(buildableSubsystem))
	{
		return false;
	}

	TArray<AFGBuildable*> nearbyBuildables;
	buildableSubsystem->GetNearestBuildables(nearbyBuildables, hoverPackLocation, maximumSearchRadius);

	UFGPowerConnectionComponent* bestConnection = nullptr;
	AFGBuildableRailroadTrack* bestTrack = nullptr;
	float bestRemainingReach = -1.0f;

	for (AFGBuildable* buildable : nearbyBuildables)
	{
		if (!IsValid(buildable))
		{
			continue;
		}

		if (AFGBuildableRailroadTrack* track = Cast<AFGBuildableRailroadTrack>(buildable))
		{
			UFGPowerConnectionComponent* thirdRail = track->GetThirdRail();
			if (!IsValid(thirdRail) || !thirdRail->HasPower())
			{
				continue;
			}

			const float candidateRange = GetScaledRange(hoverPack, thirdRail, track, state->nativeSearchRadius);
			const float candidateDistance = getTrackDistance(track);
			const float remainingReach = candidateRange - candidateDistance;
			if (remainingReach >= 0.0f && remainingReach > bestRemainingReach)
			{
				bestConnection = thirdRail;
				bestTrack = track;
				bestRemainingReach = remainingReach;
			}
			continue;
		}

		TInlineComponentArray<UFGPowerConnectionComponent*> powerConnections;
		buildable->GetComponents(powerConnections);
		for (UFGPowerConnectionComponent* candidateConnection : powerConnections)
		{
			if (!IsValid(candidateConnection) || !candidateConnection->HasPower())
			{
				continue;
			}

			const float candidateRange = GetScaledRange(hoverPack, candidateConnection, nullptr, state->nativeSearchRadius);
			const float candidateDistance = FVector::Distance(hoverPackLocation, candidateConnection->GetComponentLocation());
			const float remainingReach = candidateRange - candidateDistance;
			if (remainingReach >= 0.0f && remainingReach > bestRemainingReach)
			{
				bestConnection = candidateConnection;
				bestTrack = nullptr;
				bestRemainingReach = remainingReach;
			}
		}
	}

	if (!IsValid(bestConnection))
	{
		return false;
	}

	hoverPack->ConnectToPowerConnection(bestConnection, bestTrack);
	return true;
}

void FHoverPackPoleRangeModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGHoverPack::ConnectToNearestPowerConnection, [this](auto& scope, AFGHoverPack* self)
		{
			UpdateSearchRadius(self);
			if (TryConnectToBestPowerConnection(self))
			{
				scope.Cancel();
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGHoverPack::ConnectToPowerConnection,
		[this](AFGHoverPack* self, UFGPowerConnectionComponent*, AFGBuildableRailroadTrack*)
		{
			UpdateSearchRadius(self);
		});

	SUBSCRIBE_METHOD_AFTER(AFGHoverPack::DisconnectFromCurrentPowerConnection, [this](AFGHoverPack* self)
		{
			UpdateSearchRadius(self);
		});

	SUBSCRIBE_METHOD(AFGHoverPack::OnRep_CurrentPowerConnection, [this](auto& scope, AFGHoverPack* self)
		{
			UpdateSearchRadius(self);
		});

	SUBSCRIBE_METHOD(AFGHoverPack::OnRep_HasConnection, [this](auto& scope, AFGHoverPack* self)
		{
			UpdateSearchRadius(self);
		});

	AFGHoverPack* hoverPackCDO = GetMutableDefault<AFGHoverPack>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHoverPack::Tick, hoverPackCDO, [this](AFGHoverPack* self, float deltaTime)
		{
			UpdateSearchRadius(self);
		});
#endif
}

IMPLEMENT_GAME_MODULE(FHoverPackPoleRangeModule, HoverPackPoleRange);
