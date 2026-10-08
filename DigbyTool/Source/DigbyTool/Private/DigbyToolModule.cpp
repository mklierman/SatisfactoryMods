#include "DigbyToolModule.h"
#include "Misc/ConfigCacheIni.h"

void FDigbyToolModule::StartupModule() 
{
    if (!IsRunningDedicatedServer())
    {
        FString AudioMixerName;
        GConfig->GetString(TEXT("Audio"), TEXT("AudioMixerModuleName"), AudioMixerName, GEngineIni);
        if (AudioMixerName.IsEmpty())
        {
            GConfig->SetString(TEXT("Audio"), TEXT("AudioMixerModuleName"), TEXT("AudioMixerXAudio2"), GEngineIni);
        }
    }
}


IMPLEMENT_GAME_MODULE(FDigbyToolModule, DigbyTool);