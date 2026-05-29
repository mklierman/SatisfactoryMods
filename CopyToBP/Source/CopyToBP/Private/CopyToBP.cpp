// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopyToBP.h"
#include "Logging/StructuredLog.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGPlayerController.h"
#include "FGFactoryClipboard.h"
#include "FGCharacterPlayer.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "CTBP_ConfigStruct.h"
#include "CTBP_Component.h"

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
			UCTBP_Component* ccomp = nullptr;
			for (UActorComponent* Component : state->GetComponents())
			{
				if (auto comp = Cast<UCTBP_Component>(Component))
				{
					ccomp = comp;
					break;
				}
			}

			auto clippy = state->mFactoryClipboard;
			clippy.Empty();
			for (FClipboardData clipData : ccomp->PlayerClipboardData)
			{
				clippy.Add(clipData.ObjectClass, clipData.ClipboardSettings);
			}
			state->mFactoryClipboard = clippy;

			if (ccomp->ShouldPasteToBP)
			{
				for (auto child : out_children)
				{
					if (auto buildable = Cast<AFGBuildableManufacturer>(child))
					{
						state->PasteFactoryClipboard(child);
						//UE_LOGFMT(CTBP_Log, Display, "Pasted to {0}", buildable->GetName());
					}
				}
			}
		});

	// void AFGPlayerState::CopyFactoryClipboard(UObject* object)
	SUBSCRIBE_METHOD_AFTER(AFGPlayerState::CopyFactoryClipboard, [this](AFGPlayerState* self, UObject* object)
		{
			UCTBP_Component* ccomp = nullptr;
			for (UActorComponent* Component : self->GetComponents())
			{
				if (auto comp = Cast<UCTBP_Component>(Component))
				{
					ccomp = comp;
					break;
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