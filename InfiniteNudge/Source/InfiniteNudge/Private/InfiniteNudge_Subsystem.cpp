#include "InfiniteNudge_Subsystem.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif

DEFINE_LOG_CATEGORY(InfiniteNudge_Log);
//UE_LOG(InfiniteNudge_Log, Warning, TEXT("For Easy Copy-Paste"));
#pragma optimize("", off)

namespace
{
	// Helper: apply vector as world or local offset depending on toggle
	static void ApplyOffset(AActor* target, const FVector& vec, float amount, bool useWorld)
	{
		if (!target)
		{
			return;
		}
		if (useWorld)
		{
			target->AddActorWorldOffset(vec * amount);
		}
		else
		{
			target->AddActorLocalOffset(vec * amount);
		}
	}

	// Helper: compute nudge vector by using subsystem to get look angle and rotating input
	static FVector ComputeNudgeVector(AInfiniteNudge_Subsystem* subsystem, AActor* target, float xDirection, float yDirection, AFGPlayerController* controller)
	{
		if (!subsystem || !target)
		{
			return FVector(xDirection, yDirection, 0);
		}
		int lookAngle = subsystem->GetClosestLookAngle(target, controller);
		return FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
	}

	// Helper: apply nudge to target with optional custom vector calculator
	static void ApplyNudgeToTarget(AInfiniteNudge_Subsystem* subsystem, AActor* target, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller, TFunction<FVector(AActor*, float, float, AFGPlayerController*)> computeVec = nullptr)
	{
		if (!subsystem || !target || !controller)
		{
			return;
		}

		const float amount = subsystem->GetCurrentNudgeAmount(controller);
		const bool useWorld = subsystem->ShouldWorldOffset();

		// Vertical nudge via modifier key maps to X input being used as vertical amount in many cases
		if (controller->IsInputKeyDown(subsystem->VerticalNudgeKey) || controller->IsInputKeyDown(EKeys::Gamepad_RightShoulder))
		{
			// Default vertical behavior: move along Z by xDirection
			ApplyOffset(target, FVector(0, 0, xDirection), amount, useWorld);
			return;
		}

		// explicit Z direction (gamepad shoulder mapping etc)
		if (zDirection != 0.0f)
		{
			ApplyOffset(target, FVector(0, 0, zDirection), amount, useWorld);
			return;
		}

		// Default or custom XY mapping
		FVector vec;
		if (computeVec)
		{
			vec = computeVec(target, xDirection, yDirection, controller);
		}
		else
		{
			vec = ComputeNudgeVector(subsystem, target, xDirection, yDirection, controller);
		}
		ApplyOffset(target, vec, amount, useWorld);
	}
} // anonymous namespace

//Get Building direction closest to camera direction

void AInfiniteNudge_Subsystem::NudgeHologram(AFGHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	if (hologram && hologram->IsHologramLocked())
	{
		GetConfigValues(hologram->GetWorld(), controller);

		if (controller->IsInputKeyDown(VerticalNudgeKey) || controller->IsInputKeyDown(EKeys::Gamepad_RightShoulder))
		{
			zDirection = xDirection;
		}

		if (zDirection != 0.0)
		{
			if (ShouldWorldOffset())
			{
				hologram->GetNudgeHologramTarget()->AddActorWorldOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * zDirection));
			}
			else
			{
				// Handle wall attachment's "vertical" movement
				if (auto wallAttachmentHolo = Cast<AFGWallAttachmentHologram>(hologram))
				{
					hologram->GetNudgeHologramTarget()->AddActorLocalOffset(FVector((GetCurrentNudgeAmount(controller) * -zDirection), 0, 0));
				}
				else
				{
					hologram->GetNudgeHologramTarget()->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * zDirection));
				}
			}
		}
		else if (auto beltHolo = Cast< AFGConveyorBeltHologram>(hologram))
		{
			NudgeBelt(beltHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto pipeHolo = Cast<AFGPipelineHologram>(hologram))
		{
			NudgePipe(pipeHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto wireHolo = Cast<AFGWireHologram>(hologram))
		{
			NudgeWire(wireHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto trackHolo = Cast<AFGRailroadTrackHologram>(hologram))
		{
			NudgeRailroadTrack(trackHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto wallAttachmentHolo = Cast<AFGWallAttachmentHologram>(hologram))
		{
			NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto signHolo = Cast<AFGStandaloneSignHologram>(hologram))
		{
			NudgeSign(signHolo, xDirection, yDirection, zDirection, controller);
		}
		else if (auto beamHolo = Cast<AFGBeamHologram>(hologram))
		{
			NudgeBeam(beamHolo, xDirection, yDirection, zDirection, controller);
		}
		else
		{
			FVector rVectorz = ComputeNudgeVector(this, hologram, xDirection, yDirection, controller);
			ApplyOffset(hologram, rVectorz, GetCurrentNudgeAmount(controller), ShouldWorldOffset());
		}
		hologram->mHologramLockLocation = hologram->GetActorLocation();
		hologram->GetNudgeHologramTarget()->mHologramLockLocation = hologram->GetNudgeHologramTarget()->GetActorLocation();
	}
}

void AInfiniteNudge_Subsystem::NudgeBelt(AFGConveyorBeltHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (auto wallAttachmentHolo = Cast< AFGWallAttachmentHologram>(nudgeTarget))
	{
		NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, zDirection, controller);
	}
	else
	{
		ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
	}
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

// Refactor NudgePipe
void AInfiniteNudge_Subsystem::NudgePipe(AFGPipelineHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (auto wallAttachmentHolo = Cast< AFGWallAttachmentHologram>(nudgeTarget))
	{
		NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, zDirection, controller);
	}
	else
	{
		ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
	}
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

// Refactor NudgeWire
void AInfiniteNudge_Subsystem::NudgeWire(AFGWireHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

// Refactor NudgeRailroadTrack
void AInfiniteNudge_Subsystem::NudgeRailroadTrack(AFGRailroadTrackHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

// Refactor NudgeWallAttachment: keep vertical special-case and use custom compute for the xDirection!=0 branch
void AInfiniteNudge_Subsystem::NudgeWallAttachment(AFGWallAttachmentHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	// Vertical nudge case handled in earlier code, so only do anything if we're not doing that
	if (!controller->IsInputKeyDown(VerticalNudgeKey))
	{
		// custom mapping used previously: FVector(0, yDirection, (xDirection * -1)).RotateAngleAxis(lookAngle, FVector(0, 1, 0))
		 
		// No need for look angle; Adjust y for case of ceiling wall attachment, otherwise just move it relative to itself and the inputs 
		auto compute = [this, xDirection](AActor* target, float x, float y, AFGPlayerController* pc) -> FVector
			{
				FRotator rot = target->GetActorRotation();
				FVector v;
				if (FMath::IsNearlyEqual(rot.Pitch, -90, 0.01f))
				{
					return v = FVector(0.0f, y, x);
				}
				else
				{
					return v = FVector(0.0f, -y, x);
				}
			};

		ApplyNudgeToTarget(this, hologram, xDirection, yDirection, zDirection, controller, compute);

		hologram->mHologramLockLocation = hologram->GetActorLocation();
	}
}

// Refactor NudgeSign
void AInfiniteNudge_Subsystem::NudgeSign(AFGStandaloneSignHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	if (auto nudgeTarget = Cast<AFGSignPoleHologram>(hologram->GetNudgeHologramTarget()))
	{
		ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
		nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
	}
	else
	{
		ApplyNudgeToTarget(this, hologram, xDirection, yDirection, zDirection, controller, nullptr);
	}
}

// Refactor NudgeBeam
void AInfiniteNudge_Subsystem::NudgeBeam(AFGBeamHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	ApplyNudgeToTarget(this, nudgeTarget, xDirection, yDirection, zDirection, controller, nullptr);
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

void AInfiniteNudge_Subsystem::ScaleHologram(AFGHologram* hologram, AFGPlayerController* controller)
{
	if (hologram && hologram->IsHologramLocked())
	{
		GetConfigValues(hologram->GetWorld(), controller);
		if (controller)
		{
			// 1.0 == 100%
			float scaleAmount = 5;
			if (controller->IsInputKeyDown(TinyNudgeKey))
			{
				scaleAmount = TinyScaleAmount;
			}
			else if (controller->IsInputKeyDown(SmallNudgeKey))
			{
				scaleAmount = SmallScaleAmount;
			}
			else if (controller->IsInputKeyDown(LargeNudgeKey))
			{
				scaleAmount = LargeScaleAmount;
			}
			if (controller->IsInputKeyDown(DecreaseScaleKey))
			{
				scaleAmount = scaleAmount * -1.0;
			}

			if (scaleAmount != 0.0)
			{
				FVector currentScale = hologram->GetActorRelativeScale3D();
				FVector newScale = currentScale;
				if (controller->IsInputKeyDown(EKeys::RightControl))
				{
					newScale.X = newScale.X + (scaleAmount / 100);
				}
				else if (controller->IsInputKeyDown(EKeys::RightAlt))
				{
					newScale.Y = newScale.Y + (scaleAmount / 100);
				}
				else if (controller->IsInputKeyDown(EKeys::RightShift))
				{
					newScale.Z = newScale.Z + (scaleAmount / 100);
				}
				else
				{
					newScale = newScale + (scaleAmount / 100);
				}

				if (HasAuthority() && false)
				{
					hologram->SetActorRelativeScale3D(newScale);
					ForceNetUpdate();
				}
				else
				{
					if (UInfiniteNudge_RCO* RCO = UInfiniteNudge_RCO::Get(GetWorld()))
					{
						hologram->SetActorRelativeScale3D(newScale);
						RCO->Server_SetHologramScale(controller, newScale);
					}
				}
			}
		}
	}
}

void AInfiniteNudge_Subsystem::GetConfigValues(UObject* worldContext, APlayerController* controller)
{
	TArray<FKey> ModifierKeys;
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.LargeNudge", LargeNudgeKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.VerticalNudge", VerticalNudgeKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.IncreaseScale", IncreaseScaleKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.DecreaseScale", DecreaseScaleKey, ModifierKeys);

	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.TinyNudgeController", TinyNudgeGamepadKey, ModifierKeys);
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.SmallNudgeController", SmallNudgeGamepadKey, ModifierKeys);

	auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(worldContext);
	TinyNudgeAmount = (float)config.LeftCtrlNudgeAmount;
	SmallNudgeAmount = (float)config.LeftAltNudgeAmount;
	LargeNudgeAmount = (float)config.LargeNudgeAmount;
	TinyScaleAmount = (float)config.TinyScaleAmount;
	SmallScaleAmount = (float)config.SmallScaleAmount;
	LargeScaleAmount = (float)config.LargeScaleAmount;
}

float AInfiniteNudge_Subsystem::GetCurrentNudgeAmount(APlayerController* controller)
{
	if (controller->IsInputKeyDown(TinyNudgeKey) || controller->IsInputKeyDown(TinyNudgeGamepadKey))
	{
		return TinyNudgeAmount;
	}
	else if (controller->IsInputKeyDown(SmallNudgeKey) || controller->IsInputKeyDown(SmallNudgeGamepadKey))
	{
		return SmallNudgeAmount;
	}
	else if (controller->IsInputKeyDown(LargeNudgeKey))
	{
		return LargeNudgeAmount;
	}
	else
	{
		return 100.0f;
	}
}

bool AInfiniteNudge_Subsystem::ShouldWorldOffset()
{
#if PLATFORM_WINDOWS
	return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
	return false;
#endif
}

FVector AInfiniteNudge_Subsystem::GetFrontOffset(TSubclassOf<AFGGenericBuildableHologram> hologram)
{
	FVector offset = FVector(0, 0, 0);
	if (auto holo = Cast<AFGGenericBuildableHologram>(hologram))
	{
		// Determine "Front" of holo
		//auto snapAxis = holo->mSnapAxis;
		//FVector offset = FVector(0, 0, 0);
		//switch (snapAxis)
		//{
		//case EAxis::X:
		//	offset = holo->GetActorForwardVector();
		//	break;

		//case EAxis::Y:
		//	offset = holo->GetActorRightVector();
		//	break;

		//case EAxis::Z:
		//	offset = holo->GetActorUpVector();
		//	break;

		//default:
		//	break;
		//}
	}
	return offset;
}

FVector AInfiniteNudge_Subsystem::CalcPivotAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat)
{
	auto ProcessAxes = [&](const FVector& VAxis, const FVector& Axis1, const FVector& Axis2) -> FVector
		{
			int Inverted = FMath::Sign(VAxis.Z);
			if (DesiredAxis == EAxis::Z) {
				return Inverted * VAxis;
			}
			float Check1 = FVector(Axis1.X, Axis1.Y, 0.f).GetSafeNormal() | ViewVector;
			float Check2 = FVector(Axis2.X, Axis2.Y, 0.f).GetSafeNormal() | ViewVector;
			if (FMath::Abs(Check1) >= FMath::Abs(Check2)) {
				return FMath::Sign(Check1) * ((DesiredAxis == EAxis::X) ? Axis1 : (Inverted * Axis2));
			}
			return FMath::Sign(Check2) * ((DesiredAxis == EAxis::X) ? Axis2 : (Inverted * -Axis1));
		};

	FVector XAxis = ActorQuat.GetAxisX();
	FVector YAxis = ActorQuat.GetAxisY();
	FVector ZAxis = ActorQuat.GetAxisZ();
	if (FMath::Abs(ZAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(ZAxis, XAxis, YAxis);
	}
	else if (FMath::Abs(YAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(YAxis, ZAxis, XAxis);
	}
	return ProcessAxes(XAxis, YAxis, ZAxis);
}

FVector AInfiniteNudge_Subsystem::CalcViewAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat, EAxis::Type& FoundAxis, bool& Inverted)
{
	auto ProcessAxes = [&](const FVector& VAxis, const FVector& Axis1, const FVector& Axis2) -> FVector
		{
			int Inverted = FMath::Sign(VAxis.Z);
			if (DesiredAxis == EAxis::Z) {
				return Inverted * VAxis;
			}
			float Check1 = FVector(Axis1.X, Axis1.Y, 0.f).GetSafeNormal() | ViewVector;
			float Check2 = FVector(Axis2.X, Axis2.Y, 0.f).GetSafeNormal() | ViewVector;
			if (FMath::Abs(Check1) >= FMath::Abs(Check2)) {
				return FMath::Sign(Check1) * ((DesiredAxis == EAxis::X) ? Axis1 : (Inverted * Axis2));
			}
			return FMath::Sign(Check2) * ((DesiredAxis == EAxis::X) ? Axis2 : (Inverted * -Axis1));
		};

	FVector XAxis = ActorQuat.GetAxisX();
	FVector YAxis = ActorQuat.GetAxisY();
	FVector ZAxis = ActorQuat.GetAxisZ();
	if (FMath::Abs(ZAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(ZAxis, XAxis, YAxis);
	}
	else if (FMath::Abs(YAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(YAxis, ZAxis, XAxis);
	}
	return ProcessAxes(XAxis, YAxis, ZAxis);
}

int AInfiniteNudge_Subsystem::GetClosestLookAngle(AActor* actor, APlayerController* controller)
{
	if (actor && controller)
	{
		float inverseRotation = roundf(actor->GetActorRotation().GetInverse().Yaw);
		double inverseCameraRotation = controller->PlayerCameraManager->GetCameraRotation().Add(0, inverseRotation, 0).Yaw;
		//double inverseCameraRotation = controller->PlayerCameraManager->GetCameraRotation().Yaw;
		double angleToCheck = fmod(inverseCameraRotation, 360);
		//angleToCheck = angleToCheck - 180;
		if (angleToCheck < 0) angleToCheck += 360;

		if (angleToCheck >= double(0.0) && angleToCheck < double(90.0))
		{
			double firstNum = 0.0;
			double secondNum = 90.0;
			if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
			{
				return firstNum;
			}
			else
			{
				return secondNum;
			}
		}
		else if (angleToCheck >= double(90.0) && angleToCheck < double(180.0))
		{
			double firstNum = 90.0;
			double secondNum = 180.0;
			if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
			{
				return firstNum;
			}
			else
			{
				return secondNum;
			}
		}
		else if (angleToCheck >= double(180.0) && angleToCheck < double(270.0))
		{
			double firstNum = 180.0;
			double secondNum = 270.0;
			if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
			{
				return firstNum;
			}
			else
			{
				return secondNum;
			}
		}
		else if (angleToCheck >= double(270.0) && angleToCheck <= double(360.0))
		{
			double firstNum = 270.0;
			double secondNum = 360.0;
			if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
			{
				return firstNum;
			}
			else
			{
				return 0;
			}
		}
	}
	return -1;
}

FVector AInfiniteNudge_Subsystem::GetLookVector(int lookAngle)
{
	FVector lookVector = FVector(0, 0, 0);

	if (lookAngle == 0)
	{
		lookVector.X = 1;
	}
	else if (lookAngle == 180)
	{
		lookVector.X = -1;
	}
	else if (lookAngle == 90)
	{
		lookVector.Y = 1;
	}
	else if (lookAngle == 270)
	{
		lookVector.Y = -1;
	}

	return lookVector;
}

#pragma optimize("", on)