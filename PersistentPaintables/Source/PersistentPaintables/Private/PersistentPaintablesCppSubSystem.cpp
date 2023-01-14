#include "PersistentPaintablesCppSubSystem.h"
#include "FGGameState.h"
#include "FGBuildableSubsystem.h"
#include "Hologram/FGHologram.h"
#include "FGColorInterface.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"

APersistentPaintablesCppSubSystem::APersistentPaintablesCppSubSystem()
{

}

//#pragma optimize("", off)
void APersistentPaintablesCppSubSystem::HookConstruct()
{
	if (HasAuthority())
	{
		//virtual AActor* Construct( TArray< AActor* >& out_children, FNetConstructionID constructionID );
		//#if !WITH_EDITOR
		SUBSCRIBE_METHOD_AFTER(AFGBuildableSubsystem::AddBuildable, [=](AFGBuildableSubsystem* self, class AFGBuildable* buildable)
			{
				if (buildable)
				{
					if (buildable->GetCanBeColored_Implementation())
					{
						if (PlayerCustomizationStructs.Num() > 0)
						{
							if (auto instigator = buildable->mBuildEffectInstignator)
							{
								for (auto custData : PlayerCustomizationStructs)
								{
									if (custData.CharacterPlayer && custData.CharacterPlayer == instigator)
									{
										buildable->SetCustomizationData_Implementation(custData.CustomizationData);
										buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
										return;
									}
								}
							}
						}
					}
				}
			});
		//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::OnFullRebuildCompleted, &UpdateColor)
//#endif
	}
}

void APersistentPaintablesCppSubSystem::PaintBuildableConstructed(AFGBuildable* buildable)
{
		if (PlayerCustomizationStructs.Num() > 0)
		{
			if (auto instigator = buildable->mBuildEffectInstignator)
			{
				for (auto custData : PlayerCustomizationStructs)
				{
					if (custData.CharacterPlayer && custData.CharacterPlayer == instigator)
					{
						buildable->SetCustomizationData_Implementation(custData.CustomizationData);
						buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
						return;
					}
				}
			}
		}
}
//#pragma optimize("", on)

void APersistentPaintablesCppSubSystem::SetCustomSwatchColor(uint8 ColorSlot, FLinearColor PColor, FLinearColor SColor)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState)
	{

		FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(
			PColor,
			SColor
		);

		//Subsystem->mColorSlots_Data[ColorSlot] = NewColourSlot;
		//FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
		FGGameState->Server_SetBuildingColorDataForSlot(ColorSlot, NewColourSlot);

		Subsystem->SetColorSlot_Data(ColorSlot, NewColourSlot);
		Subsystem->mColorSlotsAreDirty = true;

		//FGGameState->mBuildingColorSlots_Data[ColorSlot] = Subsystem->mColorSlots_Data[ColorSlot];
		//FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);

	}
}

void APersistentPaintablesCppSubSystem::ApplyCustomizationPreview(AFGBuildableSubsystem* buildableSubsystem, AFGBuildable* buildable, const FFactoryCustomizationData& previewData)
{
	buildableSubsystem->ApplyCustomizationPreview(buildable, previewData);
}

TSubclassOf<class UFGItemDescriptor> APersistentPaintablesCppSubSystem::GetBuildableDescriptor(AFGBuildable* buildable)
{
	return buildable->GetBuiltWithDescriptor();
}
