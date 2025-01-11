// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopySampledSign.h"

#define LOCTEXT_NAMESPACE "FCopySampledSignModule"

#pragma optimize("", off)
void FCopySampledSignModule::StartupModule()
{
#if !WITH_EDITOR
	AFGBuildableHologram* hg = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, hg, [this](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			if (self)
			{
				AddBuildable(inBuildable, self);
				return;
			}
		});
#endif
}

void FCopySampledSignModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FCopySampledSignModule::AddBuildable(AFGBuildable* buildable, const AFGBuildableHologram* hologram)
{
	if (buildable)
	{
		if (auto signBuildable = Cast<AFGBuildableWidgetSign>(buildable))
		{
			if (auto instigatorPawn = hologram->GetConstructionInstigator())
			{
				auto instigator = Cast<AFGCharacterPlayer>(instigatorPawn);
				if (instigator)
				{
					auto component = instigator->GetComponentByClass<UCSS_ActorComponent>();
					if (component)
					{
						if (component->SignData.GetGUID() != FPrefabSignData().GetGUID())
						{
							signBuildable->SetPrefabSignData(component->SignData);
						}
					}
				}
			}
		}
	}
}

#pragma optimize("", on)
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCopySampledSignModule, CopySampledSign)