#include "AE_BPFL.h"
#include "Interfaces/Interface_PostProcessVolume.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "GameFramework/Volume.h"
#include "FGSkySphere.h"
#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"
#include "Atmosphere/BiomeHelpers.h"

//void AE_BPFL::ExampleFunction() {}

//FExponentialFogSettings UAE_BPFL::GetFogSettings(AFGAtmosphereVolume* volume)
//{
//    FExponentialFogSettings efs;
//    volume->GetSettings(efs);
//    return efs;
//}
//
//FSkySphereSettings UAE_BPFL::GetSkySphereSettings(AFGAtmosphereVolume* volume, float atTime)
//{
//    FSkySphereSettings sss;
//    volume->GetSkySphereSettings(atTime, sss);
//    return sss;
//}
//
//FSkyAtmosphereSettings UAE_BPFL::GetAtmosphereSettings(AFGAtmosphereVolume* volume, float atTime)
//{
//    FSkyAtmosphereSettings sas;
//    volume->GetAtmosphereSettings(atTime, sas);
//    return sas;
//}
//
////FPostProcessVolumeProperties UAE_BPFL::GetPostProcessProperties(AFGAtmosphereVolume* volume)
////{
////    auto props =  volume->GetProperties();
////    return props;
////}
//
//void UAE_BPFL::GetBiomeExponentialFogSettings(UFGBiome* biome, float atTime, FExponentialFogSettings& out_settings)
//{
//    FBiomeHelpers::GetExponentialFogSettings(biome, atTime, out_settings);
//}
//
//void UAE_BPFL::GetBiomeSkySphereSettings(UFGBiome* biome, float atTime, FSkySphereSettings& out_settings)
//{
//    FBiomeHelpers::GetSkySphereSettings(biome, atTime, out_settings);
//}
//
//void UAE_BPFL::GetBiomeAtmosphereSettings(UFGBiome* biome, float atTime, FSkyAtmosphereSettings& out_settings)
//{
//    FBiomeHelpers::GetAtmosphereSettings(biome, atTime, out_settings);
//}

#pragma optimize("", off)
TArray<UFGBiome*> UAE_BPFL::GetAllBiomes()
{

	TArray<UFGBiome*> FoundObjects;
	for (TObjectIterator<UFGBiome> Iterator; Iterator; ++Iterator)
	{
		UFGBiome* Object = *Iterator;
		if (IsValid(Object))
		{
			FoundObjects.Add(Object);
		}
	}
    return FoundObjects;
}
UFGBiome* UAE_BPFL::GetGrassyFieldsMain()
{
	auto object2  = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Map/GameLevel01/Persistent_Level/_Generated_/CLWMOZIO3TRH9C4JQFD3Q3UWG.Persistent_Level:PersistentLevel.Atmosphere_GrassFields_Valley.Biome_C"));
	auto object = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/World/Environment/Atmosphere/Biome_GrassFieldsValley.Biome_GrassFieldsValley_C"));
	if (object)
	{
		auto biome = Cast<UFGBiome>(object);
		if (biome)
		{
			return biome;
		}
	}
	return nullptr;
}
#pragma optimize("", on)
