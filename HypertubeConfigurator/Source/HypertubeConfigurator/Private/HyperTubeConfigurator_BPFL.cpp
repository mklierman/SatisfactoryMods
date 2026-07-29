


#include "HyperTubeConfigurator_BPFL.h"
#include <SessionSettings/SessionSettingsManager.h>

void UHyperTubeConfigurator_BPFL::SetHypertubeStuff(UFGCharacterMovementComponent* movementComp)
{
	USessionSettingsManager* SessionSettings = movementComp->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto minSpeed = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.MinSpeed");
	auto gravityStrength = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.GravityStrength");
	auto friction = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.Friction");
	auto curveDamping = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.CurveDamping");
	auto ConstAccel = SessionSettings->GetFloatOptionValue("HypertubeConfigurator.ConstantAcceleration");
	auto hyperData = movementComp->mPipeData;
	hyperData.mMinPipeSpeed = minSpeed;
	hyperData.mPipeGravityStrength = gravityStrength;
	hyperData.mPipeFriction = friction;
	hyperData.mPipeCurveDamping = curveDamping;
	hyperData.mPipeConstantAcceleration = ConstAccel;
}
