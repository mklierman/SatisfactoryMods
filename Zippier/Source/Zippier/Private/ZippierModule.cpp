#include "ZippierModule.h"
#include "Patching/NativeHookManager.h"
#include "Zippier_ConfigStruct.h"

#pragma optimize("", off)
void FZippierModule::StartupModule() {
#if !WITH_EDITOR
	//void UFGCharacterMovementComponent::StartZiplineMovement(AActor* ziplineActor, const FVector& point1, const FVector& point2, const FVector& actorForward){ }
	SUBSCRIBE_METHOD(UFGCharacterMovementComponent::StartZiplineMovement, [=](auto scope, UFGCharacterMovementComponent* self, AActor* ziplineActor, const FVector& point1, const FVector& point2, const FVector& actorForward)
		{
			SetZiplineStuff(self);
		});
#endif
}

void FZippierModule::SetZiplineStuff(UFGCharacterMovementComponent* movementComp)
{
	auto config = FZippier_ConfigStruct::GetActiveConfig(movementComp->GetWorld());
	movementComp->mZiplineSpeed = config.ZiplineBaseSpeed;
	movementComp->mZiplineSprintSpeed = config.ZiplineSprintSpeed;
	movementComp->mZiplineContinuousTravelMaxAngle = config.ZiplineCheckAngle;
	movementComp->mZiplineSpeedMultiplierDown = config.ZiplineDownSpeedMult;
	movementComp->mZiplineSpeedMultiplierUp = config.ZiplineUpSpeedMult;
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FZippierModule, Zippier);