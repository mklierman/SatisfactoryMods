// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParadiseIslandEnhanced.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceNodeManager.h"
#include "Resources/FGResourceNode.h"
#include "FGSubsystem.h"
#include "Engine/DataAsset.h"
#include "FGGameState.h"
#include "Hologram/FGHologram.h"

#define LOCTEXT_NAMESPACE "FParadiseIslandEnhancedModule"

void FParadiseIslandEnhancedModule::StartupModule()
{

#if !WITH_EDITOR	
#endif
	//AFGResourceNode* rn = GetMutableDefault<AFGResourceNode>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNode::BeginPlay, rn, [this](AFGResourceNode* self)
	//	{
	//		auto tags = self->Tags;
	//		bool isPieNode = false;
	//		for (auto tag : tags)
	//		{
	//			if (tag.ToString() == "PIE")
	//			{
	//				isPieNode = true;
	//				break;
	//			}
	//		}
	//		if (isPieNode)
	//		{
	//			auto state = Cast<AFGGameState>(self->GetWorld()->GetGameState());
	//			auto puritySettings = state->GetNodePuritySettings();
	//			auto nodeManager = AFGResourceNodeManager::Get(self->GetWorld());
	//			auto newPurity = nodeManager->GetPurityOverride(self->mPurity, puritySettings);
	//			self->mPurity = newPurity;
	//		}
	//	});
	//SUBSCRIBE_METHOD(AFGResourceNodeManager::ApplyRandomizationSettings, [](auto& scope, AFGResourceNodeManager* self, ENodeRandomizationMode randomizationMode, ENodePuritySettings puritySettings, const int32 seed)
	//	{
	//		GEngine->BlockTillLevelStreamingCompleted(self->GetWorld());
	//	});
}

void FParadiseIslandEnhancedModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FParadiseIslandEnhancedModule, ParadiseIslandEnhanced)