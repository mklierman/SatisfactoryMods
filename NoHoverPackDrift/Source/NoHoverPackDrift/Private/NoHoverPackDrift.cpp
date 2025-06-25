// Copyright Epic Games, Inc. All Rights Reserved.

#include "NoHoverPackDrift.h"
#include "Patching/NativeHookManager.h"
#include "Subsystem/SubsystemActorManager.h"
#include "NoHoverPackDrift_Subsystem.h"
#include <SessionSettings/SessionSettingsManager.h>

#define LOCTEXT_NAMESPACE "FNoHoverPackDriftModule"

void FNoHoverPackDriftModule::StartupModule()
{
	//virtual void OnCharacterMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode, EMovementMode NewMovementMode, uint8 NewCustomMode)

	AFGHoverPack* hp = GetMutableDefault<AFGHoverPack>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL(AFGHoverPack::OnCharacterMovementModeChanged, hp, [this](auto& scope, AFGHoverPack* self, EMovementMode PreviousMovementMode, uint8 PreviousCustomMode, EMovementMode NewMovementMode, uint8 NewCustomMode)
		{
			OnCharacterMovementModeChanged(self);
		});
#endif
}

void FNoHoverPackDriftModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FNoHoverPackDriftModule::OnCharacterMovementModeChanged(AFGHoverPack* hoverpack)
{
	if (!hoverpack)
		return;

	USessionSettingsManager* SessionSettings = hoverpack->GetWorld()->GetSubsystem<USessionSettingsManager>();
	if (!SessionSettings)
		return;

	auto speed = SessionSettings->GetFloatOptionValue("NoHoverPackDrift.Speed");
	auto acceleration = SessionSettings->GetFloatOptionValue("NoHoverPackDrift.Acceleration");
	auto friction = SessionSettings->GetFloatOptionValue("NoHoverPackDrift.Friction");
	auto radius = SessionSettings->GetFloatOptionValue("NoHoverPackDrift.Radius");
	auto sprint = SessionSettings->GetFloatOptionValue("NoHoverPackDrift.Sprint");


	USubsystemActorManager* SubsystemActorManager = hoverpack->GetWorld()->GetSubsystem<USubsystemActorManager>();
	if (!SubsystemActorManager)
		return;
	ANoHoverPackDrift_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ANoHoverPackDrift_Subsystem>();
	if (!subsystem)
		return;

	UClass* myClass = hoverpack->GetClass();
	if (!subsystem->HoverpackMap.Contains(myClass))
	{
		SetDefaults(hoverpack);
	}
	auto defaults = subsystem->HoverpackMap[myClass];

	hoverpack->mHoverSpeed = defaults.Speed * speed;
	hoverpack->mHoverAccelerationSpeed = defaults.Acceleration * acceleration;
	hoverpack->mHoverFriction = defaults.Friction * friction;
	hoverpack->mPowerConnectionSearchRadius = defaults.Radius * radius;
	hoverpack->mHoverSprintMultiplier = defaults.Sprint * sprint;
}

void FNoHoverPackDriftModule::SetDefaults(AFGHoverPack* hoverpack)
{
	if (hoverpack)
	{
		USubsystemActorManager* SubsystemActorManager = hoverpack->GetWorld()->GetSubsystem<USubsystemActorManager>();
		ANoHoverPackDrift_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ANoHoverPackDrift_Subsystem>();
		FHoverPackDefaultsStruct defaults;
		defaults.Speed = hoverpack->mHoverSpeed;
		defaults.Acceleration = hoverpack->mHoverAccelerationSpeed;
		defaults.Friction = hoverpack->mHoverFriction;
		defaults.Radius = hoverpack->mPowerConnectionSearchRadius;
		defaults.Sprint = hoverpack->mHoverSprintMultiplier;

		subsystem->HoverpackMap.Add(hoverpack->GetClass(), defaults);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNoHoverPackDriftModule, NoHoverPackDrift)