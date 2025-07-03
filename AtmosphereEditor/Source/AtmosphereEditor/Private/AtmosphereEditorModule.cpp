#include "AtmosphereEditorModule.h"
#include "Patching/NativeHookManager.h"
#include "FGAtmosphereVolume.h"
#include "Atmosphere/UFGBiome.h"

#pragma optimize("", off)
void FAtmosphereEditorModule::StartupModule() {


	//AFGAtmosphereVolume* av = GetMutableDefault<AFGAtmosphereVolume>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGAtmosphereVolume::BeginPlay, av,[this](AFGAtmosphereVolume* self)
	//	{
	//		AtmosphereVolueBeginPlay(self);
	//	});

}

void FAtmosphereEditorModule::AtmosphereVolueBeginPlay(AFGAtmosphereVolume* volume)
{
	if (volume)
	{
		FExponentialFogSettings fogSettings;
		FSkySphereSettings sphereSettings;
		FSkyAtmosphereSettings atmoSettings;
		//volume->GetSettings(fogSettings);
		//volume->GetSkySphereSettings(0, sphereSettings);
		//volume->GetAtmosphereSettings(0, atmoSettings);
		auto biome = volume->mBiome;
		auto idk = "idk";
	}
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FAtmosphereEditorModule, AtmosphereEditor);