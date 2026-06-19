// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeothermalPurity.h"
#include "FGResourceNodeGeyser.h"
#include "Patching/NativeHookManager.h"
#include "FGGameState.h"

#define LOCTEXT_NAMESPACE "FGeothermalPurityModule"

void FGeothermalPurityModule::StartupModule()
{
#if !WITH_EDITOR	
	AFGResourceNodeGeyser* rng = GetMutableDefault<AFGResourceNodeGeyser>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNodeGeyser::BeginPlay, rng, [this](AFGResourceNodeGeyser* self)
		{
			auto state = Cast<AFGGameState>(self->GetWorld()->GetGameState());
			auto puritySettings = state->GetNodePuritySettings();
			switch (puritySettings)
			{
			case ENodePuritySettings::NPS_NoChange:
			{
				break;
			}
			case ENodePuritySettings::NPS_AllImpure:
			{
				self->mPurity = EResourcePurity::RP_Inpure;
				break;
			}
			case ENodePuritySettings::NPS_AllNormal:
			{
				self->mPurity = EResourcePurity::RP_Normal;
				break;
			}
			case ENodePuritySettings::NPS_AllPure:
			{
				self->mPurity = EResourcePurity::RP_Pure;
				break;
			}
			case ENodePuritySettings::NPS_AllRandom:
			{
				int32 RandomValue = FMath::RandRange(1, 3);
				if (RandomValue == 1)
				{
					self->mPurity = EResourcePurity::RP_Inpure;
				}
				else if (RandomValue == 2)
				{
					self->mPurity = EResourcePurity::RP_Normal;
				}
				else
				{
					self->mPurity = EResourcePurity::RP_Pure;
				}
				break;
			}
			case ENodePuritySettings::NPS_Decrease:
			{
				if (self->mPurity == EResourcePurity::RP_Normal)
				{
					self->mPurity = EResourcePurity::RP_Inpure;
				}
				else if (self->mPurity == EResourcePurity::RP_Pure)
				{
					self->mPurity = EResourcePurity::RP_Normal;
				}
				break;
			}
			case ENodePuritySettings::NPS_Increase:
			{
				if (self->mPurity == EResourcePurity::RP_Normal)
				{
					self->mPurity = EResourcePurity::RP_Pure;
				}
				else if (self->mPurity == EResourcePurity::RP_Inpure)
				{
					self->mPurity = EResourcePurity::RP_Normal;
				}
				break;
			}
			}
		});
#endif
}

void FGeothermalPurityModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGeothermalPurityModule, GeothermalPurity)