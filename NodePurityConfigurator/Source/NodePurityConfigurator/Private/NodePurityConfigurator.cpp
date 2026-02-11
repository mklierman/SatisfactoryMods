// Copyright Epic Games, Inc. All Rights Reserved.

#include "NodePurityConfigurator.h"

#include "NPC_Subsystem.h"
#include "SubsystemActorManager.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceNode.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Logging/StructuredLog.h"
#include "FGSaveSession.h"

#define LOCTEXT_NAMESPACE "FNodePurityConfiguratorModule"

DEFINE_LOG_CATEGORY(Log_NodePurityConfigurator);

//#pragma optimize("", off)
void FNodePurityConfiguratorModule::StartupModule()
{
	AFGResourceNode* rn = GetMutableDefault<AFGResourceNode>();
	AFGResourceNodeFrackingSatellite* rnfs = GetMutableDefault<AFGResourceNodeFrackingSatellite>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNode::BeginPlay, rn, [this](AFGResourceNode* self)
	{
			NodeBeginPlay(self);
	});
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNodeFrackingSatellite::BeginPlay, rnfs, [this](AFGResourceNodeFrackingSatellite* self)
	{
			SatelliteBeginPlay(self);
	});
#endif
}

void FNodePurityConfiguratorModule::ShutdownModule()
{
}

void FNodePurityConfiguratorModule::NodeBeginPlay(AFGResourceNode* self, bool isWell)
{
	USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto purityInt = SessionSettings->GetIntOptionValue("NodePurityConfigurator.Purity");
	//UE_LOGFMT(Log_NodePurityConfigurator, Display, "PurityInt = {0}", purityInt);
	if (purityInt != 3)
	{
		auto newPurity = IntToPurity(purityInt);
		self->mPurity = newPurity;
		return;
	}

	auto randomBool = SessionSettings->GetBoolOptionValue("NodePurityConfigurator.Ratios");
	if (!randomBool && purityInt == 3)
	{
		FRandomStream randStream;
		auto locationHash = GetTypeHash(self->GetActorLocation());
		auto saveSession = UFGSaveSession::Get(self->GetWorld());
		auto saveHash = GetTypeHash(saveSession->GetSaveIdentifier());
		randStream.Initialize(locationHash + saveHash);
		auto randValue = randStream.RandRange(0, 2);
		//UE_LOGFMT(Log_NodePurityConfigurator, Display, "randValue = {0}", randValue);
		auto newPurity = IntToPurity(randValue);
		self->mPurity = newPurity;
		return;
	}

	USubsystemActorManager* SubsystemActorManager = self->GetWorld()->GetSubsystem<USubsystemActorManager>();
	if (SubsystemActorManager)
	{
		ANPC_Subsystem* npcSubsystem = SubsystemActorManager->GetSubsystemActor<ANPC_Subsystem>();
		auto nodeType = self->GetResourceClass();
		auto nodeTypeName = nodeType->GetName();

		if (nodeTypeName.ToLower() == "desc_oreiron_c")
		{
			AssignPurityFromArray(npcSubsystem->IronPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_orecopper_c")
		{
			AssignPurityFromArray(npcSubsystem->CopperPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_oregold_c")
		{
			AssignPurityFromArray(npcSubsystem->CateriumPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_oreuranium_c")
		{
			AssignPurityFromArray(npcSubsystem->UraniumPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_orebauxite_c")
		{
			AssignPurityFromArray(npcSubsystem->BauxitePurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_coal_c")
		{
			AssignPurityFromArray(npcSubsystem->CoalPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_orecopper_c")
		{
			AssignPurityFromArray(npcSubsystem->CopperPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_liquidoil_c")
		{
			if (isWell)
			{
				AssignPurityFromArray(npcSubsystem->OilWellPurities, npcSubsystem->SavedPurities, self);
			}
			else
			{
				AssignPurityFromArray(npcSubsystem->OilPurities, npcSubsystem->SavedPurities, self);
			}
		}
		else if (nodeTypeName.ToLower() == "desc_nitrogengas_c")
		{
			AssignPurityFromArray(npcSubsystem->NitrogenWellPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_rawquartz_c")
		{
			AssignPurityFromArray(npcSubsystem->QuartzPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_sam_c")
		{
			AssignPurityFromArray(npcSubsystem->SAMPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_stone_c")
		{
			AssignPurityFromArray(npcSubsystem->LimestonePurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_sulfur_c")
		{
			AssignPurityFromArray(npcSubsystem->SulfurPurities, npcSubsystem->SavedPurities, self);
		}
		else if (nodeTypeName.ToLower() == "desc_water_c")
		{
			AssignPurityFromArray(npcSubsystem->WaterWellPurities, npcSubsystem->SavedPurities, self);
		}
	}
}

void FNodePurityConfiguratorModule::SatelliteBeginPlay(AFGResourceNodeFrackingSatellite* self)
{
	auto node = Cast<AFGResourceNode>(self);
	if (node)
	{
		NodeBeginPlay(node, true);
	}
}

EResourcePurity FNodePurityConfiguratorModule::IntToPurity(int32 value)
{
	EResourcePurity returnValue = EResourcePurity::RP_Normal;
	switch(value)
	{
	case 0:
		returnValue = EResourcePurity::RP_Inpure;
		break;
	case 1:
		returnValue = EResourcePurity::RP_Normal;
		break;
	case 2:
		returnValue = EResourcePurity::RP_Pure;
		break;
	}
	return returnValue;
}

bool FNodePurityConfiguratorModule::AssignPurityFromArray(TMap<int32, int32>& PurityMap, TMap<FVector, int32>& SavedPurities,
	AFGResourceNode* self)
{
	FVector ActorLocation = self->GetActorLocation();

	// Check for saved purity data
	auto savedPurity = SavedPurities.Find(ActorLocation);
	if (savedPurity != nullptr)
	{
		auto newPurity = IntToPurity(*savedPurity);
		self->mPurity = newPurity;
		return true;
	}

	// Check if any purities are available
	bool anyAvailable = PurityMap[0] != 0 ||
		PurityMap[1] != 0 ||
		PurityMap[2] != 0;

	if (anyAvailable)
	{
		int randomValue;
		do
		{
			randomValue = FMath::RandRange(0, 2);
		} while (PurityMap[randomValue] == 0);

		auto newPurity = IntToPurity(randomValue);
		self->mPurity = newPurity;
		PurityMap[randomValue]--;
		SavedPurities.Add(ActorLocation, randomValue);
		return true;
	}
	else
	{
		UE_LOGFMT(Log_NodePurityConfigurator, Warning, "No purities are available");
		return false;
	}
}

//#pragma optimize("", on)

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNodePurityConfiguratorModule, NodePurityConfigurator)