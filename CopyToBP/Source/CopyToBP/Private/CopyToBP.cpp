// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopyToBP.h"
#include "Logging/StructuredLog.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGPlayerController.h"
#include "FGCharacterPlayer.h"

#define LOCTEXT_NAMESPACE "FCopyToBPModule"

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
			//auto clipboard = state->mFactoryClipboard;
			//auto firstItem = clipboard[0];
			//clipboard.Empty();
			//clipboard.Add(firstItem);
			for (auto child : out_children)
			{
				state->PasteFactoryClipboard(child);
			}
		});
#endif
}

void FCopyToBPModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCopyToBPModule, CopyToBP)