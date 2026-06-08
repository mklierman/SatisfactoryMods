#include "InfiniteNudgeModule.h"
#include <Patching/NativeHookManager.h>
#include <Buildables/FGBuildable.h>
#include "InfiniteNudge_ConfigurationStruct.h"
#include "FGInputLibrary.h"
#include "Hologram/FGRailroadSignalHologram.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Resources/FGBuildDescriptor.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "Hologram/FGPipeAttachmentHologram.h"
#include "InfiniteGizmo.h"
#include "ScrollMode.h"
#include <InfiniteNudge_RCO.h>



DEFINE_LOG_CATEGORY(InfiniteNudge);

//EScrollMode::Value EScrollMode::value = EScrollMode::RotateZ;

struct FAmount
{
	float Tiny;
	float Small;
	float Large;
	float Default; 
};


//Checks if the key for a mapping (and its modifiers if any) is pressed.
static bool isModifier(APlayerController* playerController, FName mapping)
{
	FKey key;
	TArray<FKey> modifiers;
	UFGInputLibrary::GetCurrentMappingForAction(playerController, mapping, key, modifiers);

	if (!key.IsValid() || !playerController->IsInputKeyDown(key))
		return false;

	for (const FKey& k : modifiers)
	{
		if (!playerController->IsInputKeyDown(k))
			return false;

	}
	return true;
}

//Gets the nudge/rotate/scale amount based on which modifier keys are being pressed.
static float GetModifierAmount(APlayerController* controller, FAmount amount)
{
	if (isModifier(controller, "InfiniteNudge.TinyNudge"))
		return amount.Tiny;
	if (isModifier(controller, "InfiniteNudge.SmallNudge"))
		return amount.Small;
	if (isModifier(controller, "InfiniteNudge.LargeNudge"))
		return amount.Large;

	return amount.Default;
}


void FInfiniteNudgeModule::StartupModule() {

#if !WITH_EDITOR		

	SUBSCRIBE_METHOD(UFGBuildDescriptor::GetHologramClass, [this](auto& scope, TSubclassOf<UFGBuildDescriptor> inClass)
		{
			auto original = scope(inClass);
			if (!original)
				return;

			if (auto replaced = mHologramOverrides.Find(original))
				scope.Override(*replaced);
		});

	auto holo = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::LockHologramPosition, holo, [this](auto& scope, const AFGHologram* self, bool lock)
		{
			mPivot = FVector::ZeroVector;
			if (self->mParent || self->mChildren.Num() > 0)
				return;
			
			if (lock) {
				ScrollMode::Reset();
				UInfiniteGizmo::Create(const_cast<AFGHologram*>(self));
			}
			else
				UInfiniteGizmo::Destroy();
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::GetNudgeDistance, holo, ([this](auto& scope, const AFGHologram* self)
		{
			auto controller = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
			if (!controller)
				return;

			auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(self->GetWorld());
			FAmount nudgeAmount = { config.TinyNudgeAmount, config.SmallNudgeAmount, config.LargeNudgeAmount, 100.0f };
			float distance = GetModifierAmount(controller, nudgeAmount);

			scope.Override(distance);
		}));

	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorLiftHologram::CanNudgeHologram, GetMutableDefault<AFGConveyorLiftHologram>(), [this](auto& scope, const AFGHologram* self)
		{
			scope.Override(true);
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::CanNudgeHologram, GetMutableDefault<AFGConveyorAttachmentHologram>(), [this](auto& scope, const AFGHologram* self)
		{
			scope.Override(true);
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::CanNudgeHologram, GetMutableDefault<AFGPipeAttachmentHologram>(), [this](auto& scope, const AFGHologram* self)
		{
			scope.Override(true);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, holo, [this](auto& scope, AFGHologram* self, int32 delta)
		{
			if (!self->IsHologramLocked())
				return;
			
			if (ScrollMode::IsRotate()) 
				FInfiniteNudgeModule::RotateHologram(self, delta);
			else if (ScrollMode::IsScale())
				FInfiniteNudgeModule::ScaleHologram(self, delta);
			//scope.Cancel();
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGRailroadSignalHologram::ConfigureActor, GetMutableDefault<AFGRailroadSignalHologram>(), [](const AFGRailroadSignalHologram* self, AFGBuildable* inBuildable)
		{
			if (!inBuildable || !self || !self->GetRootComponent())
				return;

			UE_LOG(InfiniteNudge, Verbose, TEXT("AFGRailroadSignalHologram::ConfigureActor overriden"));

			FVector ScaleVec = self->GetRootComponent()->GetComponentScale();
			float Scale = (ScaleVec.X + ScaleVec.Y + ScaleVec.Z) / 3.0f;
			
			float yLocation = -280.0f * Scale + 280.0f * (self->mIsLeftHanded ? -1.0f : 1.0f);		
			FVector location = FVector(0.0f, yLocation, 0.0f);
				
			if (!self->mUpgradeTarget)
				inBuildable->GetRootComponent()->AddLocalOffset(location);
			else if (self->mIsLeftHanded != self->mUpgradeTarget->mIsLeftHanded)
				inBuildable->GetRootComponent()->AddLocalOffset(2 * location);
		});

#endif
}

void FInfiniteNudgeModule::RotateHologram(AFGHologram* hologram, int32 step) {

	if (!ScrollMode::IsRotate())
		return;

	auto controller = Cast<APlayerController>(hologram->GetConstructionInstigator()->GetController());
	if (!controller)
		return;

	//Rotation from modifier
	auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(hologram->GetWorld());
	FAmount rotationAmount = { config.TinyRotateAmount, config.SmallRotateAmount, config.LargeRotateAmount, 15.0 };
	auto rotation = GetModifierAmount(controller, rotationAmount) * step;

	auto rotator = FRotator::ZeroRotator;
	rotator.SetComponentForAxis(ScrollMode::GetRotationAxis(), rotation);

	//pivot calculations
	if (!mPivot.IsNearlyZero()) { //if pivot is at 0,0,0, offset should be 0, so might as well skip the calculations
		FVector rotatedPivot = rotator.RotateVector(mPivot);
		FVector offset = hologram->GetActorTransform().TransformVector(mPivot - rotatedPivot);
		hologram->SetNudgeOffset(hologram->GetNudgeOffset() + offset); //Set because Add does additional checks and limits the distance
	} 
	
	//Apply rotation
	hologram->AddActorLocalRotation(rotator);
	for (auto c : hologram->mChildren)
		c->AddActorLocalRotation(rotator);
}

void FInfiniteNudgeModule::ScaleHologram(AFGHologram* hologram, int32 step) {

	if (!ScrollMode::IsScale())
		return;

	auto controller = Cast<AFGPlayerController>(hologram->GetConstructionInstigator()->GetController());
	if (!controller)
		return;

	//Scale from modifier
	auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(hologram->GetWorld());
	FAmount scaleAmount = { config.TinyScaleAmount, config.SmallScaleAmount, config.LargeScaleAmount, 1 };
	auto scale = GetModifierAmount(controller, scaleAmount) / 10 + 1; //change the config value to / 10 ?
	if (step < 0)
		scale = 1.0f / scale;
	
	//pivot calculations
	if (!mPivot.IsNearlyZero()) { 
		FVector offset = hologram->GetActorTransform().TransformVector(mPivot - mPivot * scale);
		hologram->SetNudgeOffset(hologram->GetNudgeOffset() + offset);
	}

	//Apply scale
	auto newScale = hologram->GetActorRelativeScale3D() * scale;
	hologram->SetActorRelativeScale3D(newScale);
	for (auto c : hologram->mChildren)
		c->SetActorRelativeScale3D(newScale);

	if (UInfiniteNudge_RCO* RCO = UInfiniteNudge_RCO::Get(hologram->GetWorld()))
		RCO->Server_SetHologramScale(controller, newScale);
}

IMPLEMENT_GAME_MODULE(FInfiniteNudgeModule, InfiniteNudge);
