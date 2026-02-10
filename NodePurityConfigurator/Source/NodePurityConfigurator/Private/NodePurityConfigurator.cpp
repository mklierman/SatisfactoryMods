// Copyright Epic Games, Inc. All Rights Reserved.

#include "NodePurityConfigurator.h"

#include "NPC_Subsystem.h"
#include "SubsystemActorManager.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceNode.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Logging/StructuredLog.h"

#define LOCTEXT_NAMESPACE "FNodePurityConfiguratorModule"

DEFINE_LOG_CATEGORY(Log_NodePurityConfigurator);

void FNodePurityConfiguratorModule::StartupModule()
{
	AFGResourceNode* rn = GetMutableDefault<AFGResourceNode>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGResourceNode::BeginPlay, rn, [this](AFGResourceNode* self)
	{
		USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
		auto purityInt = SessionSettings->GetIntOptionValue("NodePurityConfigurator.Purity");
		UE_LOGFMT(Log_NodePurityConfigurator, Display, "PurityInt = {0}",purityInt);
		if (purityInt != 3)
		{
			auto newPurity = IntToPurity(purityInt);
			self->mPurity = newPurity;
			return;
		}
		
		USubsystemActorManager* SubsystemActorManager = self->GetWorld()->GetSubsystem<USubsystemActorManager>();
		ANPC_Subsystem* NPCS = SubsystemActorManager->GetSubsystemActor<ANPC_Subsystem>();
		auto randomValue = FMath::RandRange(0, 2);
		// auto nodeType = self->GetResourceClass();
		//
		// if (nodeType == NPCS->ironDescClass)
		// {
		// 	UE_LOGFMT(Log_NodePurityConfigurator, Display, "Is Iron");
		// }
		
		UE_LOGFMT(Log_NodePurityConfigurator, Display, "RandomValue = {0}",randomValue);
		auto newPurity = IntToPurity(randomValue);
		self->mPurity = newPurity;
		
		
	});
#endif
}

void FNodePurityConfiguratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
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
	

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNodePurityConfiguratorModule, NodePurityConfigurator)