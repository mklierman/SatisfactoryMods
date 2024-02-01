#include "DaisyChainEverythingSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Buildables/FGBuildable.h"
#include "Components/ActorComponent.h"
#include "FGPowerConnectionComponent.h"
#include "DaisyChainEverythingSubsystem.h"
#include "Tasks/Task.h"

ADaisyChainEverythingSubsystem::ADaisyChainEverythingSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
}

void ADaisyChainEverythingSubsystem::Init()
{
	USessionSettingsManager* SessionSettingsManager = GetWorld()->GetSubsystem<USessionSettingsManager>();
	check(SessionSettingsManager);

	OnOptionUpdatedDelegate = FOnOptionUpdated::CreateUObject(this, &ADaisyChainEverythingSubsystem::OnSessionSettingUpdated);
	SessionSettingsManager->SubscribeToOptionUpdate("DaisyChainEverything.MinConnections", OnOptionUpdatedDelegate);
}

void ADaisyChainEverythingSubsystem::OnSessionSettingUpdated(const FString StrID, FVariant value)
{
	USessionSettingsManager* SessionSettings = GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto minConnections = (int)SessionSettings->GetFloatOptionValue("DaisyChainEverything.MinConnections");
	auto IgnorePowerPoles = SessionSettings->GetBoolOptionValue("DaisyChainEverything.IgnorePowerPolesBool");

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildable::StaticClass(), FoundActors);
	for (auto buildable : FoundActors)
	{
		if (IgnorePowerPoles)
		{
			auto PowerPole = Cast< AFGBuildablePowerPole>(buildable);
			if (PowerPole)
			{
				return;
			}
		}
		TInlineComponentArray<UFGPowerConnectionComponent*> powerConns(buildable);
		for (int i = 0; i < powerConns.Num(); i++)
		{
			if (powerConns[i]->mMaxNumConnectionLinks < minConnections)
			{
				powerConns[i]->mMaxNumConnectionLinks = minConnections;
			}
		}
	}
}
