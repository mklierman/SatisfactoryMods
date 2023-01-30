#include "PersistentPaintablesCppSubSystem.h"
#include "FGGameState.h"
#include "FGBuildableSubsystem.h"
#include "Hologram/FGHologram.h"
#include "FGColorInterface.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"

APersistentPaintablesCppSubSystem::APersistentPaintablesCppSubSystem()
{

}

#pragma optimize("", off)
void APersistentPaintablesCppSubSystem::HookConstruct()
{
#if !WITH_EDITOR
	if (HasAuthority())
	{
		//virtual AActor* Construct( TArray< AActor* >& out_children, FNetConstructionID constructionID );
		SUBSCRIBE_METHOD_AFTER(AFGBuildableSubsystem::AddBuildable, [=](AFGBuildableSubsystem* self, class AFGBuildable* buildable)
			{
				if (buildable)
				{
					if (buildable->GetCanBeColored_Implementation())
					{
						if (PlayerCustomizationStructs.Num() > 0)
						{
							if (auto instigator = buildable->mBuildEffectInstignator)
							{
								for (auto custData : PlayerCustomizationStructs)
								{
									if (custData.CharacterPlayer && custData.CharacterPlayer == instigator && custData.CustomizationData.SwatchDesc)
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
			});
		//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::OnFullRebuildCompleted, &UpdateColor)
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
	if (HasAuthority())
	{
#if !WITH_EDITOR
#endif
		//void AddFluidIntegrant(class IFGFluidIntegrantInterface* fluidIntegrant);
		//AFGBuildablePipeline* BuildablePipeline = GetMutableDefault<AFGBuildablePipeline>();
		SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::UpdateFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
			{
				if (self && IsInGameThread())
				{
					//FTimerHandle FluidColorTimerHandle;
					//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
					this->UpdateColor(self);
				}
			});
		SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::AddFluidIntegrant, [=](AFGPipeNetwork* self, class IFGFluidIntegrantInterface* fluidIntegrant)
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
								}
							}
						}
					}
					
					//FTimerHandle FluidColorTimerHandle;
					//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
					//this->UpdateColor(self);
				}
			});
	}
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

		if (auto pipe = Cast<AFGBuildablePipeline>(buildable))
		{
			if (pipe->mPipeConnections.Num() > 0)
			{
				for (auto conn : pipe->mPipeConnections)
				{
					if (conn->mConnectedComponent)
					{
						if (auto owner = conn->mConnectedComponent->GetOwner())
						{
							if (auto junction = Cast<AFGBuildablePipelineJunction>(owner))
							{
								ApplyColor(junction, swatchClass, newData);
							}
							else if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(owner))
							{
								ApplyColor(pipeSupport, swatchClass, newData);
							}
							else if (auto newPipe = Cast<AFGBuildablePipeline>(owner)) // Might be connected to a pipe through a support
							{
								auto supports = FindNearbySupports(pipe, conn);
								if (supports.Num() > 0)
								{
									for (auto support : supports)
									{
										if (auto fgpipeSupport = Cast<AFGBuildable>(support))
										{
											if (auto pipeSupport2 = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
											{
												ApplyColor(fgpipeSupport, swatchClass, newData);
											}
										}
										else if (auto aim = Cast<AAbstractInstanceManager>(support))
										{
											if (aim->InstanceMap.Num() > 0)
											{
												for (auto instance : aim->InstanceMap)
												{
													if (instance.Key.ToString().StartsWith("SM_PipelineSupport"))
													{
														if (instance.Value.InstanceHandles.Num() > 0)
														{
															for (auto ihandle : instance.Value.InstanceHandles)
															{
																if (auto wallSupport = Cast<AFGBuildable>(ihandle->GetOwner()))
																{
																	ApplyColor(wallSupport, swatchClass, newData);
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
					else //Connection isn't connected to anything. May be connected to a support
					{
						auto supports = FindNearbySupports(pipe, conn);
						if (supports.Num() > 0)
						{
							for (auto support : supports)
							{
								if (auto fgpipeSupport = Cast<AFGBuildable>(support))
								{
									if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
									{
										ApplyColor(fgpipeSupport, swatchClass, newData);
									}
									else if (auto aim = Cast<AAbstractInstanceManager>(support))
									{
										if (aim->InstanceMap.Num() > 0)
										{
											for (auto instance : aim->InstanceMap)
											{
												if (instance.Key.ToString().StartsWith("SM_PipelineSupport"))
												{
													if (instance.Value.InstanceHandles.Num() > 0)
													{
														for (auto ihandle : instance.Value.InstanceHandles)
														{
															if (auto wallSupport = Cast<AFGBuildable>(ihandle->GetOwner()))
															{
																ApplyColor(wallSupport, swatchClass, newData);
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
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
		/*buildable->ApplyCustomizationData_Implementation(customizationData);
		buildable->SetCustomizationData_Implementation(customizationData);*/
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
#pragma optimize("", on)

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

		//Subsystem->mColorSlots_Data[ColorSlot] = NewColourSlot;
		//FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);
		FGGameState->Server_SetBuildingColorDataForSlot(ColorSlot, NewColourSlot);

		Subsystem->SetColorSlot_Data(ColorSlot, NewColourSlot);
		Subsystem->mColorSlotsAreDirty = true;

		//FGGameState->mBuildingColorSlots_Data[ColorSlot] = Subsystem->mColorSlots_Data[ColorSlot];
		//FGGameState->SetupColorSlots_Data(Subsystem->mColorSlots_Data);

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
