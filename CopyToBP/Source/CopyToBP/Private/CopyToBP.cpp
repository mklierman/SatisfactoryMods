// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopyToBP.h"
#include "Logging/StructuredLog.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGPlayerController.h"
#include "FGCharacterPlayer.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "CTBP_ConfigStruct.h"

#define LOCTEXT_NAMESPACE "FCopyToBPModule"
DEFINE_LOG_CATEGORY(CTBP_Log);

void FCopyToBPModule::StartupModule()
{
#if !WITH_EDITOR
	AFGBlueprintHologram* bph = GetMutableDefault<AFGBlueprintHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBlueprintHologram::Construct, bph, [this](AActor* result, AFGBlueprintHologram* self, TArray< AActor* >& out_children, FNetConstructionID NetConstructionID)
		{

			auto instPawn = self->GetConstructionInstigator();
			auto cont = instPawn->GetController();
			auto fgCont = Cast<AFGPlayerController>(cont);
			auto character = Cast<AFGCharacterPlayer>(fgCont->GetCharacter());
			auto state = character->GetControllingPlayerState();
			auto config = FCTBP_ConfigStruct::GetActiveConfig(fgCont->GetWorld());
			
			//UE_LOGFMT(CTBP_Log, Display, "AFGBlueprintHologram::Construct");
			if (config.ShouldPaste)
			{
				for (auto child : out_children)
				{
					if (auto buildable = Cast<AFGBuildableManufacturer>(child))
					{
						state->PasteFactoryClipboard(child);
					}
				}
			}
		});
#endif
}

void FCopyToBPModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCopyToBPModule, CopyToBP)