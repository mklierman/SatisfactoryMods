#include "InfiniteNudgeModule.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGResourceExtractorHologram.h"
#include "InfiniteNudge_ConfigurationStruct.h"
#include "FGPlayerController.h"
#include "Hologram/FGPipeAttachmentHologram.h"
#include "Hologram/FGWireHologram.h"
#include "FGInputLibrary.h"

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
	//ENudgeFailReason AFGHologram::NudgeHologram(const FVector& NudgeInput, const FHitResult& HitResult){ return ENudgeFailReason(); }
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::NudgeHologram, bh, [=](auto scope, const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
		{
			NudgeHologram(self, NudgeInput, HitResult);
		});


	AFGGenericBuildableHologram* gbh = GetMutableDefault<AFGGenericBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGGenericBuildableHologram::NudgeHologram, gbh, [=](auto scope, const AFGGenericBuildableHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
		{
			NudgeGenericHologram(self, NudgeInput, HitResult);
		});

	AFGBuildableHologram* bhg = GetMutableDefault<AFGBuildableHologram>();

	//AFGPipeAttachmentHologram* pahg = GetMutableDefault<AFGPipeAttachmentHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::CanNudgeHologram, pahg, [=](auto scope, const AFGPipeAttachmentHologram* self)
	//	{
	//		DebugChecker();
	//		scope.Override(true);
	//	});

	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::CanNudgeHologram, bhg, [=](auto& scope, const AFGBuildableHologram* self)
	//	{
	//		scope.Override(true);
	//	});

	//AFGResourceExtractorHologram* rehg = GetMutableDefault<AFGResourceExtractorHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGResourceExtractorHologram::CanNudgeHologram, rehg, [=](auto& scope, const AFGResourceExtractorHologram* self)
	//	{
	//		scope.Override(true);
	//	});


	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::GetNudgeHologramTarget, bh, [=](auto& scope, AFGHologram* self)
	//	{
	//		scope.Override(self);
	//	});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHologram::BeginPlay, bh, [=](AFGHologram* self)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
			self->mMaxNudgeDistance = 50000;
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGenericBuildableHologram::BeginPlay, bh, [=](AFGGenericBuildableHologram* self)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
			self->mMaxNudgeDistance = 50000;
		});

	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::CanNudgeHologram, bh, [=](auto& scope, const AFGHologram* self)
	//	{
	//		scope.Override(true);
	//	});

	//const FVector& GetNudgeOffset() const { return mHologramNudgeOffset; }
	SUBSCRIBE_METHOD(AFGHologram::LockHologramPosition, [=](auto scope, AFGHologram* self, bool lock)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
		});
		//const FVector& GetNudgeOffset() const { return mHologramNudgeOffset; }
	SUBSCRIBE_METHOD(AFGHologram::GetMaxNudgeDistance, [=](auto scope, const AFGHologram* self)
		{
			FVector ret = FVector(50000, 50000, 50000);
			scope.Override(ret);
			//auto hg = Cast<AFGHologram>(self);
			//hg->mHologramNudgeOffset.X = 0;
			//hg->mHologramNudgeOffset.Y = 0;
			//hg->mHologramNudgeOffset.Z = 0;
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::AddNudgeOffset, bh, [=](auto& scope, AFGHologram* self, const FVector& Direction)
		{
			FVector newVector = AddNudgeOffset(self, Direction);
			if (newVector.X != 0 || newVector.Y != 0 || newVector.Z != 0)
			{
				ENudgeFailReason fr = scope(self, newVector);
				scope.Override(fr);
			}
		});

	//virtual ENudgeFailReason NudgeHologram( const FVector& NudgeInput, const FHitResult& HitResult );
	// 
	///Hooking this causes weird bugs. Need to find another spot?
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::SetNudgeOffset, bh, [=](auto& scope, AFGHologram* self, const FVector& NewNudgeOffset)
	//	{
	//		savedNudgeDistance = self->mDefaultNudgeDistance;
	//		auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	//		if (contr && contr->IsInputKeyDown(EKeys::LeftControl))
	//		{
	//			self->mDefaultNudgeDistance = 20.0;
	//		}
	//		else if (contr && contr->IsInputKeyDown(EKeys::LeftAlt))
	//		{
	//			self->mDefaultNudgeDistance = 50.0;
	//		}
	//		else
	//		{
	//			self->mDefaultNudgeDistance = 100.0;
	//		}
	//	});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, bh, [=](auto& scope, AFGHologram* self, int32 delta)
		{
			if (self->IsHologramLocked())
			{
				//DebugChecker();
				RotateLockedHologram(self, delta);
			}
		});
#endif
}

FVector FInfiniteNudgeModule::NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction)
{
	return Direction;
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
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.BigNudge", LargeNudgeKey, ModifierKeys);

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
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyNudge", TinyNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallNudge", SmallNudgeKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.BigNudge", LargeNudgeKey, ModifierKeys);

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
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.BigNudge", LargeNudgeKey, ModifierKeys);
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

		FKey TinyRotateKey;
		FKey SmallRotateKey;
		FKey PitchRotateKey;
		FKey RollRotateKey;
		TArray<FKey> ModifierKeys;
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.TinyRotate", TinyRotateKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.SmallRotate", SmallRotateKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.PitchRotate", PitchRotateKey, ModifierKeys);
		UFGInputLibrary::GetCurrentMappingForAction(contr, "InfiniteNudge.RollRotate", RollRotateKey, ModifierKeys);

		//auto newRotation = self->GetActorRotation();
		auto newRotation = FRotator(0, 0, 0);
		int rotationAmount = 15 * delta;

		// Set Fine Rotation
		if (contr->IsInputKeyDown(TinyRotateKey))
		{
			rotationAmount = TinyRotateAmount * delta;
		}
		if (contr->IsInputKeyDown(SmallRotateKey))
		{
			rotationAmount = SmallRotateAmount * delta;
		}

		// Set rotation types

		auto pipeAttachHolo = Cast<AFGPipeAttachmentHologram>(self);
		if (pipeAttachHolo)
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
#pragma optimize("", off)
void FInfiniteNudgeModule::DebugChecker()
{
	FString thisThing = "Hi";
	FString thatThing = "Hello";
	auto themThings = thisThing + thatThing;
}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FInfiniteNudgeModule, InfiniteNudge);