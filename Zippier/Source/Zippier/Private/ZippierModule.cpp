#include "ZippierModule.h"
#include "Patching/NativeHookManager.h"
#include "Zippier_ConfigStruct.h"
#include <SessionSettings/SessionSettingsManager.h>

#pragma optimize("", off)
void FZippierModule::StartupModule() {
#if !WITH_EDITOR
	//void UFGCharacterMovementComponent::StartZiplineMovement(AActor* ziplineActor, const FVector& point1, const FVector& point2, const FVector& actorForward){ }
	SUBSCRIBE_METHOD(UFGCharacterMovementComponent::StartZiplineMovement, [this](auto scope, UFGCharacterMovementComponent* self, AActor* ziplineActor, const FVector& point1, const FVector& point2, const FVector& actorForward)
		{
			SetZiplineStuff(self);
		});
#endif
}

void FZippierModule::SetZiplineStuff(UFGCharacterMovementComponent* movementComp)
{
	USessionSettingsManager* SessionSettings = movementComp->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto baseSpeed = SessionSettings->GetFloatOptionValue("Zippier.BaseSpeed");
	auto sprintSpeed = SessionSettings->GetFloatOptionValue("Zippier.SprintSpeed");
	auto upMult = SessionSettings->GetFloatOptionValue("Zippier.UpSpeedMult");
	auto downMult = SessionSettings->GetFloatOptionValue("Zippier.DownSpeedMult");
	auto angle = SessionSettings->GetFloatOptionValue("Zippier.CheckAngle");
	movementComp->mZiplineSpeed = baseSpeed;
	movementComp->mZiplineSprintSpeed = sprintSpeed;
	movementComp->mZiplineContinuousTravelMaxAngle = angle;
	movementComp->mZiplineSpeedMultiplierDown = downMult;
	movementComp->mZiplineSpeedMultiplierUp = upMult;
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FZippierModule, Zippier);