#include "Smokey_BPFL.h"

#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGDecorationTemplate.h"
#include "FGSignificanceManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr float maxDistance = 1.0e9f;

	const FMaterialParameterInfo cameraFadeOffsetParameter(TEXT("CameraFadeOffset"));
	const FMaterialParameterInfo cameraFadeLengthParameter(TEXT("CameraFadeLength"));
	const FMaterialParameterInfo fadeDistanceParameter(TEXT("Fade_Distance"));
	const FMaterialParameterInfo distanceParameter(TEXT("Distance"));
	const FMaterialParameterInfo distanceFadeStrengthParameter(TEXT("DIstanceFade_Strength"));

	struct FScalarParameterState
	{
		FMaterialParameterInfo Parameter;
		float Value;
	};

	struct FMaterialState
	{
		TArray<FScalarParameterState> ScalarParameters;
		int32 ReferenceCount = 0;
	};

	struct FRendererState
	{
		bool EnableCameraDistanceCulling;
		float MaxCameraDistance;
		ENiagaraRendererPixelCoverageMode PixelCoverageMode;
		float PixelCoverageBlend;
		TStrongObjectPtr<UMaterialInterface> Material;
		int32 ReferenceCount = 0;
	};

	struct FNiagaraMaterialParameterState
	{
		FNiagaraMaterialParameterState(const FNiagaraVariable& inParameter, UMaterialInterface* inMaterial, bool hadOverride) :
			Parameter(inParameter),
			Material(inMaterial),
			HadOverride(hadOverride)
		{
		}

		FNiagaraVariable Parameter;
		TStrongObjectPtr<UMaterialInterface> Material;
		bool HadOverride;
	};

	struct FComponentState
	{
		bool AllowCullDistanceVolume;
		bool NeverDistanceCull;
		float MinDrawDistance;
		float MaxDrawDistance;
		float CachedMaxDrawDistance;
		float BoundsScale;

		bool OverrideLODMethod = false;
		TEnumAsByte<ParticleSystemLODMethod> LODMethod;
		int32 LODLevel = 0;
		EParticleSignificanceLevel RequiredSignificance = EParticleSignificanceLevel::Low;
		bool ManagingSignificance = false;
		float SecondsBeforeInactive = 0.0f;

		bool AllowScalability = true;
		bool ForceLocalPlayerEffect = false;
		bool ActivatedBySmokey = false;
		bool MadeVisibleBySmokey = false;

		TArray<TStrongObjectPtr<UMaterialInterface>> Materials;
		TArray<FNiagaraMaterialParameterState> NiagaraMaterialParameters;
		TSet<TWeakObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;
		TSet<TWeakObjectPtr<UNiagaraSpriteRendererProperties>> SpriteRenderers;
	};

	struct FBuildableState
	{
		TArray<TWeakObjectPtr<UFXSystemComponent>> EffectComponents;
		TArray<TWeakObjectPtr<UFXSystemComponent>> LocalEffectComponents;
		float SignificanceRange = 0.0f;
		bool HasSignificanceRange = false;
	};

	TMap<TWeakObjectPtr<UNiagaraComponent>, TWeakObjectPtr<UNiagaraSystem>> configuredNiagaraSystems;
	TMap<TWeakObjectPtr<UMaterialInstanceDynamic>, FMaterialState> materialStates;
	TMap<TWeakObjectPtr<UNiagaraSpriteRendererProperties>, FRendererState> rendererStates;
	TMap<TWeakObjectPtr<UFXSystemComponent>, FComponentState> componentStates;
	TMap<TWeakObjectPtr<AFGBuildable>, FBuildableState> buildableStates;
	TMap<UClass*, TArray<TWeakObjectPtr<UFXSystemComponent>>> effectTemplatesByClass;
	TSet<TWeakObjectPtr<AFGBuildable>> pendingBuildables;
	TMap<TWeakObjectPtr<AFGBuildable>, int32> pendingRetryCounts;
	TMap<TWeakObjectPtr<UWorld>, FTimerHandle> pendingRetryTimers;

	constexpr float pendingRetryInterval = 0.25f;
	constexpr int32 maxPendingRetries = 40;

	void SchedulePendingRetry(UWorld* world);

	void RetryPendingBuildables(TWeakObjectPtr<UWorld> worldKey)
	{
		pendingRetryTimers.Remove(worldKey);

		UWorld* world = worldKey.Get();
		if (!world)
		{
			return;
		}

		TArray<TWeakObjectPtr<AFGBuildable>> buildablesToRetry;
		for (const TWeakObjectPtr<AFGBuildable>& pendingBuildable : pendingBuildables)
		{
			if (!pendingBuildable.IsValid())
			{
				pendingRetryCounts.Remove(pendingBuildable);
				continue;
			}

			if (pendingBuildable->GetWorld() == world)
			{
				buildablesToRetry.Add(pendingBuildable);
			}
		}

		for (const TWeakObjectPtr<AFGBuildable>& pendingBuildable : buildablesToRetry)
		{
			int32& retryCount = pendingRetryCounts.FindOrAdd(pendingBuildable);
			if (retryCount >= maxPendingRetries)
			{
				pendingBuildables.Remove(pendingBuildable);
				pendingRetryCounts.Remove(pendingBuildable);
				continue;
			}

			++retryCount;
			USmokey_BPFL::ConfigureBuildableParticleSystems(pendingBuildable.Get());
		}

		for (const TWeakObjectPtr<AFGBuildable>& pendingBuildable : pendingBuildables)
		{
			if (pendingBuildable.IsValid() && pendingBuildable->GetWorld() == world)
			{
				SchedulePendingRetry(world);
				break;
			}
		}
	}

	void SchedulePendingRetry(UWorld* world)
	{
		if (!world)
		{
			return;
		}

		const TWeakObjectPtr<UWorld> worldKey(world);
		if (pendingRetryTimers.Contains(worldKey))
		{
			return;
		}

		FTimerHandle timerHandle;
		world->GetTimerManager().SetTimer(
			timerHandle,
			FTimerDelegate::CreateLambda([worldKey]()
			{
				RetryPendingBuildables(worldKey);
			}),
			pendingRetryInterval,
			false);
		pendingRetryTimers.Add(worldKey, timerHandle);
	}

	bool HasScalarParameter(UMaterialInterface* material, const FMaterialParameterInfo& parameter)
	{
		float value;
		return material->GetScalarParameterValue(FHashedMaterialParameterInfo(parameter), value);
	}

	void SetScalarParameter(UMaterialInstanceDynamic* material, UMaterialInterface* parentMaterial, const FMaterialParameterInfo& parameter, float value)
	{
		if (HasScalarParameter(parentMaterial, parameter))
		{
			material->SetScalarParameterValueByInfo(parameter, value);
		}
	}

	void AcquireMaterial(UMaterialInstanceDynamic* material, FComponentState& componentState)
	{
		const TWeakObjectPtr<UMaterialInstanceDynamic> materialKey(material);
		if (componentState.DynamicMaterials.Contains(materialKey))
		{
			return;
		}

		auto& materialState = materialStates.FindOrAdd(materialKey);
		if (materialState.ReferenceCount == 0)
		{
			const FMaterialParameterInfo parameters[] =
			{
				cameraFadeOffsetParameter,
				cameraFadeLengthParameter,
				fadeDistanceParameter,
				distanceParameter,
				distanceFadeStrengthParameter
			};

			for (const FMaterialParameterInfo& parameter : parameters)
			{
				float value;
				if (material->GetScalarParameterValue(FHashedMaterialParameterInfo(parameter), value))
				{
					materialState.ScalarParameters.Add({parameter, value});
				}
			}
		}

		++materialState.ReferenceCount;
		componentState.DynamicMaterials.Add(materialKey);
	}

	void ReleaseMaterials(FComponentState& componentState)
	{
		for (const TWeakObjectPtr<UMaterialInstanceDynamic>& materialKey : componentState.DynamicMaterials)
		{
			auto materialState = materialStates.Find(materialKey);
			if (!materialState || --materialState->ReferenceCount > 0)
			{
				continue;
			}

			if (auto material = materialKey.Get())
			{
				for (const FScalarParameterState& scalarParameter : materialState->ScalarParameters)
				{
					material->SetScalarParameterValueByInfo(scalarParameter.Parameter, scalarParameter.Value);
				}
			}

			materialStates.Remove(materialKey);
		}
	}

	bool ConfigureMaterial(UMaterialInterface* material, TFunctionRef<UMaterialInstanceDynamic*(UMaterialInterface*)> createDynamicMaterial, FComponentState& componentState)
	{
		if (!IsValid(material))
		{
			return false;
		}

		const bool hasCameraFadeOffset = HasScalarParameter(material, cameraFadeOffsetParameter);
		const bool hasCameraFadeLength = HasScalarParameter(material, cameraFadeLengthParameter);
		const bool hasFadeDistance = HasScalarParameter(material, fadeDistanceParameter);

		if (!hasCameraFadeOffset && !hasCameraFadeLength && !hasFadeDistance)
		{
			return false;
		}

		auto dynamicMaterial = Cast<UMaterialInstanceDynamic>(material);
		const bool createdDynamicMaterial = dynamicMaterial == nullptr;
		if (!dynamicMaterial)
		{
			dynamicMaterial = createDynamicMaterial(material);
		}

		if (!dynamicMaterial)
		{
			return false;
		}

		AcquireMaterial(dynamicMaterial, componentState);

		SetScalarParameter(dynamicMaterial, material, cameraFadeOffsetParameter, maxDistance);
		SetScalarParameter(dynamicMaterial, material, cameraFadeLengthParameter, maxDistance);
		SetScalarParameter(dynamicMaterial, material, fadeDistanceParameter, maxDistance);

		if (hasFadeDistance)
		{
			SetScalarParameter(dynamicMaterial, material, distanceParameter, maxDistance);
			SetScalarParameter(dynamicMaterial, material, distanceFadeStrengthParameter, 0.0f);
		}

		return createdDynamicMaterial;
	}

	void FindEffectComponents(AFGBuildable* buildable, TArray<UFXSystemComponent*>& effectComponents)
	{
		buildable->GetComponents(effectComponents);

		TInlineComponentArray<USceneComponent*> sceneComponents;
		buildable->GetComponents(sceneComponents);
		for (USceneComponent* sceneComponent : sceneComponents)
		{
			TArray<USceneComponent*> children;
			sceneComponent->GetChildrenComponents(true, children);
			for (USceneComponent* child : children)
			{
				if (auto effectComponent = Cast<UFXSystemComponent>(child))
				{
					effectComponents.AddUnique(effectComponent);
				}
			}
		}

		for (TFieldIterator<FObjectProperty> property(buildable->GetClass()); property; ++property)
		{
			if (!property->PropertyClass->IsChildOf(UFXSystemComponent::StaticClass()))
			{
				continue;
			}

			if (auto effectComponent = Cast<UFXSystemComponent>(property->GetObjectPropertyValue_InContainer(buildable)))
			{
				effectComponents.AddUnique(effectComponent);
			}
		}
	}

	void PrepareLocalEffectComponent(AFGBuildable* buildable, UFXSystemComponent* sourceComponent, UFXSystemComponent* localComponent, FBuildableState& buildableState)
	{
		FTransform relativeTransform = sourceComponent->GetRelativeTransform();
		if (auto sourceOwner = sourceComponent->GetOwner())
		{
			if (auto sourceRoot = sourceOwner->GetRootComponent())
			{
				relativeTransform = sourceComponent->GetComponentTransform().GetRelativeTransform(sourceRoot->GetComponentTransform());
			}
		}

		localComponent->SetAbsolute(false, false, false);
		localComponent->AttachToComponent(buildable->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		localComponent->SetRelativeTransform(relativeTransform);
		buildable->ConfigureDynamicDecoratorComponent(localComponent);
		localComponent->SetHiddenInGame(false, true);
		localComponent->SetVisibility(true, true);
		localComponent->SetComponentTickEnabled(true);
		localComponent->Deactivate();
		buildableState.LocalEffectComponents.Add(localComponent);
	}

	UFXSystemComponent* CreateLocalEffectComponent(AFGBuildable* buildable, UFXSystemComponent* sourceComponent, FBuildableState& buildableState)
	{
		UFXSystemComponent* localComponent = nullptr;

		if (auto sourceParticleComponent = Cast<UParticleSystemComponent>(sourceComponent))
		{
			auto particleComponent = NewObject<UParticleSystemComponent>(buildable);
			particleComponent->bAutoActivate = false;
			particleComponent->InstanceParameters = sourceParticleComponent->InstanceParameters;
			particleComponent->SetTemplate(sourceParticleComponent->Template);
			localComponent = particleComponent;
		}
		else
		{
			localComponent = AFGDecorationTemplate::SpawnComponentFromTemplate(
				buildable, sourceComponent, EComponentMobility::Static);
		}

		if (!localComponent)
		{
			return nullptr;
		}

		buildable->AddInstanceComponent(localComponent);
		PrepareLocalEffectComponent(buildable, sourceComponent, localComponent, buildableState);

		if (!localComponent->IsRegistered())
		{
			localComponent->RegisterComponentWithWorld(buildable->GetWorld());
		}

		return localComponent;
	}

	void SpawnLocalEffectComponents(AFGBuildable* buildable, const TArray<TWeakObjectPtr<UFXSystemComponent>>& componentTemplates, FBuildableState& buildableState, TArray<UFXSystemComponent*>& effectComponents)
	{
		for (const TWeakObjectPtr<UFXSystemComponent>& componentTemplate : componentTemplates)
		{
			if (!componentTemplate.IsValid())
			{
				continue;
			}

			if (auto localComponent = CreateLocalEffectComponent(buildable, componentTemplate.Get(), buildableState))
			{
				effectComponents.Add(localComponent);
			}
		}
	}

	FComponentState& SaveComponentState(UFXSystemComponent* component)
	{
		const TWeakObjectPtr<UFXSystemComponent> componentKey(component);
		if (auto componentState = componentStates.Find(componentKey))
		{
			return *componentState;
		}

		FComponentState componentState;
		componentState.AllowCullDistanceVolume = component->bAllowCullDistanceVolume;
		componentState.NeverDistanceCull = component->bNeverDistanceCull;
		componentState.MinDrawDistance = component->MinDrawDistance;
		componentState.MaxDrawDistance = component->LDMaxDrawDistance;
		componentState.CachedMaxDrawDistance = component->CachedMaxDrawDistance;
		componentState.BoundsScale = component->BoundsScale;

		for (int32 materialIndex = 0; materialIndex < component->GetNumMaterials(); ++materialIndex)
		{
			componentState.Materials.Emplace(component->GetMaterial(materialIndex));
		}

		if (auto particleComponent = Cast<UParticleSystemComponent>(component))
		{
			componentState.OverrideLODMethod = particleComponent->bOverrideLODMethod;
			componentState.LODMethod = particleComponent->LODMethod;
			componentState.LODLevel = particleComponent->GetLODLevel();
			componentState.RequiredSignificance = particleComponent->RequiredSignificance;
			componentState.ManagingSignificance = particleComponent->bIsManagingSignificance;
			componentState.SecondsBeforeInactive = particleComponent->SecondsBeforeInactive;
		}
		else if (auto niagaraComponent = Cast<UNiagaraComponent>(component))
		{
			componentState.AllowScalability = niagaraComponent->GetAllowScalability();
			componentState.ForceLocalPlayerEffect = niagaraComponent->GetForceLocalPlayerEffect();
		}

		return componentStates.Add(componentKey, MoveTemp(componentState));
	}

	void AcquireRenderer(UNiagaraSpriteRendererProperties* spriteRenderer, FComponentState& componentState)
	{
		const TWeakObjectPtr<UNiagaraSpriteRendererProperties> rendererKey(spriteRenderer);
		if (componentState.SpriteRenderers.Contains(rendererKey))
		{
			return;
		}

		auto& rendererState = rendererStates.FindOrAdd(rendererKey);
		if (rendererState.ReferenceCount == 0)
		{
			rendererState.EnableCameraDistanceCulling = spriteRenderer->bEnableCameraDistanceCulling;
			rendererState.MaxCameraDistance = spriteRenderer->MaxCameraDistance;
			rendererState.PixelCoverageMode = spriteRenderer->PixelCoverageMode;
			rendererState.PixelCoverageBlend = spriteRenderer->PixelCoverageBlend;
			rendererState.Material.Reset(spriteRenderer->Material.Get());
		}

		++rendererState.ReferenceCount;
		componentState.SpriteRenderers.Add(rendererKey);
	}

	void ReleaseRenderers(FComponentState& componentState)
	{
		for (const TWeakObjectPtr<UNiagaraSpriteRendererProperties>& rendererKey : componentState.SpriteRenderers)
		{
			auto rendererState = rendererStates.Find(rendererKey);
			if (!rendererState || --rendererState->ReferenceCount > 0)
			{
				continue;
			}

			if (auto spriteRenderer = rendererKey.Get())
			{
				spriteRenderer->bEnableCameraDistanceCulling = rendererState->EnableCameraDistanceCulling;
				spriteRenderer->MaxCameraDistance = rendererState->MaxCameraDistance;
				spriteRenderer->PixelCoverageMode = rendererState->PixelCoverageMode;
				spriteRenderer->PixelCoverageBlend = rendererState->PixelCoverageBlend;
				spriteRenderer->Material = rendererState->Material.Get();
			}

			rendererStates.Remove(rendererKey);
		}
	}

	void ConfigureNiagaraRenderers(UNiagaraComponent* niagaraComponent, FComponentState& componentState)
	{
		auto niagaraSystem = niagaraComponent->GetAsset();
		if (!niagaraSystem)
		{
			return;
		}

		bool hasSpriteRenderer = false;
		bool rendererChanged = false;

		for (const FNiagaraEmitterHandle& emitterHandle : niagaraSystem->GetEmitterHandles())
		{
			auto emitterData = emitterHandle.GetEmitterData();
			if (!emitterData)
			{
				continue;
			}

			for (UNiagaraRendererProperties* renderer : emitterData->GetRenderers())
			{
				auto spriteRenderer = Cast<UNiagaraSpriteRendererProperties>(renderer);
				if (!spriteRenderer)
				{
					continue;
				}

				hasSpriteRenderer = true;
				AcquireRenderer(spriteRenderer, componentState);
				rendererChanged |= spriteRenderer->bEnableCameraDistanceCulling ||
					spriteRenderer->MaxCameraDistance != maxDistance ||
					spriteRenderer->PixelCoverageMode != ENiagaraRendererPixelCoverageMode::Disabled ||
					spriteRenderer->PixelCoverageBlend != 0.0f;

				spriteRenderer->bEnableCameraDistanceCulling = false;
				spriteRenderer->MaxCameraDistance = maxDistance;
				spriteRenderer->PixelCoverageMode = ENiagaraRendererPixelCoverageMode::Disabled;
				spriteRenderer->PixelCoverageBlend = 0.0f;

				if (!spriteRenderer->NeedsMIDsForMaterials())
				{
					rendererChanged |= ConfigureMaterial(
						spriteRenderer->Material.Get(),
						[spriteRenderer](UMaterialInterface* parentMaterial)
						{
							auto dynamicMaterial = UMaterialInstanceDynamic::Create(parentMaterial, spriteRenderer);
							if (dynamicMaterial)
							{
								spriteRenderer->Material = dynamicMaterial;
							}

							return dynamicMaterial;
						},
						componentState);
				}
			}
		}

		const TWeakObjectPtr<UNiagaraComponent> componentKey(niagaraComponent);
		const auto configuredSystem = configuredNiagaraSystems.Find(componentKey);
		const bool componentNeedsRefresh = !configuredSystem || configuredSystem->Get() != niagaraSystem;

		if (hasSpriteRenderer && (rendererChanged || componentNeedsRefresh))
		{
			configuredNiagaraSystems.Add(componentKey, niagaraSystem);
			niagaraComponent->ReinitializeSystem();
			niagaraComponent->MarkRenderStateDirty();
		}
	}

	void ConfigureNiagaraMaterials(UNiagaraComponent* niagaraComponent, FComponentState& componentState)
	{
		auto niagaraSystem = niagaraComponent->GetAsset();
		if (!niagaraSystem)
		{
			return;
		}

		TArray<FNiagaraVariable> userParameters;
		niagaraSystem->GetExposedParameters().GetUserParameters(userParameters);
		for (const FNiagaraVariable& parameter : userParameters)
		{
			if (!parameter.IsUObject() ||
				!parameter.GetType().GetClass()->IsChildOf(UMaterialInterface::StaticClass()))
			{
				continue;
			}

			auto material = Cast<UMaterialInterface>(
				niagaraComponent->GetOverrideParameters().GetUObject(parameter));
			if (!material)
			{
				material = Cast<UMaterialInterface>(
					niagaraSystem->GetExposedParameters().GetUObject(parameter));
			}

			const bool hadOverride = niagaraComponent->GetOverrideParameters().IndexOf(parameter) != INDEX_NONE;
			if (!componentState.NiagaraMaterialParameters.ContainsByPredicate(
				[&parameter](const FNiagaraMaterialParameterState& state)
				{
					return state.Parameter == parameter;
				}))
			{
				componentState.NiagaraMaterialParameters.Emplace(parameter, material, hadOverride);
			}

			ConfigureMaterial(material, [niagaraComponent, &parameter](UMaterialInterface* parentMaterial)
			{
				auto dynamicMaterial = UMaterialInstanceDynamic::Create(parentMaterial, niagaraComponent);
				if (dynamicMaterial)
				{
					niagaraComponent->SetVariableMaterial(parameter.GetName(), dynamicMaterial);
				}
				return dynamicMaterial;
			}, componentState);
		}

		TArray<UMaterialInterface*> usedMaterials;
		niagaraComponent->GetUsedMaterials(usedMaterials);
		for (UMaterialInterface* material : usedMaterials)
		{
			ConfigureMaterial(material, [](UMaterialInterface*) -> UMaterialInstanceDynamic*
			{
				return nullptr;
			}, componentState);
		}
	}

	void RestoreComponent(UFXSystemComponent* component, FComponentState& componentState)
	{
		for (int32 materialIndex = 0; materialIndex < componentState.Materials.Num(); ++materialIndex)
		{
			component->SetMaterial(materialIndex, componentState.Materials[materialIndex].Get());
		}

		if (auto niagaraComponent = Cast<UNiagaraComponent>(component))
		{
			for (const FNiagaraMaterialParameterState& parameterState : componentState.NiagaraMaterialParameters)
			{
				if (parameterState.HadOverride)
				{
					niagaraComponent->SetVariableMaterial(parameterState.Parameter.GetName(), parameterState.Material.Get());
				}
				else
				{
					niagaraComponent->GetOverrideParameters().RemoveParameter(parameterState.Parameter);
				}
			}

			niagaraComponent->SetAllowScalability(componentState.AllowScalability);
			niagaraComponent->SetForceLocalPlayerEffect(componentState.ForceLocalPlayerEffect);
		}

		ReleaseRenderers(componentState);
		ReleaseMaterials(componentState);

		component->bAllowCullDistanceVolume = componentState.AllowCullDistanceVolume;
		component->bNeverDistanceCull = componentState.NeverDistanceCull;
		component->MinDrawDistance = componentState.MinDrawDistance;
		component->SetCullDistance(componentState.MaxDrawDistance);
		component->SetCachedMaxDrawDistance(componentState.CachedMaxDrawDistance);
		component->SetBoundsScale(componentState.BoundsScale);

		if (auto particleComponent = Cast<UParticleSystemComponent>(component))
		{
			particleComponent->bOverrideLODMethod = componentState.OverrideLODMethod;
			particleComponent->LODMethod = componentState.LODMethod;
			particleComponent->SetLODLevel(componentState.LODLevel);
			particleComponent->SetRequiredSignificance(componentState.RequiredSignificance);
			particleComponent->SetManagingSignificance(componentState.ManagingSignificance);
			particleComponent->SecondsBeforeInactive = componentState.SecondsBeforeInactive;
		}
		else if (auto niagaraComponent = Cast<UNiagaraComponent>(component))
		{
			configuredNiagaraSystems.Remove(niagaraComponent);
			niagaraComponent->ReinitializeSystem();
		}

		if (componentState.ActivatedBySmokey)
		{
			component->Deactivate();
		}

		if (componentState.MadeVisibleBySmokey)
		{
			component->SetVisibility(false, true);
		}

		component->MarkRenderStateDirty();
	}
}

void USmokey_BPFL::SetGeneratorSignificanceRange(AFGBuildableFactory* generator, float newRange)
{
	if (!IsValid(generator))
	{
		return;
	}

	auto rangeProperty = FindFProperty<FFloatProperty>(AFGBuildableFactory::StaticClass(), TEXT("mSignificanceRange"));
	if (!rangeProperty)
	{
		return;
	}

	rangeProperty->SetPropertyValue_InContainer(generator, newRange);

	if (auto manager = generator->GetWorld()->GetSubsystem<UFGSignificanceManager>())
	{
		manager->UpdateStaticObjectSignificanceRange(generator);
	}
}

void USmokey_BPFL::ConfigureBuildableParticleSystems(AFGBuildable* buildable)
{
	if (!IsValid(buildable))
	{
		return;
	}

	auto& buildableState = buildableStates.FindOrAdd(buildable);
	auto factory = Cast<AFGBuildableFactory>(buildable);

	if (factory)
	{
		if (!buildableState.HasSignificanceRange)
		{
			if (auto rangeProperty = FindFProperty<FFloatProperty>(AFGBuildableFactory::StaticClass(), TEXT("mSignificanceRange")))
			{
				buildableState.SignificanceRange = rangeProperty->GetPropertyValue_InContainer(factory);
				buildableState.HasSignificanceRange = true;
			}
		}

		SetGeneratorSignificanceRange(factory, maxDistance);
	}

	TArray<UFXSystemComponent*> effectComponents;
	FindEffectComponents(buildable, effectComponents);
	bool learnedEffectTemplates = false;

	if (!effectComponents.IsEmpty() && !effectTemplatesByClass.Contains(buildable->GetClass()))
	{
		TArray<TWeakObjectPtr<UFXSystemComponent>> componentTemplates;
		for (UFXSystemComponent* component : effectComponents)
		{
			if (component->IsRegistered() && !buildableState.LocalEffectComponents.Contains(component))
			{
				componentTemplates.Add(component);
			}
		}

		if (!componentTemplates.IsEmpty())
		{
			effectTemplatesByClass.Add(buildable->GetClass(), MoveTemp(componentTemplates));
			learnedEffectTemplates = true;
		}
	}

	if (effectComponents.IsEmpty())
	{
		if (auto componentTemplates = effectTemplatesByClass.Find(buildable->GetClass()))
		{
			SpawnLocalEffectComponents(buildable, *componentTemplates, buildableState, effectComponents);
		}
	}

	if (effectComponents.IsEmpty())
	{
		for (USceneComponent* componentTemplate :
			AFGDecorationTemplate::GetSignificantComponentsFromSubclass(buildable->GetDecorationTemplate()))
		{
			auto effectTemplate = Cast<UFXSystemComponent>(componentTemplate);
			if (!effectTemplate)
			{
				continue;
			}

			if (auto localComponent = CreateLocalEffectComponent(buildable, effectTemplate, buildableState))
			{
				effectComponents.Add(localComponent);
			}
		}
	}

	for (UFXSystemComponent*& component : effectComponents)
	{
		if (!IsValid(component) || component->IsRegistered())
		{
			continue;
		}

		if (auto localComponent = CreateLocalEffectComponent(buildable, component, buildableState))
		{
			component = localComponent;
		}
	}

	if (effectComponents.IsEmpty())
	{
		pendingBuildables.Add(buildable);
		SchedulePendingRetry(buildable->GetWorld());
	}
	else
	{
		pendingBuildables.Remove(buildable);
		pendingRetryCounts.Remove(buildable);
	}

	for (UFXSystemComponent* component : effectComponents)
	{
		if (!IsValid(component))
		{
			continue;
		}

		buildableState.EffectComponents.AddUnique(component);
		auto& componentState = SaveComponentState(component);

		component->bAllowCullDistanceVolume = false;
		component->SetCullDistance(0.0f);
		component->bNeverDistanceCull = true;
		component->SetCachedMaxDrawDistance(0.0f);
		component->MinDrawDistance = 0.0f;
		component->SetBoundsScale(100.0f);
		component->MarkRenderStateDirty();

		if (auto particleComponent = Cast<UParticleSystemComponent>(component))
		{
			particleComponent->bOverrideLODMethod = true;
			particleComponent->LODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
			particleComponent->SetLODLevel(0);
			particleComponent->SetRequiredSignificance(EParticleSignificanceLevel::Low);
			particleComponent->SetManagingSignificance(false);
			particleComponent->SecondsBeforeInactive = 0.0f;
		}
		else if (auto niagaraComponent = Cast<UNiagaraComponent>(component))
		{
			niagaraComponent->SetAllowScalability(false);
			niagaraComponent->SetForceLocalPlayerEffect(true);
			niagaraComponent->SetLODDistance(0.0f, maxDistance);
			ConfigureNiagaraRenderers(niagaraComponent, componentState);
			ConfigureNiagaraMaterials(niagaraComponent, componentState);
		}

		for (int32 materialIndex = 0; materialIndex < component->GetNumMaterials(); ++materialIndex)
		{
			ConfigureMaterial(
				component->GetMaterial(materialIndex),
				[component, materialIndex](UMaterialInterface* parentMaterial)
				{
					return component->CreateDynamicMaterialInstance(materialIndex, parentMaterial);
				},
				componentState);
		}

		if (factory && factory->IsProducing())
		{
			if (!component->IsVisible())
			{
				component->SetVisibility(true, true);
				componentState.MadeVisibleBySmokey = true;
			}

			if (buildableState.LocalEffectComponents.Contains(component))
			{
				if (auto particleComponent = Cast<UParticleSystemComponent>(component))
				{
					particleComponent->ActivateSystem(true);
				}
				else
				{
					component->Activate(true);
				}

				componentState.ActivatedBySmokey = true;
			}
			else if (!component->IsActive())
			{
				component->Activate(true);
				componentState.ActivatedBySmokey = true;
			}
		}
		else if (factory)
		{
			if (componentState.ActivatedBySmokey)
			{
				component->Deactivate();
				componentState.ActivatedBySmokey = false;
			}

			if (componentState.MadeVisibleBySmokey)
			{
				component->SetVisibility(false, true);
				componentState.MadeVisibleBySmokey = false;
			}
		}
	}

	if (learnedEffectTemplates)
	{
		TArray<TWeakObjectPtr<AFGBuildable>> buildablesToConfigure;
		for (const TWeakObjectPtr<AFGBuildable>& pendingBuildable : pendingBuildables)
		{
			if (pendingBuildable.IsValid() && pendingBuildable->GetClass() == buildable->GetClass())
			{
				buildablesToConfigure.Add(pendingBuildable);
			}
		}

		for (const TWeakObjectPtr<AFGBuildable>& pendingBuildable : buildablesToConfigure)
		{
			pendingBuildables.Remove(pendingBuildable);
			pendingRetryCounts.Remove(pendingBuildable);
			USmokey_BPFL::ConfigureBuildableParticleSystems(pendingBuildable.Get());
		}
	}
}

void USmokey_BPFL::RestoreBuildableParticleSystems(AFGBuildable* buildable)
{
	if (!IsValid(buildable))
	{
		return;
	}

	const TWeakObjectPtr<AFGBuildable> buildableKey(buildable);
	auto buildableState = buildableStates.Find(buildableKey);
	if (!buildableState)
	{
		pendingBuildables.Remove(buildableKey);
		pendingRetryCounts.Remove(buildableKey);
		return;
	}

	for (const TWeakObjectPtr<UFXSystemComponent>& componentKey : buildableState->EffectComponents)
	{
		if (auto component = componentKey.Get())
		{
			if (auto componentState = componentStates.Find(componentKey))
			{
				RestoreComponent(component, *componentState);
			}
		}

		componentStates.Remove(componentKey);
	}

	if (buildableState->HasSignificanceRange)
	{
		if (auto factory = Cast<AFGBuildableFactory>(buildable))
		{
			SetGeneratorSignificanceRange(factory, buildableState->SignificanceRange);
		}
	}

	for (const TWeakObjectPtr<UFXSystemComponent>& component : buildableState->LocalEffectComponents)
	{
		if (component.IsValid())
		{
			component->DestroyComponent();
		}
	}

	buildableStates.Remove(buildableKey);
	pendingBuildables.Remove(buildableKey);
	pendingRetryCounts.Remove(buildableKey);
}
