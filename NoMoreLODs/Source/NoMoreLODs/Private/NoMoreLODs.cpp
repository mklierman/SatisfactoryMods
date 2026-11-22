#include "NoMoreLODs.h"

#include "Components/PrimitiveComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Configuration/ConfigManager.h"
#include "Configuration/ConfigProperty.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Level.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "GrassInstancedStaticMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "LandscapeComponent.h"
#include "Misc/OutputDeviceNull.h"
#include "NoMoreLODs_ConfigListener.h"
#include "NoMoreLODs_ConfigStruct.h"
#include "TimerManager.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

namespace
{
	constexpr float wholeMapStreamingRadius = 100000000.0f;
}

class FNoMoreLODsStreamingSourceProvider final : public IWorldPartitionStreamingSourceProvider {
public:
	FNoMoreLODsStreamingSourceProvider(UWorld* world, const float radius)
		: world(world)
		, radius(radius)
		, sourceName(*FString::Printf(TEXT("NoMoreLODs_%s"), *world->GetName()))
	{
	}

	virtual bool GetStreamingSource(FWorldPartitionStreamingSource& outStreamingSource) const override
	{
		if (!world.IsValid() || radius <= 0.0f)
		{
			return false;
		}

		outStreamingSource = FWorldPartitionStreamingSource();
		outStreamingSource.Name = sourceName;
		outStreamingSource.Location = FVector::ZeroVector;
		outStreamingSource.Rotation = FRotator::ZeroRotator;
		outStreamingSource.TargetState = EStreamingSourceTargetState::Activated;
		outStreamingSource.bBlockOnSlowLoading = true;
		outStreamingSource.Priority = EStreamingSourcePriority::Highest;
		outStreamingSource.DebugColor = FColor::Magenta;
		outStreamingSource.bForce2D = true;

		FStreamingSourceShape& shape = outStreamingSource.Shapes.AddDefaulted_GetRef();
		shape.bUseGridLoadingRange = false;
		shape.Radius = radius;
		shape.bIsSector = false;
		shape.Location = FVector::ZeroVector;
		outStreamingSource.UpdateHash();
		return true;
	}

	virtual const UObject* GetStreamingSourceOwner() const override
	{
		return world.Get();
	}

private:
	TWeakObjectPtr<UWorld> world;
	float radius;
	FName sourceName;
};

void FNoMoreLODsModule::StartupModule()
{
	ApplyConsoleVariables();

	postWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(
		this, &FNoMoreLODsModule::HandlePostWorldInitialization);
	worldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(
		this, &FNoMoreLODsModule::HandleWorldCleanup);
	levelAddedToWorldHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(
		this, &FNoMoreLODsModule::HandleLevelAddedToWorld);

	if (GEngine != nullptr)
	{
		for (const FWorldContext& worldContext : GEngine->GetWorldContexts())
		{
			ConfigureWorld(worldContext.World());
		}
	}
}

void FNoMoreLODsModule::ShutdownModule()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(postWorldInitializationHandle);
	FWorldDelegates::OnWorldCleanup.Remove(worldCleanupHandle);
	FWorldDelegates::LevelAddedToWorld.Remove(levelAddedToWorldHandle);

	if (configListener != nullptr)
	{
		configListener->Unbind();
		configListener->RemoveFromRoot();
		configListener = nullptr;
	}

	for (const TPair<UWorld*, FDelegateHandle>& entry : actorSpawnedHandles)
	{
		if (IsValid(entry.Key))
		{
			entry.Key->RemoveOnActorSpawnedHandler(entry.Value);
		}
	}
	actorSpawnedHandles.Empty();

	TArray<UWorld*> registeredWorlds;
	streamingSourceProviders.GetKeys(registeredWorlds);
	for (UWorld* world : registeredWorlds)
	{
		UnregisterWorldPartitionStreamingSource(world);
	}
}

void FNoMoreLODsModule::BindSMLConfiguration(UWorld* world)
{
	if (!IsValid(world) || !IsValid(world->GetGameInstance()))
	{
		return;
	}

	UConfigManager* configManager = world->GetGameInstance()->GetSubsystem<UConfigManager>();
	const FConfigId configId{TEXT("NoMoreLODs"), TEXT("")};
	if (!IsValid(configManager) || configManager->GetConfigurationById(configId) == nullptr)
	{
		return;
	}

	UConfigPropertySection* rootSection = configManager->GetConfigurationRootSection(configId);
	const TObjectPtr<UConfigProperty>* propertyPtr = IsValid(rootSection)
		? rootSection->SectionProperties.Find(TEXT("LoadEntireMap"))
		: nullptr;
	UConfigProperty* loadEntireMapProperty = propertyPtr != nullptr ? propertyPtr->Get() : nullptr;
	if (!IsValid(loadEntireMapProperty))
	{
		return;
	}

	if (configListener == nullptr)
	{
		configListener = NewObject<UNoMoreLODsConfigListener>(configManager);
		configListener->AddToRoot();
	}
	configListener->Bind(this, loadEntireMapProperty);

	loadEntireMap = FNoMoreLODs_ConfigStruct::GetActiveConfig(world).LoadEntireMap;
}

void FNoMoreLODsModule::SetEntireMapEnabled(UWorld* world, const bool enabled)
{
	loadEntireMap = enabled;
	if (!IsValid(world))
	{
		return;
	}

	if (loadEntireMap)
	{
		RegisterWorldPartitionStreamingSource(world);
	}
	else
	{
		UnregisterWorldPartitionStreamingSource(world);
		FOutputDeviceNull outputDevice;
		IConsoleManager::Get().ProcessUserConsoleInput(TEXT("wp.Runtime.HLOD 1"), outputDevice, world);
	}
}

void FNoMoreLODsModule::HandleSMLConfigurationChanged(UObject* worldContext)
{
	UWorld* world = GEngine != nullptr
		? GEngine->GetWorldFromContextObject(worldContext, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	if (!IsValid(world))
	{
		return;
	}

	const bool enabled = FNoMoreLODs_ConfigStruct::GetActiveConfig(worldContext).LoadEntireMap;
	SetEntireMapEnabled(world, enabled);
}

void FNoMoreLODsModule::ApplyConsoleVariables() const
{
	struct FCVarOverride
	{
		const TCHAR* name;
		const TCHAR* value;
	};

	static constexpr FCVarOverride overrides[] = {
		{TEXT("FGSignificanceManager.ForceFullSignificance"), TEXT("1")},
		{TEXT("FGServerSignificanceSubsystem.ForceFullSignificance"), TEXT("1")},
		{TEXT("r.StaticMeshLODDistanceScale"), TEXT("0.001")},
		{TEXT("r.SkeletalMeshLODBias"), TEXT("-1000")},
		{TEXT("foliage.ForceLOD"), TEXT("0")},
		{TEXT("foliage.LODDistanceScale"), TEXT("1000")},
		{TEXT("foliage.MinimumScreenSize"), TEXT("0.00000001")},
		{TEXT("foliage.MaxTrianglesToRender"), TEXT("2147483647")},
		{TEXT("r.LandscapeLOD0DistributionScale"), TEXT("1000")},
		{TEXT("r.LandscapeLODDistributionScale"), TEXT("1000")},
		{TEXT("r.Nanite.MaxPixelsPerEdge"), TEXT("1.0")},
		{TEXT("r.Nanite.ImposterMaxPixels"), TEXT("0")},
		{TEXT("r.Nanite.Culling.DrawDistance"), TEXT("0")},
		{TEXT("r.Nanite.PrimaryRaster.PixelsPerEdgeScaling"), TEXT("100")},
		{TEXT("r.Nanite.Streaming.QualityScale.MinQuality"), TEXT("1")},
		{TEXT("r.Landscape.AllowNanitePerClusterDisplacementDisable"), TEXT("0")},
		{TEXT("landscape.RenderNanite"), TEXT("0")},
		{TEXT("r.ViewDistanceScale"), TEXT("1000")},
		{TEXT("r.Streaming.FullyLoadMeshes"), TEXT("1")},
		{TEXT("r.Streaming.FullyLoadUsedTextures"), TEXT("1")},
		{TEXT("r.Streaming.UseAllMips"), TEXT("1")},
		{TEXT("r.Streaming.MipBias"), TEXT("0")}
	};

	IConsoleManager& consoleManager = IConsoleManager::Get();
	for (const FCVarOverride& override : overrides)
	{
		if (IConsoleVariable* variable = consoleManager.FindConsoleVariable(override.name))
		{
			variable->Set(override.value, ECVF_SetByCode);
		}
	}
}

void FNoMoreLODsModule::RegisterWorldPartitionStreamingSource(UWorld* world)
{
	if (!loadEntireMap || !IsValid(world) || streamingSourceProviders.Contains(world))
	{
		return;
	}

	UWorldPartitionSubsystem* worldPartitionSubsystem = world->GetSubsystem<UWorldPartitionSubsystem>();
	if (!IsValid(worldPartitionSubsystem))
	{
		return;
	}

	TSharedPtr<FNoMoreLODsStreamingSourceProvider> provider =
		MakeShared<FNoMoreLODsStreamingSourceProvider>(world, wholeMapStreamingRadius);
	worldPartitionSubsystem->RegisterStreamingSourceProvider(provider.Get());
	streamingSourceProviders.Add(world, provider);

	FOutputDeviceNull outputDevice;
	IConsoleManager::Get().ProcessUserConsoleInput(TEXT("wp.Runtime.HLOD 0"), outputDevice, world);

	const TWeakObjectPtr<UWorld> weakWorld(world);
	world->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([weakWorld]()
	{
		if (UWorld* streamingWorld = weakWorld.Get(); IsValid(streamingWorld))
		{
			streamingWorld->BlockTillLevelStreamingCompleted();
		}
	}));
}

void FNoMoreLODsModule::UnregisterWorldPartitionStreamingSource(UWorld* world)
{
	TSharedPtr<FNoMoreLODsStreamingSourceProvider> provider;
	if (!streamingSourceProviders.RemoveAndCopyValue(world, provider))
	{
		return;
	}

	if (IsValid(world))
	{
		if (UWorldPartitionSubsystem* worldPartitionSubsystem = world->GetSubsystem<UWorldPartitionSubsystem>())
		{
			worldPartitionSubsystem->UnregisterStreamingSourceProvider(provider.Get());
		}
	}
}

void FNoMoreLODsModule::ConfigureWorld(UWorld* world)
{
	if (!IsValid(world) || !world->IsGameWorld() || actorSpawnedHandles.Contains(world))
	{
		return;
	}

	ApplyConsoleVariables();
	BindSMLConfiguration(world);
	SetEntireMapEnabled(world, loadEntireMap);
	actorSpawnedHandles.Add(world, world->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateRaw(this, &FNoMoreLODsModule::HandleActorSpawned)));

	for (ULevel* level : world->GetLevels())
	{
		HandleLevelAddedToWorld(level, world);
	}

}

void FNoMoreLODsModule::HandlePostWorldInitialization(
	UWorld* world, const UWorld::InitializationValues initializationValues)
{
	ConfigureWorld(world);
}

void FNoMoreLODsModule::HandleWorldCleanup(UWorld* world, bool sessionEnded, bool cleanupResources)
{
	UnregisterWorldPartitionStreamingSource(world);

	if (FDelegateHandle* handle = actorSpawnedHandles.Find(world))
	{
		world->RemoveOnActorSpawnedHandler(*handle);
		actorSpawnedHandles.Remove(world);
	}
}

void FNoMoreLODsModule::HandleLevelAddedToWorld(ULevel* level, UWorld* world)
{
	if (!IsValid(level) || !IsValid(world) || !world->IsGameWorld())
	{
		return;
	}

	for (AActor* actor : level->Actors)
	{
		ApplyToActor(actor);
	}
}

void FNoMoreLODsModule::HandleActorSpawned(AActor* actor)
{
	ApplyToActor(actor);
}

void FNoMoreLODsModule::ApplyToActor(AActor* actor) const
{
	if (!IsValid(actor) || actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return;
	}

	TInlineComponentArray<UActorComponent*> components(actor);
	for (UActorComponent* component : components)
	{
		ApplyToComponent(component);
	}
}

void FNoMoreLODsModule::ApplyToComponent(UActorComponent* component) const
{
	UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(component);
	if (!IsValid(primitive))
	{
		return;
	}

	primitive->MinDrawDistance = 0.0f;
	primitive->bNeverDistanceCull = true;
	primitive->SetCullDistance(0.0f);
	primitive->SetCachedMaxDrawDistance(0.0f);

	if (UStaticMeshComponent* staticMesh = Cast<UStaticMeshComponent>(primitive))
	{
		staticMesh->SetForcedLodModel(1);
	}

	if (UInstancedStaticMeshComponent* instancedMesh = Cast<UInstancedStaticMeshComponent>(primitive))
	{
		instancedMesh->SetCullDistances(0, 0);
	}

	if (Cast<UFoliageInstancedStaticMeshComponent>(primitive) != nullptr ||
		Cast<UGrassInstancedStaticMeshComponent>(primitive) != nullptr)
	{
		CastChecked<UStaticMeshComponent>(primitive)->SetForceDisableNanite(true);
	}

	if (USkinnedMeshComponent* skinnedMesh = Cast<USkinnedMeshComponent>(primitive))
	{
		skinnedMesh->SetForcedLOD(1);
	}

	if (ULandscapeComponent* landscape = Cast<ULandscapeComponent>(primitive))
	{
		landscape->SetForcedLOD(0);
	}

	primitive->MarkRenderStateDirty();
}

IMPLEMENT_MODULE(FNoMoreLODsModule, NoMoreLODs)
