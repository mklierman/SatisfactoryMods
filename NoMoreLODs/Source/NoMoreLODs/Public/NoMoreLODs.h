#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Modules/ModuleManager.h"

class AActor;
class FNoMoreLODsStreamingSourceProvider;
class UActorComponent;
class UNoMoreLODsConfigListener;
class ULevel;
class FNoMoreLODsModule : public IModuleInterface {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void HandleSMLConfigurationChanged(UObject* worldContext);

private:
	void BindSMLConfiguration(UWorld* world);
	void SetEntireMapEnabled(UWorld* world, bool enabled);
	void ApplyConsoleVariables() const;
	void RegisterWorldPartitionStreamingSource(UWorld* world);
	void UnregisterWorldPartitionStreamingSource(UWorld* world);
	void ConfigureWorld(UWorld* world);
	void HandlePostWorldInitialization(UWorld* world, const UWorld::InitializationValues initializationValues);
	void HandleWorldCleanup(UWorld* world, bool sessionEnded, bool cleanupResources);
	void HandleLevelAddedToWorld(ULevel* level, UWorld* world);
	void HandleActorSpawned(AActor* actor);
	void ApplyToActor(AActor* actor) const;
	void ApplyToComponent(UActorComponent* component) const;

	FDelegateHandle postWorldInitializationHandle;
	FDelegateHandle worldCleanupHandle;
	FDelegateHandle levelAddedToWorldHandle;
	TMap<UWorld*, FDelegateHandle> actorSpawnedHandles;
	TMap<UWorld*, TSharedPtr<FNoMoreLODsStreamingSourceProvider>> streamingSourceProviders;
	UNoMoreLODsConfigListener* configListener = nullptr;
	bool loadEntireMap = false;
};
