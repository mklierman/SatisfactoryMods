// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Materials/MaterialInstance.h"
class FOutlineLootModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FLinearColor GetColor(AActor* actor);
	float GetScale(AActor* actor);

	UMaterialInstance* material;
};
