#include "OutlineLootSubsystem.h"
#include "Patching/NativeHookManager.h"
#include "FGItemPickup.h"
#include "FGItemPickup_Spawnable.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <Kismet/GameplayStatics.h>
#include <OutlineLoot_ConfigurationStruct.h>

namespace
{
	const FName OutlineLootTag(TEXT("OutlineLoot"));

	bool IsOutlineComponent(const UActorComponent* component)
	{
		return component && (component->ComponentHasTag(OutlineLootTag) || component->GetName().StartsWith(TEXT("OutlineLoot")) || component->GetName().StartsWith(TEXT("OutlineStaticMesh")));
	}

	bool IsInstancedFruit(const UStaticMeshComponent* component)
	{
		const UInstancedStaticMeshComponent* instanced = Cast<UInstancedStaticMeshComponent>(component);
		return instanced && !IsOutlineComponent(instanced) && instanced->GetStaticMesh() != nullptr;
	}

	bool IsFruitVisible(const USceneComponent* component)
	{
		return component && component->IsVisible();
	}

	bool IsBushBody(const UStaticMeshComponent* component)
	{
		return component && !IsOutlineComponent(component) && component->GetName().Contains(TEXT("BushMesh"));
	}

	bool IsOutlineSource(const UStaticMeshComponent* component)
	{
		return component && !IsOutlineComponent(component) && !component->IsA<UInstancedStaticMeshComponent>() && component->GetStaticMesh() != nullptr;
	}

	UStaticMeshComponent* FindOutlineComponent(USceneComponent* source)
	{
		for (USceneComponent* child : source->GetAttachChildren())
		{
			if (UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(child))
			{
				if (IsOutlineComponent(mesh) && !mesh->IsA<UInstancedStaticMeshComponent>())
				{
					return mesh;
				}
			}
		}
		return nullptr;
	}

	UStaticMeshComponent* CreateOutlineComponent(AActor* owner, USceneComponent* parent, const FTransform& relativeTransform)
	{
		UStaticMeshComponent* outline = Cast<UStaticMeshComponent>(owner->AddComponentByClass(UStaticMeshComponent::StaticClass(), true, relativeTransform, false));
		if (!outline)
		{
			return nullptr;
		}

		outline->ComponentTags.Add(OutlineLootTag);
		outline->SetMobility(EComponentMobility::Movable);
		outline->AttachToComponent(parent, FAttachmentTransformRules::KeepRelativeTransform);
		return outline;
	}

	void ClearOutlines(AActor* owner)
	{
		TArray<UStaticMeshComponent*> meshComponents;
		owner->GetComponents<UStaticMeshComponent>(meshComponents);
		for (UStaticMeshComponent* meshComponent : meshComponents)
		{
			if (IsOutlineComponent(meshComponent))
			{
				meshComponent->DestroyComponent();
			}
		}
	}

	bool ShouldOutlinePickup(AFGItemPickup* pickup)
	{
		const FString actorName = pickup->GetClass()->GetName();
		if (actorName.StartsWith(TEXT("BP_Crystal")) || actorName.StartsWith(TEXT("BP_WAT")))
		{
			return false;
		}

		const TSubclassOf<UFGItemDescriptor> itemClass = pickup->GetPickupItemClass();
		if (!itemClass)
		{
			return true;
		}

		const FString itemName = itemClass->GetName();
		if (itemName.StartsWith(TEXT("Desc_Crystal")) || itemName.StartsWith(TEXT("Desc_WAT")))
		{
			return false;
		}
		if (itemName == TEXT("Desc_Berry_C"))
		{
			return FOutlineLoot_ConfigurationStruct::GetActiveConfig(pickup).Berry;
		}
		if (itemName == TEXT("Desc_Nut_C"))
		{
			return FOutlineLoot_ConfigurationStruct::GetActiveConfig(pickup).Nut;
		}
		if (itemName == TEXT("Desc_Shroom_C"))
		{
			return FOutlineLoot_ConfigurationStruct::GetActiveConfig(pickup).Bacon;
		}
		return true;
	}

	void ApplyOutlineMaterial(UStaticMeshComponent* outline, UStaticMesh* mesh, const FTransform& relativeTransform, UMaterialInterface* sourceMaterial, const FLinearColor& color, float scale, float cullDistance)
	{
		// Nanite ignores the outline material's vertex offset, so use the fallback mesh.
		outline->bDisallowNanite = true;
		outline->SetStaticMesh(mesh);
		outline->SetRelativeTransform(relativeTransform);
		outline->SetCullDistance(cullDistance);
		outline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		outline->SetGenerateOverlapEvents(false);

		const int32 numMaterials = FMath::Max(outline->GetNumMaterials(), 1);
		for (int32 materialIndex = 0; materialIndex < numMaterials; ++materialIndex)
		{
			if (UMaterialInstanceDynamic* dynamicMaterial = outline->CreateDynamicMaterialInstance(materialIndex, sourceMaterial))
			{
				dynamicMaterial->SetVectorParameterValue(TEXT("Color"), color);
				dynamicMaterial->SetScalarParameterValue(TEXT("Scale"), scale);
			}
		}
	}
}

void AOutlineLootSubsystem::BeginPlay()
{
	Super::BeginPlay();

	UConfigManager* ConfigManager = this->GetGameInstance()->GetSubsystem<UConfigManager>();
	FConfigId configId;
	configId.ModReference = "OutlineLoot";
	auto section = ConfigManager->GetConfigurationRootSection(configId);
	auto colorSection = section->SectionProperties["OutlineColor"];
	auto scaleSection = section->SectionProperties["OutlineSize"];
	lootOptionsChanged.BindUFunction(this, FName("OnLootOptionsChanged"));
	section->SectionProperties["Bacon"]->OnPropertyValueChanged.AddUnique(lootOptionsChanged);
	section->SectionProperties["Berry"]->OnPropertyValueChanged.AddUnique(lootOptionsChanged);
	section->SectionProperties["Nut"]->OnPropertyValueChanged.AddUnique(lootOptionsChanged);
	colorChanged.BindUFunction(this, FName("OnColorChanged"));
	sizeChanged.BindUFunction(this, FName("OnSizeChanged"));
	colorSection->OnPropertyValueChanged.AddUnique(colorChanged);
	scaleSection->OnPropertyValueChanged.AddUnique(sizeChanged);

	material = LoadObject<UMaterialInstance>(NULL, TEXT("/OutlineLoot/Materials/MI_Outline.MI_Outline"));
	ApplyToAll();

	AFGItemPickup* itemPickupCDO = GetMutableDefault<AFGItemPickup>();
	ItemPickupHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup::BeginPlay, itemPickupCDO, [this](AFGItemPickup* self)
		{
			ApplySingle(self);
		});

	AFGItemPickup_Spawnable* spawnableCDO = GetMutableDefault<AFGItemPickup_Spawnable>();
	Hook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup_Spawnable::BeginPlay, spawnableCDO, [this](AFGItemPickup_Spawnable* self)
		{
			ApplySingle(self);
		});

	VisualsHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup::UpdateVisuals, itemPickupCDO, [this](AFGItemPickup* self)
		{
			ApplySingle(self);
		});
}

void AOutlineLootSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ItemPickupHook.IsValid())
	{
		UNSUBSCRIBE_METHOD(AFGItemPickup::BeginPlay, ItemPickupHook);
	}
	if (Hook.IsValid())
	{
		UNSUBSCRIBE_METHOD(AFGItemPickup_Spawnable::BeginPlay, Hook);
	}
	if (VisualsHook.IsValid())
	{
		UNSUBSCRIBE_METHOD(AFGItemPickup::UpdateVisuals, VisualsHook);
	}
	Super::EndPlay(EndPlayReason);
}

FLinearColor AOutlineLootSubsystem::GetColor(AActor* actor)
{
	auto config = FOutlineLoot_ConfigurationStruct::GetActiveConfig(actor);
	auto colorInt = config.OutlineColor;
	switch (colorInt)
	{
	case 0:
		return FLinearColor::White;
	case 1:
		return FLinearColor::Red;
	case 2:
		return FLinearColor::Green;
	case 3:
		return FLinearColor::Yellow;
	case 4:
		return FLinearColor::Blue;
	case 5:
		return FLinearColor(0.913726, 0.0, 0.952941, 1.0);
	}
	return FLinearColor::White;
}

float AOutlineLootSubsystem::GetScale(AActor* actor)
{
	auto config = FOutlineLoot_ConfigurationStruct::GetActiveConfig(actor);
	auto scale = config.OutlineSize;
	return scale;
}

void AOutlineLootSubsystem::ApplyToAll()
{
	TArray<AActor*> out_Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AFGItemPickup::StaticClass(), out_Actors);
	for (auto actor : out_Actors)
	{
		auto pickup = Cast<AFGItemPickup>(actor);
		if (pickup)
		{
			ApplySingle(pickup);
		}
	}
}

void AOutlineLootSubsystem::ApplySingle(AFGItemPickup* pickup)
{
	if (!pickup || !material)
	{
		return;
	}

	if (!ShouldOutlinePickup(pickup))
	{
		ClearOutlines(pickup);
		return;
	}

	const FLinearColor color = GetColor(pickup);
	const float scale = GetScale(pickup);
	const float cullDistance = pickup->GetSignificanceRange_Implementation() * 5.f;

	TArray<UStaticMeshComponent*> meshComponents;
	pickup->GetComponents<UStaticMeshComponent>(meshComponents);

	bool bHasInstancedFruit = false;
	bool bHasBushBody = false;
	bool bHasLooseFruit = false;
	bool bLooseFruitVisible = false;
	for (UStaticMeshComponent* source : meshComponents)
	{
		if (IsInstancedFruit(source))
		{
			bHasInstancedFruit = true;
		}
		else if (IsBushBody(source))
		{
			bHasBushBody = true;
		}
		else if (IsOutlineSource(source))
		{
			bHasLooseFruit = true;
			bLooseFruitVisible |= IsFruitVisible(source);
		}
	}
	const bool bFruitPickup = bHasInstancedFruit || bHasBushBody || InstancedFruitPickups.Contains(pickup);
	bool bFruitAvailable = false;
	if (bFruitPickup && !pickup->IsPickedUp())
	{
		for (UStaticMeshComponent* source : meshComponents)
		{
			const UInstancedStaticMeshComponent* instanced = Cast<UInstancedStaticMeshComponent>(source);
			if (instanced && IsInstancedFruit(instanced) && instanced->GetInstanceCount() > 0 && IsFruitVisible(instanced))
			{
				bFruitAvailable = true;
				break;
			}
		}
		bFruitAvailable |= bHasBushBody && bLooseFruitVisible;
		if (!bHasInstancedFruit && !bHasLooseFruit && bHasBushBody)
		{
			bFruitAvailable = true;
		}
	}

	TSet<UStaticMeshComponent*> activeOutlines;
	for (UStaticMeshComponent* source : meshComponents)
	{
		if (UInstancedStaticMeshComponent* instanced = Cast<UInstancedStaticMeshComponent>(source))
		{
			if (!IsInstancedFruit(instanced) || pickup->IsPickedUp() || instanced->GetInstanceCount() == 0 || !IsFruitVisible(instanced))
			{
				continue;
			}

			TArray<UStaticMeshComponent*> existingOutlines;
			for (USceneComponent* child : instanced->GetAttachChildren())
			{
				if (UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(child))
				{
					if (IsOutlineComponent(mesh) && !mesh->IsA<UInstancedStaticMeshComponent>())
					{
						existingOutlines.Add(mesh);
					}
				}
			}

			const int32 instanceCount = instanced->GetInstanceCount();
			for (int32 instanceIndex = 0; instanceIndex < instanceCount; ++instanceIndex)
			{
				FTransform instanceTransform;
				if (!instanced->GetInstanceTransform(instanceIndex, instanceTransform, false))
				{
					continue;
				}

				UStaticMeshComponent* outline = existingOutlines.IsValidIndex(instanceIndex) ? existingOutlines[instanceIndex] : nullptr;
				if (!outline)
				{
					outline = CreateOutlineComponent(pickup, instanced, instanceTransform);
					if (!outline)
					{
						continue;
					}
				}

				ApplyOutlineMaterial(outline, instanced->GetStaticMesh(), instanceTransform, material, color, scale, cullDistance);
				activeOutlines.Add(outline);
			}
			continue;
		}

		if ((bFruitPickup && !bFruitAvailable) || !IsOutlineSource(source) || !IsFruitVisible(source))
		{
			continue;
		}

		UStaticMeshComponent* outline = FindOutlineComponent(source);
		if (!outline)
		{
			outline = CreateOutlineComponent(pickup, source, FTransform::Identity);
			if (!outline)
			{
				continue;
			}
		}

		ApplyOutlineMaterial(outline, source->GetStaticMesh(), FTransform::Identity, material, color, scale, cullDistance);
		activeOutlines.Add(outline);
	}

	for (UStaticMeshComponent* meshComponent : meshComponents)
	{
		if (IsOutlineComponent(meshComponent) && !activeOutlines.Contains(meshComponent))
		{
			meshComponent->DestroyComponent();
		}
	}

	if (bHasInstancedFruit || bHasBushBody)
	{
		InstancedFruitPickups.Add(pickup);
	}
}

void AOutlineLootSubsystem::OnColorChanged()
{
	ApplyToAll();
}

void AOutlineLootSubsystem::OnSizeChanged()
{
	ApplyToAll();
}

void AOutlineLootSubsystem::OnLootOptionsChanged()
{
	ApplyToAll();
}
