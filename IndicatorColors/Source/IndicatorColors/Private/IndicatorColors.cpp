// Copyright Epic Games, Inc. All Rights Reserved.

#include "IndicatorColors.h"
#include <Logging/StructuredLog.h>
#include <SessionSettings/SessionSettingsManager.h>
#include "Patching/NativeHookManager.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "IndicatorColors_ConfigurationStruct.h"

#define LOCTEXT_NAMESPACE "FIndicatorColorsModule"
DEFINE_LOG_CATEGORY(IndicatorColors_Log);

//#pragma optimize("", off)
void FIndicatorColorsModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGProductionIndicatorInstanceComponent::SetVisuals, [this](auto& scope, UFGProductionIndicatorInstanceComponent* self, EProductionStatus newStatus)
		{
			//UE_LOGFMT(IndicatorColors_Log, Display, "SetVisuals");

			auto config = FIndicatorColors_ConfigurationStruct::GetActiveConfig(self);
			switch (newStatus)
			{
			case EProductionStatus::IS_PRODUCING:
				self->mStateActiveData.mColor = GetColorFromInt(config.ActiveColor);
				break;
			case EProductionStatus::IS_PRODUCING_WITH_CRYSTAL:
				self->mStateActiveWithCrystalData.mColor = GetColorFromInt(config.OverclockColor);
				break;
			case EProductionStatus::IS_STANDBY:
				self->mStateStandbyData.mColor = GetColorFromInt(config.StandbyColor);
				break;
			case EProductionStatus::IS_ERROR:
				self->mStateErrorData.mColor = GetColorFromInt(config.ErrorColor);
				break;
			default:
				break;
			}
		});
#endif
}

void FIndicatorColorsModule::ShutdownModule()
{

}

FLinearColor FIndicatorColorsModule::GetColorFromInt(int32 inInt)
{
	switch (inInt)
	{
	case 0:
		return FLinearColor::Red;
	case 1:
		return FLinearColor::Green;
	case 2:
		return FLinearColor::Yellow;
	case 3: // Blue
		return FLinearColor(0.15, 0.255, 0.75, 1);
	case 4: // Magenta
		return FLinearColor(0.255, 0, 0.255, 1);
	case 5: // Cyan
		return FLinearColor(0, 0.255, 0.255, 1);
	case 6: // White
		return FLinearColor::White;
	default:
		break;
	}
	return FLinearColor::Black;
}

//#pragma optimize("", on)
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FIndicatorColorsModule, IndicatorColors)