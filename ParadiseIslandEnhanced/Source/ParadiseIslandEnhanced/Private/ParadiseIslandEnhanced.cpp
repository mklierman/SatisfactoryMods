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
	AFGResourceNode* rn = GetMutableDefault<AFGResourceNode>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNode::BeginPlay, rn, [this](AFGResourceNode* self)
		{
			auto tags = self->Tags;
			bool isPieNode = false;
			for (auto tag : tags)
			{
				if (tag.ToString() == "PIE")
				{
					isPieNode = true;
					break;
				}
			}
			if (isPieNode)
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
			}
		});
#endif
	//SUBSCRIBE_METHOD(AFGResourceNodeManager::ApplyRandomizationSettings, [](auto& scope, AFGResourceNodeManager* self, ENodeRandomizationMode randomizationMode, ENodePuritySettings puritySettings, const int32 seed)
	//	{
	//		GEngine->BlockTillLevelStreamingCompleted(self->GetWorld());
	//	});
	//AFGGameMode* gm = GetMutableDefault<AFGGameMode>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::InitGameState, gm, [](auto& scope, AFGGameMode* self)
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