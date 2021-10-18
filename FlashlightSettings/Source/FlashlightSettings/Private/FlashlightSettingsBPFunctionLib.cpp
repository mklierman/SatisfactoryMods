// ILikeBanas


#include "FlashlightSettingsBPFunctionLib.h"

bool UFlashlightSettingsBPFunctionLib::UseTemperature(USpotLightComponent* SpotLightComp)
{	
	if (SpotLightComp != NULL)
	{
		SpotLightComp->bUseTemperature = true;
		return true;
	}
	return false;
}

void UFlashlightSettingsBPFunctionLib::TestAccessors()
{

}
