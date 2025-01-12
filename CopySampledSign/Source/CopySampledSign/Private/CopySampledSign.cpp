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
						FPrefabSignData newSignData = FPrefabSignData();
						newSignData.AuxiliaryColor = component->SignData.AuxiliaryColor;
						newSignData.BackgroundColor = component->SignData.BackgroundColor;
						newSignData.Emissive = component->SignData.Emissive;
						newSignData.ForegroundColor = component->SignData.ForegroundColor;
						newSignData.Glossiness = component->SignData.Glossiness;
						newSignData.IsFromReplication = component->SignData.IsFromReplication;
						newSignData.PrefabLayout = component->SignData.PrefabLayout;
						newSignData.SignTypeDesc = component->SignData.SignTypeDesc;
						if (component->SignData.IconElementDataKeys.Num() > 0)
						{
							TMap<FString, int32> IconElementData;
							for (int i = 0; i < component->SignData.IconElementDataKeys.Num(); i++)
							{
								IconElementData.Add(component->SignData.IconElementDataKeys[i], component->SignData.IconElementDataValues[i]);
							}
							newSignData.IconElementData = IconElementData;
						}
						if (component->SignData.TextElementDataKeys.Num() > 0)
						{
							TMap<FString, FString> TextElementData;
							for (int i = 0; i < component->SignData.TextElementDataKeys.Num(); i++)
							{
								TextElementData.Add(component->SignData.TextElementDataKeys[i], component->SignData.TextElementDataValues[i]);
							}
							newSignData.TextElementData = TextElementData;
						}

						if (newSignData.GetGUID() != FPrefabSignData().GetGUID())
						{
							signBuildable->SetPrefabSignData(newSignData);
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