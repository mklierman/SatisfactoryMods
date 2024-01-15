#include "InfiniteNudge_Subsystem.h"
DEFINE_LOG_CATEGORY(InfiniteNudge_Log);
#pragma optimize("", off)
//Get Building direction closest to camera direction

void AInfiniteNudge_Subsystem::NudgeHologram(AFGHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	if (hologram && hologram->IsHologramLocked())
	{
		GetConfigValues(hologram->GetWorld(), controller);
		if (auto beltHolo = Cast< AFGConveyorBeltHologram>(hologram))
		{
			NudgeBelt(beltHolo, xDirection, yDirection, controller);
		}
		else if (auto pipeHolo = Cast<AFGPipelineHologram>(hologram))
		{
			NudgePipe(pipeHolo, xDirection, yDirection, controller);
		}
		else if (auto wireHolo = Cast<AFGWireHologram>(hologram))
		{
			NudgeWire(wireHolo, xDirection, yDirection, controller);
		}
		else if (auto trackHolo = Cast<AFGRailroadTrackHologram>(hologram))
		{
			NudgeRailroadTrack(trackHolo, xDirection, yDirection, controller);
		}
		else if (auto wallAttachmentHolo = Cast<AFGWallAttachmentHologram>(hologram))
		{
			NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, controller);
		}
		else if (auto signHolo = Cast<AFGStandaloneSignHologram>(hologram))
		{
			NudgeSign(signHolo, xDirection, yDirection, controller);
		}
		else if (controller->IsInputKeyDown(VerticalNudgeKey))
		{
			hologram->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
		}
		else
		{
			int lookAngle = GetClosestLookAngle(hologram, controller);
			FVector lookVector = GetLookVector(lookAngle);
			FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));

			hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
		}
		hologram->mHologramLockLocation = hologram->GetActorLocation();
	}
}

void AInfiniteNudge_Subsystem::NudgeBelt(AFGConveyorBeltHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (auto wallAttachmentHolo = Cast< AFGWallAttachmentHologram>(nudgeTarget))
	{
		NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, controller);
	}
	else if (controller->IsInputKeyDown(VerticalNudgeKey))
	{
		nudgeTarget->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
	}
	else
	{
		int lookAngle = GetClosestLookAngle(nudgeTarget, controller);
		FVector lookVector = GetLookVector(lookAngle);
		FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));

		nudgeTarget->AddActorLocalOffset((rVectorz) * GetCurrentNudgeAmount(controller));
	}
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

void AInfiniteNudge_Subsystem::NudgePipe(AFGPipelineHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (auto wallAttachmentHolo = Cast< AFGWallAttachmentHologram>(nudgeTarget))
	{
		NudgeWallAttachment(wallAttachmentHolo, xDirection, yDirection, controller);
	}
	else if (controller->IsInputKeyDown(VerticalNudgeKey))
	{
		nudgeTarget->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
	}
	else
	{
		int lookAngle = GetClosestLookAngle(nudgeTarget, controller);
		FVector lookVector = GetLookVector(lookAngle);
		FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));

		nudgeTarget->AddActorLocalOffset((rVectorz) * GetCurrentNudgeAmount(controller));
	}
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

void AInfiniteNudge_Subsystem::NudgeWire(AFGWireHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (controller->IsInputKeyDown(VerticalNudgeKey))
	{
		nudgeTarget->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
	}
	else
	{
		int lookAngle = GetClosestLookAngle(nudgeTarget, controller);
		FVector lookVector = GetLookVector(lookAngle);
		FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));

		nudgeTarget->AddActorLocalOffset((rVectorz * -1) * GetCurrentNudgeAmount(controller));
	}
	nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
}

void AInfiniteNudge_Subsystem::NudgeRailroadTrack(AFGRailroadTrackHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	auto nudgeTarget = hologram->GetNudgeHologramTarget();
	if (controller->IsInputKeyDown(VerticalNudgeKey))
	{
		hologram->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
	}
	else
	{
		int lookAngle = GetClosestLookAngle(hologram, controller);
		FVector lookVector = GetLookVector(lookAngle);
		FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
		hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
	}
	hologram->mHologramLockLocation = hologram->GetActorLocation();
}

void AInfiniteNudge_Subsystem::NudgeWallAttachment(AFGWallAttachmentHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	if (controller->IsInputKeyDown(VerticalNudgeKey))
	{
		int lookAngle = GetClosestLookAngle(hologram, controller);
		FVector lookVector = GetLookVector(lookAngle);
		FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
		hologram->AddActorLocalOffset(FVector(GetCurrentNudgeAmount(controller) * (xDirection * -1), 0, 0));
	}
	else
	{
		if (xDirection != 0)
		{
			int lookAngle = GetClosestLookAngle(hologram, controller);
			FVector lookVector = GetLookVector(lookAngle);
			FVector rVectorz = FVector(0, yDirection, (xDirection * -1)).RotateAngleAxis(lookAngle, FVector(0, 1, 0));
			hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
		}
		else
		{
			int lookAngle = GetClosestLookAngle(hologram, controller);
			FVector lookVector = GetLookVector(lookAngle);
			FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
			hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
		}
	}
	hologram->mHologramLockLocation = hologram->GetActorLocation();
}

void AInfiniteNudge_Subsystem::NudgeSign(AFGStandaloneSignHologram* hologram, float xDirection, float yDirection, AFGPlayerController* controller)
{
	if (auto nudgeTarget = Cast<AFGSignPoleHologram>(hologram->GetNudgeHologramTarget()))
	{
		//Standard nudging	auto nudgeTarget = hologram->GetNudgeHologramTarget();
		if (controller->IsInputKeyDown(VerticalNudgeKey))
		{
			nudgeTarget->AddActorLocalOffset(FVector(0, 0, GetCurrentNudgeAmount(controller) * xDirection));
		}
		else
		{
			int lookAngle = GetClosestLookAngle(nudgeTarget, controller);
			FVector lookVector = GetLookVector(lookAngle);
			FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));

			nudgeTarget->AddActorLocalOffset((rVectorz) * GetCurrentNudgeAmount(controller));
		}
		nudgeTarget->mHologramLockLocation = nudgeTarget->GetActorLocation();
	}
	else
	{
		//Wall attachment nudging. Isn't actually a wall attachment holo, though
		if (controller->IsInputKeyDown(VerticalNudgeKey))
		{
			int lookAngle = GetClosestLookAngle(hologram, controller);
			FVector lookVector = GetLookVector(lookAngle);
			FVector rVectorz = FVector(xDirection, yDirection, 0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
			hologram->AddActorLocalOffset((rVectorz) * GetCurrentNudgeAmount(controller));
		}
		else
		{
			if (xDirection != 0)
			{
				int lookAngle = GetClosestLookAngle(hologram, controller);
				FVector lookVector = GetLookVector(lookAngle);
				FVector rVectorz = FVector(0, yDirection, xDirection).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
			}
			else
			{
				int lookAngle = GetClosestLookAngle(hologram, controller);
				FVector lookVector = GetLookVector(lookAngle);
				FVector rVectorz = FVector(xDirection, yDirection,0).RotateAngleAxis(lookAngle, FVector(0, 0, 1));
				hologram->AddActorLocalOffset(rVectorz * GetCurrentNudgeAmount(controller));
			}
		}
	}
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
				auto currentScale = hologram->GetActorRelativeScale3D();
				auto newScale = currentScale + (scaleAmount / 100);
				//UE_LOG(InfiniteNudge_Log, Display, TEXT("ScaleHologram - NewScale: %s"), *newScale.ToString());
				//UE_LOG(InfiniteNudge_Log, Display, TEXT("ScaleHologram - NewScale: %s"), *newScale.ToString());
				//auto netMode = hologram->GetNetMode();
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

				//auto currentTransform = hologram->GetActorTransform();
				//FTransform newTransform = currentTransform;
				//newTransform.SetScale3D(currentScale + (scaleAmount / 100));
				//hologram->SetActorTransform(newTransform);
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
	if (controller->IsInputKeyDown(TinyNudgeKey))
	{
		return TinyNudgeAmount;
	}
	else if (controller->IsInputKeyDown(SmallNudgeKey))
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