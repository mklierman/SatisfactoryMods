#include "LongerBeamsModule.h"
#include "Hologram/FGPillarHologram.h"
#include "Hologram/FGBeamHologram.h"
#include "FGPlayerController.h"
#include "FGCharacterPlayer.h"
#include "Patching/NativeHookManager.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGHologramBuildModeDescriptor.h"
#include "LongerBeams_ConfigStruct.h"

//#pragma optimize("", off)
void FLongerBeamsModule::StartupModule() 
{
//#endif
#if !WITH_EDITOR
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, hCDO, [=](auto scope, AFGHologram* self, int32 delta)
		{
			if (!self->IsHologramLocked())
			{
				if (auto beamhg = Cast<AFGBeamHologram>(self))
				{
					auto pc = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
					if (pc->IsInputKeyDown(EKeys::LeftShift))
					{
						ScrollingBeams.AddUnique(beamhg);
						this->ScrollHologram(beamhg, delta);
						scope.Cancel();
					}
				}
			}
		});
	
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Destroyed, hCDO, [=](auto& scope, AFGHologram* self)
		{
			if (auto beamhg = Cast<AFGBeamHologram>(self))
			{

				if (ScrollingBeams.Contains(beamhg))
				{
					ScrollingBeams.Remove(beamhg);
				}
			}
		});

	AFGHologram* bhCDO = GetMutableDefault<AFGBeamHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBeamHologram::SetHologramLocationAndRotation, bhCDO, [=](auto& scope, AFGBeamHologram* self, const FHitResult& hitResult)
		{
			if (ScrollingBeams.Contains(self))
			{
				scope.Cancel();
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGBeamHologram::GetRotationStep, bhCDO, [=](auto& scope, const AFGBeamHologram* self)
		{
			auto pc = Cast<AFGPlayerController>(self->GetWorld()->GetFirstPlayerController());
			if (!pc->IsInputKeyDown(EKeys::LeftShift) && pc->IsInputKeyDown(EKeys::LeftControl))
			{
				auto config = FLongerBeams_ConfigStruct::GetActiveConfig(self->GetWorld());
				auto rotAmount = config.RotationAmount;
				scope.Override(rotAmount);
			}
		});

	SUBSCRIBE_METHOD(UFGBuildGunState::SecondaryFire, [=](auto& scope, UFGBuildGunState* self)
		{
			if (auto bgsb = Cast<UFGBuildGunStateBuild>(self))
			{
				auto hologram = bgsb->GetHologram();
				if (hologram)
				{
					if (auto beamhg = Cast<AFGBeamHologram>(hologram))
					{
						if (ScrollingBeams.Contains(beamhg))
						{
							ScrollingBeams.Remove(beamhg);
							scope.Cancel();
						}
					}
				}
			}
		});
//#if !WITH_EDITOR
#endif
}

void FLongerBeamsModule::ScrollHologram(AFGBeamHologram* self, int32 delta)
{
	auto currentLength = self->mCurrentLength;
	auto pc = Cast<AFGPlayerController>(self->GetWorld()->GetFirstPlayerController());
	if (pc->IsInputKeyDown(EKeys::LeftShift) && !pc->IsInputKeyDown(EKeys::LeftControl))
	{
		auto desiredLength = currentLength + (delta * 100);
		if (desiredLength > 0)
		{
			self->SetCurrentLength(currentLength + (delta * 100));
		}
	}
	else if (pc->IsInputKeyDown(EKeys::LeftShift) && pc->IsInputKeyDown(EKeys::LeftControl))
	{
		auto config = FLongerBeams_ConfigStruct::GetActiveConfig(self->GetWorld());
		auto length = config.ScrollAmount * 100;
		auto desiredLength = currentLength + (delta * length);
		if (desiredLength > 0)
		{
			self->SetCurrentLength(currentLength + (delta * length));
		}
	}
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FLongerBeamsModule, LongerBeams);