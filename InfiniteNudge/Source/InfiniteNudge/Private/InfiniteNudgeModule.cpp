#include "InfiniteNudgeModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGResourceExtractorHologram.h"
#include "InfiniteNudge_ConfigurationStruct.h"
#include "FGPlayerController.h"
#include "Hologram/FGPipeAttachmentHologram.h"
#include "Hologram/FGWaterPumpHologram.h"
#include "Hologram/FGWireHologram.h"
#include "Hologram/FGWallAttachmentHologram.h"
#include "FGInputLibrary.h"
#include "FGGameMode.h"
#include "Hologram/FGRailroadSignalHologram.h"
#include <Hologram/FGSplineHologram.h>

#pragma optimize("", off)
void DoNudge(UFGBuildGunStateBuild* self, const FInputActionValue& actionValue)
{
	auto holo = self->GetHologram();
	auto canNudge = holo->CanNudgeHologram();
	auto locked = holo->IsHologramLocked();
	auto disabled = holo->IsDisabled();
	auto target = holo->GetNudgeHologramTarget();
	auto bg = self->GetBuildGun();
	auto hr = bg->GetHitResult();
	auto res = target->NudgeHologram(actionValue.Get<FVector>(), hr);
	self->OnHologramNudgeFailedDelegate.Broadcast(holo, res);
}

void FInfiniteNudgeModule::StartupModule() {


#if !WITH_EDITOR
	AFGHologram* bh = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::NudgeHologram, bh, [this](auto scope, const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
		{
			//NudgeHologram(self, NudgeInput, HitResult);
			scope.Override(ENudgeFailReason::NFR_Success);
		});

	AFGHologram* gbh = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::NudgeHologram, gbh, [this](auto scope, const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
		{
			scope.Override(ENudgeFailReason::NFR_Success);
		});

	AFGBuildableHologram* bhg = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHologram::BeginPlay, bh, [this](AFGHologram* self)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGenericBuildableHologram::BeginPlay, bh, [this](AFGGenericBuildableHologram* self)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::CanNudgeHologram, bh, [this](auto& scope, const AFGHologram* self)
		{
			scope.Override(true);
		});

	AFGWaterPumpHologram* wph = GetMutableDefault<AFGWaterPumpHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGWaterPumpHologram::PostHologramPlacement, wph, [this](auto& scope, AFGWaterPumpHologram* self, const FHitResult& hitResult, bool callForChildren)
		{
			if (self->IsHologramLocked())
			{
				scope.Cancel();
			}
		});

	//const FVector& GetNudgeOffset() const { return mHologramNudgeOffset; }
	SUBSCRIBE_METHOD(AFGHologram::LockHologramPosition, [this](auto scope, AFGHologram* self, bool lock)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
		});

	//ENudgeFailReason AFGHologram::AddNudgeOffset(const FVector & Offset, const FVector & MaxNudgeDistance) { return ENudgeFailReason(); }
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::AddNudgeOffset, bh, [this](auto& scope, AFGHologram* self, const FVector& Offset, const FVector& MaxNudgeDistance)
		{
			scope.Override(ENudgeFailReason::NFR_Success);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, bh, [this](auto& scope, AFGHologram* self, int32 delta)
		{
			if (self->IsHologramLocked())
			{
				RotateLockedHologram(self, delta);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadSignalHologram::ConfigureActor, GetMutableDefault<AFGRailroadSignalHologram>(), [](const AFGRailroadSignalHologram* self, AFGBuildable* inBuildable)
		{
			float Scale = 1.0f;
			bool isLeftHanded = false;

			if (self && self->GetRootComponent())
			{
				FVector ScaleVec = self->GetRootComponent()->GetComponentScale();
				Scale = (ScaleVec.X + ScaleVec.Y + ScaleVec.Z) / 3.0f;
				isLeftHanded = self->mIsLeftHanded;
			}

			if (inBuildable)
			{
				float yLocation = -280.0f * Scale + 280.0f;

				if (isLeftHanded)
				{
					yLocation = yLocation * -1.0f;
				}

				FVector location = FVector(0.0f, yLocation, 0.0f);
				inBuildable->GetRootComponent()->AddLocalOffset(location);
			}
		});
#endif

}

FVector FInfiniteNudgeModule::NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction)
{
	return Direction;
}

bool AreModifiersDown(APlayerController* contr, const TArray<FKey>& Modifiers)
{
	for (const FKey& Key : Modifiers)
	{
		if (!contr->IsInputKeyDown(Key))
		{
			return false;
		}
	}
	return true;
}

void FInfiniteNudgeModule::NudgeHologram(const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
{
	auto hg = const_cast<AFGHologram*>(self);
	auto contr = Cast<APlayerController>(hg->GetConstructionInstigator()->GetController());
	if (contr)
	{
		auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
		auto TinyNudgeAmount = (float)config.LeftCtrlNudgeAmount;
		auto SmallNudgeAmount = (float)config.LeftAltNudgeAmount;
		auto LargeNudgeAmount = (float)config.LargeNudgeAmount;

		FKey TinyNudgeKey;
		FKey SmallNudgeKey;
		FKey LargeNudgeKey;
		TArray<FKey> TinyModifierKeys;
		TArray<FKey> SmallModifierKeys;
		TArray<FKey> LargeModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, TinyModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, SmallModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.LargeNudge", LargeNudgeKey, LargeModifierKeys);

		if (TinyNudgeKey.IsValid() && contr->IsInputKeyDown(TinyNudgeKey) && AreModifiersDown(contr, TinyModifierKeys))
		{
			TinyNudgeAmount *= 2;
			hg->mDefaultNudgeDistance = TinyNudgeAmount;
		}
		else if (SmallNudgeKey.IsValid() && contr->IsInputKeyDown(SmallNudgeKey) && AreModifiersDown(contr, SmallModifierKeys))
		{
			SmallNudgeAmount *= 2;
			hg->mDefaultNudgeDistance = SmallNudgeAmount;
		}
		else if (LargeNudgeKey.IsValid() && contr->IsInputKeyDown(LargeNudgeKey) && AreModifiersDown(contr, LargeModifierKeys))
		{
			LargeNudgeAmount *= 2;
			hg->mDefaultNudgeDistance = LargeNudgeAmount;
		}
		else
		{
			hg->mDefaultNudgeDistance = 100.0;
		}
	}
}

void FInfiniteNudgeModule::NudgeGenericHologram(const AFGGenericBuildableHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
{

	auto hg = const_cast<AFGGenericBuildableHologram*>(self);
	auto contr = Cast<APlayerController>(hg->GetConstructionInstigator()->GetController());
	if (contr)
	{
		auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
		auto TinyNudgeAmount = (float)config.LeftCtrlNudgeAmount;
		auto SmallNudgeAmount = (float)config.LeftAltNudgeAmount;
		auto LargeNudgeAmount = (float)config.LargeNudgeAmount;

		FKey TinyNudgeKey;
		FKey SmallNudgeKey;
		FKey LargeNudgeKey;
		FKey VerticalNudgeKey;
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.LargeNudge", LargeNudgeKey, ModifierKeys);


		if (TinyNudgeKey.IsValid() && contr->IsInputKeyDown(TinyNudgeKey))
		{
			hg->mDefaultNudgeDistance = TinyNudgeAmount;
		}
		else if (SmallNudgeKey.IsValid() && contr->IsInputKeyDown(SmallNudgeKey))
		{
			hg->mDefaultNudgeDistance = SmallNudgeAmount;
		}
		else if (LargeNudgeKey.IsValid() && contr->IsInputKeyDown(LargeNudgeKey))
		{
			hg->mDefaultNudgeDistance = LargeNudgeAmount;
		}
		else
		{
			hg->mDefaultNudgeDistance = 100.0;
		}
	}
}

FVector FInfiniteNudgeModule::AddNudgeOffset(AFGHologram* self, const FVector& Direction)
{
	auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
	auto TinyNudgeAmount = (float)config.LeftCtrlNudgeAmount;
	auto SmallNudgeAmount = (float)config.LeftAltNudgeAmount;
	auto LargeNudgeAmount = (float)config.LargeNudgeAmount;

	FVector newVector = FVector(0, 0, 0);

	auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	if (contr)
	{
		// Get Key mappings
		FKey NudgeForwardKey;
		FKey NudgeBackwardKey;
		FKey TinyNudgeKey;
		FKey SmallNudgeKey;
		FKey LargeNudgeKey;
		FKey VerticalNudgeKey;
		
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "BuildGunBuild_NudgeForward", NudgeForwardKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "BuildGunBuild_NudgeBackward", NudgeBackwardKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.LargeNudge", LargeNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.VerticalNudge", VerticalNudgeKey, ModifierKeys);

		auto genericHolo = Cast < AFGGenericBuildableHologram>(self);
		if (genericHolo)
		{
			if (contr->IsInputKeyDown(VerticalNudgeKey) && (contr->IsInputKeyDown(NudgeForwardKey) || contr->IsInputKeyDown(NudgeBackwardKey)))
			{

				// Determine "Front" of holo
				auto snapAxis = genericHolo->mSnapAxis;
				FVector offset = FVector(0, 0, 0);
				switch (snapAxis)
				{
				case EAxis::X:
					offset = self->GetActorForwardVector();
					break;

				case EAxis::Y:
					offset = self->GetActorRightVector();
					break;

				case EAxis::Z:
					offset = self->GetActorUpVector();
					break;

				default:
					break;
				}

				if (contr->IsInputKeyDown(TinyNudgeKey))
				{
					newVector = offset * FVector(TinyNudgeAmount);
				}
				else if (contr->IsInputKeyDown(SmallNudgeKey))
				{
					newVector = offset * FVector(SmallNudgeAmount);
				}
				else if (contr->IsInputKeyDown(LargeNudgeKey))
				{
					newVector = offset * FVector(LargeNudgeAmount);
				}
				else
				{
					newVector = offset * 100;
				}

				if (contr->IsInputKeyDown(NudgeForwardKey))
				{
					return newVector;
				}
				else if (contr->IsInputKeyDown(NudgeBackwardKey))
				{
					return newVector * -1;
				}
			}
		}

		if (contr->IsInputKeyDown(VerticalNudgeKey) && (contr->IsInputKeyDown(NudgeForwardKey) || contr->IsInputKeyDown(NudgeBackwardKey)))
		{
			newVector.Z = 100;

			if (contr->IsInputKeyDown(TinyNudgeKey))
			{
				newVector.Z = TinyNudgeAmount;
			}
			else if (contr->IsInputKeyDown(SmallNudgeKey))
			{
				newVector.Z = SmallNudgeAmount;
			}
			else if (contr->IsInputKeyDown(LargeNudgeKey))
			{
				newVector.Z = LargeNudgeAmount;
			}

			if (contr->IsInputKeyDown(NudgeForwardKey))
			{
				return newVector;
			}
			else if (contr->IsInputKeyDown(NudgeBackwardKey))
			{
				newVector.Z = newVector.Z * -1;
				return newVector;
			}
		}
	}
	return newVector;
}

void FInfiniteNudgeModule::RotateLockedHologram(AFGHologram* self, int32 delta)
{
	auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	if (contr)
	{
		auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
		auto TinyRotateAmount = (float)config.TinyRotateAmount;
		auto SmallRotateAmount = (float)config.SmallRotateAmount;
		auto LargeRotateAmount = (float)config.LargeRotateAmount;

		FKey TinyNudgeKey;
		FKey SmallNudgeKey;
		FKey LargeNudgeKey;
		FKey PitchRotateKey;
		FKey RollRotateKey;
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.LargeNudge", LargeNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.PitchRotate", PitchRotateKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.RollRotate", RollRotateKey, ModifierKeys);

		//auto newRotation = self->GetActorRotation();
		auto newRotation = FRotator(0, 0, 0);
		double rotationAmount = 15.0 * delta;

		// Set Fine Rotation
		if (contr->IsInputKeyDown(TinyNudgeKey))
		{
			rotationAmount = TinyRotateAmount * delta;
		}
		if (contr->IsInputKeyDown(SmallNudgeKey))
		{
			rotationAmount = SmallRotateAmount * delta;
		}
		if (contr->IsInputKeyDown(LargeNudgeKey))
		{
			rotationAmount = LargeRotateAmount * delta;
		}

		// Set rotation types

		auto pipeAttachHolo = Cast<AFGPipeAttachmentHologram>(self);
		auto wallAttachmentHolo = Cast<AFGWallAttachmentHologram>(self);
		if (pipeAttachHolo || wallAttachmentHolo)
		{
			// Pipe attachments rotate roll by default
			if (contr->IsInputKeyDown(PitchRotateKey))
			{
				//Rotate Pitch
				newRotation.Pitch = newRotation.Pitch + rotationAmount;
			}
			else if (contr->IsInputKeyDown(RollRotateKey))
			{
				//Rotate Yaw
				newRotation.Yaw = newRotation.Yaw + rotationAmount;
			}
			else
			{
				//Rotate Roll
				newRotation.Roll = newRotation.Roll + rotationAmount;
			}
		}
		else
		{
			if (contr->IsInputKeyDown(PitchRotateKey))
			{
				//Rotate Pitch
				newRotation.Pitch = newRotation.Pitch + rotationAmount;
			}
			else if (contr->IsInputKeyDown(RollRotateKey))
			{
				//Rotate Roll
				newRotation.Roll = newRotation.Roll + rotationAmount;
			}
			else
			{
				//Rotate Yaw
				newRotation.Yaw = newRotation.Yaw + rotationAmount;
			}
		}
		auto wireHG = Cast< AFGWireHologram>(self);
		if (wireHG)
		{
			auto currentPole = wireHG->GetActiveAutomaticPoleHologram();
			if (currentPole)
			{
				currentPole->AddActorLocalRotation(newRotation);
			}
		}
		else
		{
			self->AddActorLocalRotation(newRotation);
		}
	}
}
void FInfiniteNudgeModule::ScaleHologram(AFGHologram* self)
{
	auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	if (contr)
	{
		auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
		auto TinyScaleAmount = (float)config.TinyScaleAmount;
		auto SmallScaleAmount = (float)config.SmallScaleAmount;
		auto LargeScaleAmount = (float)config.LargeScaleAmount;

		FKey IncreaseScaleKey;
		FKey DecreaseScaleKey;
		FKey TinyNudgeKey;
		FKey SmallNudgeKey;
		FKey LargeNudgeKey;
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.IncreaseScale", IncreaseScaleKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.DecreaseScale", DecreaseScaleKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.LargeNudge", LargeNudgeKey, ModifierKeys);

		float scaleAmount = 1.0;
		if (contr->IsInputKeyDown(TinyNudgeKey))
		{
			scaleAmount = TinyScaleAmount;
		}
		else if (contr->IsInputKeyDown(SmallNudgeKey))
		{
			scaleAmount = SmallScaleAmount;
		}
		else if (contr->IsInputKeyDown(LargeNudgeKey))
		{
			scaleAmount = LargeScaleAmount;
		}
		if (contr->IsInputKeyDown(DecreaseScaleKey))
		{
			scaleAmount = scaleAmount * -1.0;
		}

		auto currentScale = self->GetActorRelativeScale3D();
		self->SetActorRelativeScale3D(currentScale + scaleAmount);
	}
}


#pragma optimize("", on)
IMPLEMENT_GAME_MODULE(FInfiniteNudgeModule, InfiniteNudge);