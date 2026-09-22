// Copyright Epic Games, Inc. All Rights Reserved.

#include "CopyToBP.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBlueprintHologram.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "CTBP_Component.h"

#define LOCTEXT_NAMESPACE "FCopyToBPModule"
DEFINE_LOG_CATEGORY(CTBP_Log);

namespace
{
	AFGPlayerState* GetPlayerStateFromInstigator(APawn* InstigatorPawn)
	{
		if (!InstigatorPawn)
		{
			return nullptr;
		}

		if (AFGCharacterPlayer* Character = Cast<AFGCharacterPlayer>(InstigatorPawn))
		{
			if (AFGPlayerState* State = Character->GetControllingPlayerState())
			{
				return State;
			}
		}

		if (AController* Controller = InstigatorPawn->GetController())
		{
			return Controller->GetPlayerState<AFGPlayerState>();
		}

		return nullptr;
	}
}

void FCopyToBPModule::StartupModule()
{
#if !WITH_EDITOR
	AFGPlayerState* PlayerStateCDO = GetMutableDefault<AFGPlayerState>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPlayerState::BeginPlay, PlayerStateCDO, [](AActor* self)
		{
			if (UCTBP_Component* Component = UCTBP_Component::FindOrAdd(Cast<AFGPlayerState>(self)))
			{
				Component->ApplyClipboardToOwner();
			}
		});

	AFGBlueprintHologram* bph = GetMutableDefault<AFGBlueprintHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBlueprintHologram::Construct, bph, [this](AActor* result, AFGBlueprintHologram* self, TArray< AActor* >& out_children, FNetConstructionID NetConstructionID)
		{
			if (!self || !self->HasAuthority())
			{
				return;
			}

			AFGPlayerState* State = GetPlayerStateFromInstigator(self->GetConstructionInstigator());
			if (!State)
			{
				return;
			}

			UCTBP_Component* ccomp = State->FindComponentByClass<UCTBP_Component>();
			if (!ccomp)
			{
				return;
			}

			if (!ccomp->ShouldPasteToBP)
			{
				return;
			}

			UCTBP_Component::ApplyClipboardDataToPlayerState(State, ccomp->PlayerClipboardData);

			for (AActor* Child : out_children)
			{
				if (Cast<AFGBuildableManufacturer>(Child))
				{
					State->PasteFactoryClipboard(Child);
				}
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGPlayerState::CopyFactoryClipboard, [](AFGPlayerState* self, UObject* object)
		{
			if (!self)
			{
				return;
			}

			UCTBP_Component* ccomp = UCTBP_Component::FindOrAdd(self);
			if (!ccomp)
			{
				ccomp = self->FindComponentByClass<UCTBP_Component>();
			}
			if (!ccomp)
			{
				return;
			}

			const TArray<FClipboardData> Data = UCTBP_Component::MakeClipboardDataFromPlayerState(self);
			ccomp->PlayerClipboardData = Data;
			ccomp->ServerSetPlayerClipboardData(Data);
			ccomp->ServerSetShouldPasteToBP(ccomp->ShouldPasteToBP);
		});
#endif
}

void FCopyToBPModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCopyToBPModule, CopyToBP)
