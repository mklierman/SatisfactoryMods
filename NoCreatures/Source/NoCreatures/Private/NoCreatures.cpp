// Copyright Epic Games, Inc. All Rights Reserved.

#include "NoCreatures.h"
#include "Patching/NativeHookManager.h"
#include "Creature/FGCreature.h"
#include "Creature/FGCreatureSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "FGGameInstance.h"

#define LOCTEXT_NAMESPACE "FNoCreaturesModule"

DEFINE_LOG_CATEGORY(NoCreatures_Log);

void FNoCreaturesModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGCreatureSpawner::SpawnCreatures, [this](auto& scope, AFGCreatureSpawner* self)
		{
			scope.Cancel();
		});
	SUBSCRIBE_METHOD(AFGCreatureSpawner::SpawnSingleCreature, [this](auto& scope, AFGCreatureSpawner* self)
		{
			scope.Cancel();
		});

	AFGCreature* fgc = GetMutableDefault<AFGCreature>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGCreature::BeginPlay, fgc, [this](auto& scope, AFGCreature* self)
		{
			if (self->GetWorld())
			{
				const FString LevelName = UGameplayStatics::GetCurrentLevelName(self->GetWorld(), true);
				//UE_LOGFMT(NoCreatures_Log, Display, "Level name: {0}", LevelName);
				if (!LevelName.Contains("menu"))
				{
					self->Destroy();
				}
			}
		});
#endif
}

void FNoCreaturesModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNoCreaturesModule, NoCreatures)