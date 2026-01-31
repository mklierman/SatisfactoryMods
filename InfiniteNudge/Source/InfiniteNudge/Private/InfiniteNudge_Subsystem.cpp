#include "InfiniteNudge_Subsystem.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif

DEFINE_LOG_CATEGORY(InfiniteNudge_Log);
#pragma optimize("", off)

namespace
{
	// Apply vector as world or local offset
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

	// Compute nudge vector using look angle
	static FVector ComputeNudgeVector(AInfiniteNudge_Subsystem* subsystem, AActor* target, float xDirection, float yDirection, AFGPlayerController* controller)
	{
		if (!subsystem || !target)
		{
			return FVector(xDirection, yDirection, 0);
		}
		int lookAngle = subsystem->GetClosestLookAngle(target, controller);
		return FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
	}

	// Apply nudge with optional custom vector calculator
	static void ApplyNudgeToTarget(AInfiniteNudge_Subsystem* subsystem, AActor* target, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller, TFunction<FVector(AActor*, float, float, AFGPlayerController*)> computeVec = nullptr)
	{
		if (!subsystem || !target || !controller)
		{
			return;
		}

		const float amount = subsystem->GetCurrentNudgeAmount(controller);
		const bool useWorld = subsystem->ShouldWorldOffset(controller);

		// Vertical nudge
		if (controller->IsInputKeyDown(subsystem->VerticalNudgeKey) || controller->IsInputKeyDown(EKeys::Gamepad_RightShoulder))
		{
			ApplyOffset(target, FVector(0, 0, xDirection), amount, useWorld);
			return;
		}

		// Z direction
		if (zDirection != 0.0f)
		{
			ApplyOffset(target, FVector(0, 0, zDirection), amount, useWorld);
			return;
		}

		// XY nudge with custom or default vector
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
			if (ShouldWorldOffset(controller))
			{
				hologram->GetNudgeHologramTarget()->AddActorWorldOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * zDirection));
			}
			else
			{
				// Handle wall attachment's "vertical" movement
				if (auto wallAttachmentHolo = Cast<AFGWallAttachmentHologram>(hologram))
				{
					auto offset = FVector(1, 0, 0);
					auto snapAxis = wallAttachmentHolo->GetmSnapAxis();
					switch (snapAxis)
					{
					case EAxis::X:
						offset = FVector(1, 0, 0);
						break;

					case EAxis::Y:
						offset = FVector(0, 1, 0);
						break;

					case EAxis::Z:
						offset = FVector(0, 0, 1);
						break;

					default:
						break;
					}
					hologram->GetNudgeHologramTarget()->AddActorLocalOffset(offset * (GetCurrentNudgeAmount(controller) * -zDirection));
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
			if (auto wallAttachmentHolo = Cast< AFGWallAttachmentHologram>(wireHolo->GetNudgeHologramTarget()))
			{
				auto target = wireHolo->GetNudgeHologramTarget();
				auto location = target->GetActorLocation();
				auto rotation = target->GetActorRotation();
				if (rotation.Pitch == -90.0)
				{
					//Is on ceiling
					NudgeCeilingAttachment(wallAttachmentHolo, xDirection, yDirection, zDirection, controller);
				}
				else
				{
					NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, zDirection, controller);
				}
			}
			else
			{
				NudgeWire(wireHolo, xDirection, yDirection, zDirection, controller);
			}
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
			ApplyOffset(hologram, rVectorz, GetCurrentNudgeAmount(controller), ShouldWorldOffset(controller));
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
	if (!controller->IsInputKeyDown(VerticalNudgeKey))
	{
		// Map input based on snap axis and handle ceiling vs wall orientation
		auto compute = [this, xDirection](AActor* target, float x, float y, AFGPlayerController* pc) -> FVector
			{
				FRotator rot = target->GetActorRotation();
				FVector v;
				auto holo = Cast<AFGGenericBuildableHologram>(target);
				auto snapAxis = holo->GetmSnapAxis();
				
				switch (snapAxis)
				{
				case EAxis::X:
					v = FVector(0.0f, y, x);
					break;
				case EAxis::Y:
					v = FVector(0.0f, y, x);
					break;
				case EAxis::Z:
					v = FVector(x, y, 0.0f);
					break;
				default:
					break;
				}
				
				// Flip Y for walls (not ceilings)
				if (!FMath::IsNearlyEqual(rot.Pitch, -90, 0.01f))
				{
					v = v * FVector(1, -1, 1);
				}
				return v;
			};

		ApplyNudgeToTarget(this, hologram, xDirection, yDirection, zDirection, controller, compute);
		hologram->mHologramLockLocation = hologram->GetActorLocation();
	}
}

void AInfiniteNudge_Subsystem::NudgeCeilingAttachment(AFGWallAttachmentHologram* hologram, float xDirection, float yDirection, float zDirection, AFGPlayerController* controller)
{
	if (!controller->IsInputKeyDown(VerticalNudgeKey))
	{
		// Ceiling attachments need special handling - swap axes and fix signs at 0/180 degrees
		auto compute = [this](AActor* target, float x, float y, AFGPlayerController* pc) -> FVector
		{
			int lookAngle = this->GetClosestLookAngle(target, pc);
			auto holo = Cast<AFGGenericBuildableHologram>(target);
			
			if (!holo)
			{
				return FVector(y, x, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
			}
			
			auto snapAxis = holo->GetmSnapAxis();
			FVector v;
			
			// Fix reversed controls at certain angles
			float xSign = (lookAngle == 0 || lookAngle == 180) ? -1.0f : 1.0f;
			float ySign = (lookAngle == 0 || lookAngle == 180) ? -1.0f : 1.0f;
			
			switch (snapAxis)
			{
			case EAxis::X:
				v = FVector(y * ySign, x * xSign, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				v = FVector(0.0f, v.X, v.Y);
				break;

			case EAxis::Y:
				v = FVector(y * ySign, x * xSign, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				v = FVector(v.X, 0.0f, v.Y);
				break;

			case EAxis::Z:
				v = FVector(y * ySign, x * xSign, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				break;

			default:
				v = FVector(y * ySign, x * xSign, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				break;
			}
			
			return v;
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
	UFGInputLibrary::GetCurrentMappingForAction(controller, "InfiniteNudge.WorldOffset", WorldOffsetKey, ModifierKeys);

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

bool AInfiniteNudge_Subsystem::ShouldWorldOffset(APlayerController* controller)
{
	if (controller->IsInputKeyDown(WorldOffsetKey))
	{
		return true;
	}
	return false;

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