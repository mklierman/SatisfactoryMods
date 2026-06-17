// Copyright Epic Games, Inc. All Rights Reserved.

#include "ParadiseIsland.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceNodeManager.h"
#include "Resources/FGResourceNode.h"
#include "FGSubsystem.h"
#include "Engine/DataAsset.h"
#include "FGGameState.h"
#include "Hologram/FGHologram.h"
#include "FGGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "FGGameEngine.h"
#include "FGGameMode.h"

#define LOCTEXT_NAMESPACE "FParadiseIslandModule"

void FParadiseIslandModule::StartupModule()
{
#if !WITH_EDITOR	
	AFGResourceNode* rn = GetMutableDefault<AFGResourceNode>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNode::BeginPlay, rn, [this](AFGResourceNode* self)
		{
			auto tags = self->Tags;
			bool isPiNode = false;
			for (auto tag : tags)
			{
				if (tag.ToString() == "ParadiseIsland")
				{
					isPiNode = true;
					break;
				}
			}
			if (isPiNode)
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
	//AFGGameState* gs = GetMutableDefault<AFGGameState>();
	//AFGGameMode* ge = GetMutableDefault<AFGGameMode>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::InitStartSpot_Implementation, gs, [](auto& scope, AFGGameMode* self, AActor* StartSpot, AController* NewPlayer)
	//{
	//	GEngine->BlockTillLevelStreamingCompleted(self->GetWorld());
	//});
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameState::Init, gs, [](auto& scope, AFGGameState* self)
	//	{
	//		GEngine->BlockTillLevelStreamingCompleted(self->GetWorld());
	//	});
	//SUBSCRIBE_METHOD(
	//	AFGGameState::SpawnSubsystem<AFGResourceNodeManager>,
	//	[](auto& scope,
	//		AFGGameState* self,
	//		TObjectPtr<AFGResourceNodeManager>& out_spawnedSubsystem,
	//		TSoftClassPtr<AFGSubsystem> spawnClass,
	//		FName spawnName)
	//	{
	//		// before spawn
	//		GEngine->BlockTillLevelStreamingCompleted(self->GetWorld());
	//	}
	//);
}

void FParadiseIslandModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FParadiseIslandModule, ParadiseIsland)