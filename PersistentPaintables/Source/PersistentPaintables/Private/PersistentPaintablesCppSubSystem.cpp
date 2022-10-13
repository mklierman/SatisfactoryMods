#include "PersistentPaintablesCppSubSystem.h"
#include "FGGameState.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildable.h"

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
