// ILikeBanas

#include "FlashlightSettingsBPFunctionLib.h"
#include "UObject/UnrealTypePrivate.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/Properties/ConfigPropertyInteger.h"

//
// This cursed thing is used to automatically map a type
// to the corresponding UConfigProperty* type. #BlameRex
//
template<typename T> struct type_mapper { };

template<> struct type_mapper<bool>  { typedef UConfigPropertyBool    type; };
template<> struct type_mapper<float> { typedef UConfigPropertyFloat   type; };
template<> struct type_mapper<int32> { typedef UConfigPropertyInteger type; };

template<typename T, typename U = type_mapper<T>::type>
static void SetConfigProp(UConfigPropertySection* Section, const FString& Key, const T Value)
{
	CastChecked<U>(Section->SectionProperties.FindChecked(Key))->Value = Value;
}

void UFlashlightSettingsBPFunctionLib::StoreFlashlightSettings(UConfigPropertySection* Section, USpotLightComponent* SpotLight)
{
	SetConfigProp(Section, "Intensity", SpotLight->Intensity);
	SetConfigProp(Section, "Attenuation", SpotLight->AttenuationRadius);
	SetConfigProp(Section, "InnerCone", SpotLight->InnerConeAngle);
	SetConfigProp(Section, "OuterCone", SpotLight->OuterConeAngle);
	SetConfigProp(Section, "Shadows", bool(SpotLight->CastShadows));
	SetConfigProp(Section, "Temperature", int32(SpotLight->Temperature));
	SetConfigProp(Section, "SpecularScale", SpotLight->SpecularScale);
	SetConfigProp(Section, "LightFalloff", SpotLight->LightFalloffExponent);

	Section->MarkDirty();
}

FFlashlightSettings_ModConfigStruct_SavedSettings UFlashlightSettingsBPFunctionLib::GetFlashlightSettings(UObject* WorldContext)
{
	return FFlashlightSettings_ModConfigStruct::GetActiveConfig(WorldContext).SavedSettings;
}

void UFlashlightSettingsBPFunctionLib::ApplyFlashlightSettings(const FFlashlightSettings_ModConfigStruct_SavedSettings& SavedSettings, USpotLightComponent* SpotLight)
{
	SpotLight->SetIntensity(SavedSettings.Intensity);
	SpotLight->SetAttenuationRadius(SavedSettings.Attenuation);
	SpotLight->SetInnerConeAngle(SavedSettings.InnerCone);
	SpotLight->SetOuterConeAngle(SavedSettings.OuterCone);
	SpotLight->SetCastShadows(SavedSettings.Shadows);
	SpotLight->SetTemperature(SavedSettings.Temperature);
	SpotLight->SetSpecularScale(SavedSettings.SpecularScale);
	SpotLight->SetLightFalloffExponent(SavedSettings.LightFalloff);
	SpotLight->SetUseTemperature(true);
}
