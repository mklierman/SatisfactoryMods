// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "Resources/FGResourceNodeFrackingSatellite.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(Log_NodePurityConfigurator, Display, All);
class FNodePurityConfiguratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void NodeBeginPlay(AFGResourceNode* self, bool isWell = false);
	void SatelliteBeginPlay(AFGResourceNodeFrackingSatellite* self);
	
	EResourcePurity IntToPurity(int32 value);
	bool AssignPurityFromArray(TMap<int32, int32>& PurityMap, TMap<FVector, int32>& SavedPurities, AFGResourceNode* self);
};
