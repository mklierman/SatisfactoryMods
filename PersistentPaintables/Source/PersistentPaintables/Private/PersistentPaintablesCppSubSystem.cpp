#include "PersistentPaintablesCppSubSystem.h"
#include "FGGameState.h"
#include "FGBuildableSubsystem.h"
#include "Hologram/FGBuildableHologram.h"
#include "FGColorInterface.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include <Kismet/GameplayStatics.h>

//#pragma optimize("", off)
APersistentPaintablesCppSubSystem::APersistentPaintablesCppSubSystem()
{
	
}

void APersistentPaintablesCppSubSystem::BeginPlay()
{
	if (HasAuthority())
	{
		TArray<FPlayerCustomizationStruct> newStructs;
		PlayerCustomizationStructs = newStructs;
	}
	Super::BeginPlay();
}

void APersistentPaintablesCppSubSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void APersistentPaintablesCppSubSystem::HookConstruct()
{
#if !WITH_EDITOR
	if (HasAuthority())
	{
		AFGBuildableHologram* hg = GetMutableDefault<AFGBuildableHologram>();
		ConstructHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, hg, [this](auto& returnActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID constructionID)
			{
				//AActor* actor = const_cast<AActor*>(returnActor);
				AFGBuildable* buildable = Cast< AFGBuildable>(returnActor);
				if (buildable)
				{
					AddBuildable(buildable);
				}
			});
	}
#endif
}

TArray<AActor*> FindNearbySupports(AFGBuildable* pipe, UFGPipeConnectionComponentBase* pipeConn)
{
	FVector connLoc = pipeConn->Bounds.Origin;

	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

	TArray<AActor*> ignoreActors;
	ignoreActors.Init(pipe, 1);

	TArray<AActor*> outActors;
	TArray<UPrimitiveComponent*> outComponents;
	TArray<FHitResult> outHits;

	float radius = 5.0f;

	UClass* seekClass = AActor::StaticClass(); // NULL;
	UKismetSystemLibrary::SphereOverlapActors(pipe->GetWorld(), connLoc, radius, traceObjectTypes, seekClass, ignoreActors, outActors);
	return outActors;
}

void APersistentPaintablesCppSubSystem::HookPipeNetwork()
{

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::TrySetFluidDescriptor, [this](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
		{
			if (self && IsInGameThread())
			{
					//FTimerHandle FluidColorTimerHandle;
					//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [this]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
					this->UpdateColor(self);
			}
		});

		SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::AddFluidIntegrant, [this](AFGPipeNetwork* self, class IFGFluidIntegrantInterface* fluidIntegrant)
			{
				if (self && IsInGameThread())
				{
					auto pipeBase = Cast<UFGPipeConnectionComponentBase>(fluidIntegrant);
					if (pipeBase && pipeBase->GetPipeConnectionType() != EPipeConnectionType::PCT_CONSUMER && pipeBase->GetPipeConnectionType() != EPipeConnectionType::PCT_PRODUCER)
					{
						auto pipeCons = fluidIntegrant->GetPipeConnections();
						if (pipeCons.Num() > 0)
						{
							if (auto owner = pipeCons[0]->GetOwner())
							{
								if (auto buildable = Cast<AFGBuildable>(owner))
								{
									UpdateColorSingle(buildable, self);
									UpdateColor(self);
								}
							}
						}
					}
				}
			});
#endif
}

void APersistentPaintablesCppSubSystem::UpdateColor(AFGPipeNetwork* pipeNetwork)
{
	if (HasAuthority() && pipeNetwork && pipeNetwork->mFluidIntegrants.Num() > 0)
	{
		for (auto fi : pipeNetwork->mFluidIntegrants)
		{
			auto buildable = Cast< AFGBuildable>(fi);
			UpdateColorSingle(buildable, pipeNetwork);
		}
	}
}

void APersistentPaintablesCppSubSystem::UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork)
{
	if (buildable)
	{
		auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(pipeNetwork->GetFluidDescriptor());

		FFactoryCustomizationData newData = FFactoryCustomizationData();
		newData.SwatchDesc = swatchClass;
		newData.ColorSlot = 255;
		newData.OverrideColorData.PrimaryColor = fluidColor;
		newData.OverrideColorData.SecondaryColor = fluidColor;
		newData.Data = buildable->mCustomizationData.Data;
		newData.ExtraData = buildable->mCustomizationData.ExtraData;
		newData.NeedsSkinUpdate = true;

		ApplyColor(buildable, swatchClass, newData);
		return;
	}
}

void APersistentPaintablesCppSubSystem::ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData)
{
	if (buildable && buildable->GetCanBeColored_Implementation())
	{
		buildable->mColorSlot = 255;
		buildable->mDefaultSwatchCustomizationOverride = inSwatchClass;
		buildable->ApplyCustomizationData_Implementation(customizationData);
		buildable->SetCustomizationData_Implementation(customizationData);
	}
}

void APersistentPaintablesCppSubSystem::PaintBuildableConstructed(AFGBuildable* buildable)
{
		if (PlayerCustomizationStructs.Num() > 0)
		{
			if (auto instigator = buildable->mBuildEffectInstignator)
			{
				for (auto custData : PlayerCustomizationStructs)
				{
					if (custData.CharacterPlayer && custData.CharacterPlayer == instigator)
					{
						buildable->SetCustomizationData_Implementation(custData.CustomizationData);
						buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
						return;
					}
				}
			}
		}
}

void APersistentPaintablesCppSubSystem::SetCustomSwatchColor(uint8 ColorSlot, FLinearColor PColor, FLinearColor SColor)
{
	AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(this);
	AFGGameState* FGGameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));

	if (Subsystem && FGGameState)
	{

		FFactoryCustomizationColorSlot NewColourSlot = FFactoryCustomizationColorSlot(
			PColor,
			SColor
		);

		FGGameState->Server_SetBuildingColorDataForSlot(ColorSlot, NewColourSlot);

		Subsystem->SetColorSlot_Data(ColorSlot, NewColourSlot);
		Subsystem->mColorSlotsAreDirty = true;
	}
}

void APersistentPaintablesCppSubSystem::ApplyCustomizationPreview(AFGBuildableSubsystem* buildableSubsystem, AFGBuildable* buildable, const FFactoryCustomizationData& previewData)
{
	buildableSubsystem->ApplyCustomizationPreview(buildable, previewData);
}

TSubclassOf<class UFGItemDescriptor> APersistentPaintablesCppSubSystem::GetBuildableDescriptor(AFGBuildable* buildable)
{
	return buildable->GetBuiltWithDescriptor();
}

void APersistentPaintablesCppSubSystem::AddBuildable(AFGBuildable* buildable)
{
	if (buildable)
	{
		if (buildable->GetCanBeColored_Implementation())
		{
			if (!PlayerCustomizationStructs.IsEmpty())
			{
				if (auto instigator = buildable->mBuildEffectInstignator)
				{
					for (FPlayerCustomizationStruct custData : PlayerCustomizationStructs)
					{
						if (custData.CharacterPlayer && custData.CustomizationData.IsInitialized() && custData.CharacterPlayer == instigator)
						{
							buildable->SetCustomizationData_Implementation(custData.CustomizationData);
							buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
							return;
						}
					}
				}
			}
		}
	}
}

//#pragma optimize("", on)
