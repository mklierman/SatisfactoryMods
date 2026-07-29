// Copyright Epic Games, Inc. All Rights Reserved.

#include "HypertubeConfigurator.h"
#include <SessionSettings/SessionSettingsManager.h>
#include "Patching/NativeHookManager.h"
#include <Buildables/FGBuildablePipeHyper.h>
#include "FGCharacterPlayer.h"

#define LOCTEXT_NAMESPACE "FHypertubeConfiguratorModule"
DEFINE_LOG_CATEGORY(HypertubeConfigurator_Log);

void FHypertubeConfiguratorModule::StartupModule()
{
#if !WITH_EDITOR
#endif
	//bool EnterPipeHyper(class AFGPipeHyperStart* pipe);
	
	SUBSCRIBE_METHOD(UFGCharacterMovementComponent::EnterPipeHyper, [this](auto scope, UFGCharacterMovementComponent* self, class AFGPipeHyperStart* pipe)
		{
			SetHypertubeStuff(self);
		});
	SUBSCRIBE_METHOD(UFGCharacterMovementComponent::EnterPipeHyperDirect, [this](auto scope, UFGCharacterMovementComponent* self, UFGPipeConnectionComponentBase* connectionEnteredThrough, const float InitialMinSpeedFactor = 1.0f)
		{
			SetHypertubeStuff(self);
		});
	SUBSCRIBE_METHOD(UFGCharacterMovementComponent::EnterPipeHyperInternal, [this](auto scope, UFGCharacterMovementComponent* self, UFGPipeConnectionComponentBase* connectionEnteredThrough, const float initialPipeVelocity, const float initialPipeProgress, const float accumulatedDeltaTime)
		{
			SetHypertubeStuff(self);
		});
}

void FHypertubeConfiguratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FHypertubeConfiguratorModule::SetHypertubeStuff(UFGCharacterMovementComponent* movementComp)
{
	//UE_LOGFMT(HypertubeConfigurator_Log, Display, "mMinPipeSpeed: {0}, mPipeGravityStrength: {1}, mPipeFriction: {2}, mPipeCurveDamping: {3}, mPipeConstantAcceleration: {4}"
	//	, movementComp->mPipeData.mMinPipeSpeed, movementComp->mPipeData.mPipeGravityStrength,
	//	movementComp->mPipeData.mPipeFriction, movementComp->mPipeData.mPipeCurveDamping,
	//	movementComp->mPipeData.mPipeConstantAcceleration);
	USessionSettingsManager* SessionSettings = movementComp->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto minSpeed = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.MinSpeed");
	auto gravityStrength = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.GravityStrength");
	auto friction = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.Friction");
	auto curveDamping = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.CurveDamping");
	auto ConstAccel = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.ConstantAcceleration");
	movementComp->mPipeData.mMinPipeSpeed = minSpeed;
	movementComp->mPipeData.mPipeGravityStrength = gravityStrength;
	movementComp->mPipeData.mPipeFriction = friction;
	movementComp->mPipeData.mPipeCurveDamping = curveDamping;
	movementComp->mPipeData.mPipeConstantAcceleration = ConstAccel;

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHypertubeConfiguratorModule, HypertubeConfigurator)