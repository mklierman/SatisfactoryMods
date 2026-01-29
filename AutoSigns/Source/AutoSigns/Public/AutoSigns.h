// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Hologram/FGBuildableHologram.h"

DECLARE_LOG_CATEGORY_EXTERN(AutoSigns_Log, Display, All);
class FAutoSignsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void ConfigureActor(const AFGBuildableHologram* self, AFGBuildable* inBuildable);
	void InitializeSignPrefabData(AFGBuildableWidgetSign* sign);
	int32 GetIconIdForDescriptor(UObject* worldContext, TSubclassOf<UFGItemDescriptor> descriptor, int32 fallback);
	void UpdateSignElements(AFGBuildableWidgetSign* sign, FPrefabSignData& prefabSignData);
	void SetDefaultSignData(AFGBuildableWidgetSign* sign);

	UPROPERTY()
	TMap<AFGBuildableWidgetSign*, AFGBuildable*> SignSnaps;

	UPROPERTY()
	FPrefabSignData LastSignData;

	UPROPERTY()
	TMap< FString, FString > DefaultTextElementToDataMap;

	UPROPERTY()
	TMap< FString, int32 > DefaultIconElementToDataMap;
};
