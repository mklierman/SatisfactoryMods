// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeothermalPurity.h"
#include "FGGameState.h"
#include "FGResourceNodeGeyser.h"
#include "Logging/StructuredLog.h"
#include "Patching/NativeHookManager.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FGeothermalPurityModule"

DEFINE_LOG_CATEGORY(GeothermalPurity_Log);

void FGeothermalPurityModule::StartupModule()
{
#if !WITH_EDITOR	
	AFGResourceNodeGeyser* rng = GetMutableDefault<AFGResourceNodeGeyser>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNodeGeyser::BeginPlay, rng, [this](AFGResourceNodeGeyser* self)
		{
			SetGeyserPurity(self);
		});
#endif
}

void FGeothermalPurityModule::SetGeyserPurity(AFGResourceNodeGeyser* geyser)
{
	auto state = Cast<AFGGameState>(geyser->GetWorld()->GetGameState());
	if (!state)
	{
		return;
	}

	auto puritySettings = state->GetNodePuritySettings();
	auto randomSeed = state->GetNodeRandomizationSeed();
	//UE_LOGFMT(GeothermalPurity_Log, Display, "Purity settings for {0}: {1}", geyser->GetName(), UEnum::GetValueAsString(puritySettings));
	switch (puritySettings)
	{
	case ENodePuritySettings::NPS_NoChange:
	{
		break;
	}
	case ENodePuritySettings::NPS_AllImpure:
	{
		geyser->SetResourcePurity(EResourcePurity::RP_Inpure);
		geyser->SetResourcePurityOverride(EResourcePurity::RP_Inpure);
		break;
	}
	case ENodePuritySettings::NPS_AllNormal:
	{
		geyser->SetResourcePurity(EResourcePurity::RP_Normal);
		geyser->SetResourcePurityOverride(EResourcePurity::RP_Normal);
		break;
	}
	case ENodePuritySettings::NPS_AllPure:
	{
		geyser->SetResourcePurity(EResourcePurity::RP_Pure);
		geyser->SetResourcePurityOverride(EResourcePurity::RP_Pure);
		break;
	}
	case ENodePuritySettings::NPS_AllRandom:
	{
		FRandomStream randomStream(randomSeed);
		int32 randomValue = randomStream.RandRange(1, 3);
		if (randomValue == 1)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Inpure);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Inpure);
		}
		else if (randomValue == 2)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Normal);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Normal);
		}
		else
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Pure);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Pure);
		}
		break;
	}
	case ENodePuritySettings::NPS_Decrease:
	{
		if (geyser->GetResourcePurity() == EResourcePurity::RP_Normal)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Inpure);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Inpure);
		}
		else if (geyser->GetResourcePurity() == EResourcePurity::RP_Pure)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Normal);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Normal);
		}
		break;
	}
	case ENodePuritySettings::NPS_Increase:
	{
		if (geyser->GetResourcePurity() == EResourcePurity::RP_Normal)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Pure);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Pure);
		}
		else if (geyser->GetResourcePurity() == EResourcePurity::RP_Inpure)
		{
			geyser->SetResourcePurity(EResourcePurity::RP_Normal);
			geyser->SetResourcePurityOverride(EResourcePurity::RP_Normal);
		}
		break;
	}
	}
}

void FGeothermalPurityModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGeothermalPurityModule, GeothermalPurity)
