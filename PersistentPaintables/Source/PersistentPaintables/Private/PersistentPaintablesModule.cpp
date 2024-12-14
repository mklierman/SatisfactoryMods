#include "PersistentPaintablesModule.h"
#include "SessionSettings/SessionSettingsManager.h"
#include <Buildables/FGBuildableFoundation.h>
#include "FGLightweightBuildableSubsystem.h"
#include <Logging/StructuredLog.h>
#include "Hologram/FGFoundationHologram.h"
#include <PP_ActorComponent.h>


DEFINE_LOG_CATEGORY(PersistentPaintables_Log);
#pragma optimize("", off)
void FPersistentPaintablesModule::ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData)
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


TArray<AActor*> FPersistentPaintablesModule::FindNearbySupports(AFGBuildable* pipe, UFGPipeConnectionComponentBase* pipeConn)
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


void FPersistentPaintablesModule::UpdateColor(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
{
	////UE_LOG(PersistentPaintables_Log, Display, TEXT("UpdateFluidDescriptor"));
	//if (self && self->mFluidIntegrants.Num() > 0)
	//{
	//	for (auto fi : self->mFluidIntegrants)
	//	{
	//		auto buildable = Cast< AFGBuildable>(fi);
	//		if (buildable)
	//		{
	//			auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(descriptor);
	//			FFactoryCustomizationData newData = FFactoryCustomizationData();
	//			newData.SwatchDesc = swatchClass;
	//			newData.ColorSlot = 255;
	//			newData.OverrideColorData.PrimaryColor = fluidColor;
	//			newData.OverrideColorData.SecondaryColor = fluidColor;
	//			newData.Data = buildable->mCustomizationData.Data;
	//			newData.ExtraData = buildable->mCustomizationData.ExtraData;
	//			newData.NeedsSkinUpdate = true;

	//			ApplyColor(buildable, swatchClass, newData);

	//			if (auto pipe = Cast<AFGBuildablePipeline>(buildable))
	//			{
	//				if (pipe->mPipeConnections.Num() > 0)
	//				{
	//					for (auto conn : pipe->mPipeConnections)
	//					{
	//						if (conn->mConnectedComponent)
	//						{
	//							if (auto owner = conn->mConnectedComponent->GetOwner())
	//							{
	//								if (auto junction = Cast<AFGBuildablePipelineJunction>(owner))
	//								{
	//									ApplyColor(junction, swatchClass, newData);
	//								}
	//								else if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(owner))
	//								{
	//									ApplyColor(pipeSupport, swatchClass, newData);
	//								}
	//								else if (auto newPipe = Cast<AFGBuildablePipeline>(owner)) // Might be connected to a pipe through a support
	//								{
	//									auto supports = FindNearbySupports(pipe, conn);
	//									if (supports.Num() > 0)
	//									{
	//										for (auto support : supports)
	//										{
	//											if (auto fgpipeSupport = Cast<AFGBuildable>(support))
	//											{
	//												if (auto pipeSupport2 = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
	//												{
	//													ApplyColor(fgpipeSupport, swatchClass, newData);
	//												}
	//											}
	//											else if (auto aim = Cast<AAbstractInstanceManager>(support))
	//											{
	//												if (aim->InstanceMap.Num() > 0)
	//												{
	//													for (auto instance : aim->InstanceMap)
	//													{
	//														if (instance.Key.ToString().StartsWith("SM_PipelineSupport"))
	//														{
	//															if (instance.Value.InstanceHandles.Num() > 0)
	//															{
	//																for (auto ihandle : instance.Value.InstanceHandles)
	//																{
	//																	if (auto wallSupport = Cast<AFGBuildable>(ihandle->GetOwner()))
	//																	{
	//																		FTimerHandle FluidColorTimerHandle;
	//																		//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [&]() { this->ApplyColor(wallSupport, swatchClass, newData); }, 0.f, false, 1.f);
	//																		ApplyColor(wallSupport, swatchClass, newData);
	//																	}
	//																}
	//															}
	//														}
	//													}
	//												}
	//											}
	//										}
	//									}
	//								}
	//							}
	//						}
	//						else //Connection isn't connected to anything. May be connected to a support
	//						{
	//							auto supports = FindNearbySupports(pipe, conn);
	//							if (supports.Num() > 0)
	//							{
	//								for (auto support : supports)
	//								{
	//									if (auto fgpipeSupport = Cast<AFGBuildable>(support))
	//									{
	//										if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
	//										{
	//											ApplyColor(fgpipeSupport, swatchClass, newData);
	//										}
	//										else if (auto aim = Cast<AAbstractInstanceManager>(support))
	//										{
	//											if (aim->InstanceMap.Num() > 0)
	//											{
	//												for (auto instance : aim->InstanceMap)
	//												{
	//													if (instance.Key.ToString().StartsWith("SM_PipelineSupport"))
	//													{
	//														if (instance.Value.InstanceHandles.Num() > 0)
	//														{
	//															for (auto ihandle : instance.Value.InstanceHandles)
	//															{
	//																if (auto wallSupport = Cast<AFGBuildable>(ihandle->GetOwner()))
	//																{
	//																	FTimerHandle FluidColorTimerHandle;
	//																	//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [&]() { this->ApplyColor(wallSupport, swatchClass, newData); }, 0.f, false, 1.f);
	//																	ApplyColor(wallSupport, swatchClass, newData);
	//																}
	//															}
	//														}
	//													}
	//												}
	//											}
	//										}
	//									}
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

void FPersistentPaintablesModule::UpdateNetworkColor(AFGPipeNetwork* pipeNetwork)
{
	if (pipeNetwork && pipeNetwork->mFluidIntegrants.Num() > 0)
	{
		for (auto fi : pipeNetwork->mFluidIntegrants)
		{
			auto buildable = Cast< AFGBuildable>(fi);
			UpdateColorSingle(buildable, pipeNetwork);
		}
	}
}

void FPersistentPaintablesModule::UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork)
{
	if (buildable)
	{
		auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(pipeNetwork->GetFluidDescriptor());
		//if (fluidColor == FLinearColor())
		//{
		//	return;
		//}
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


void FPersistentPaintablesModule::StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
{
	if (self)
	{
		FTimerHandle FluidColorTimerHandle;
		self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [&]() { this->UpdateColor(self, descriptor); }, 1.f, false, 1.f);
	}
}


void FPersistentPaintablesModule::StartupModule() {

	wallSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWall/Build_PipelineSupportWall.Build_PipelineSupportWall_C"));
	wallHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWallHole/Build_PipelineSupportWallHole.Build_PipelineSupportWallHole_C"));
	floorHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundationPassthrough/Build_FoundationPassthrough_Pipe.Build_FoundationPassthrough_Pipe_C"));
	swatchClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/-Shared/Customization/Swatches/SwatchDesc_Custom.SwatchDesc_Custom_C"));

	//AFGBuildablePipeline* BuildablePipeline = GetMutableDefault<AFGBuildablePipeline>();
	//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::UpdateFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
	//	{
	//		if (self)
	//		{
	//			this->UpdateColor(self, descriptor);
	//		}
	//	});
		//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::OnFullRebuildCompleted, &UpdateColor)
	//void CopyCustomizationDataFromTemporaryToInstance(AFGBuildable * buildable);
	//SUBSCRIBE_METHOD(AFGLightweightBuildableSubsystem::CopyCustomizationDataFromTemporaryToInstance, [=](auto& scope, AFGLightweightBuildableSubsystem* self, AFGBuildable* buildable)
	//	{
	//		UE_LOG(PersistentPaintables_Log, Display, TEXT("CopyCustomizationDataFromTemporaryToInstance"));
	//		if (buildable)
	//		{

	//			if (buildable->GetCanBeColored_Implementation())
	//			{
	//				if (auto instigatorPawn = buildable->mBuildEffectInstignator)
	//				{
	//					UE_LOG(PersistentPaintables_Log, Display, TEXT("instigatorPawn"));
	//					auto instigator = Cast <AFGCharacterPlayer>(instigatorPawn);
	//					if (instigator)
	//					{
	//						UE_LOG(PersistentPaintables_Log, Display, TEXT("instigator"));
	//						auto component = instigator->GetComponentByClass<UPP_ActorComponent>();
	//						if (component)
	//						{
	//							UE_LOG(PersistentPaintables_Log, Display, TEXT("component"));
	//							buildable->SetCustomizationData_Implementation(component->CustomizationStruct);
	//							buildable->ApplyCustomizationData_Implementation(component->CustomizationStruct);
	//							buildable->SetCustomizationDataLightweightNoApply(component->CustomizationStruct);
	//							return;
	//						}
	//					}
	//				}
	//				//USubsystemActorManager* SubsystemActorManager = buildable->GetWorld()->GetSubsystem<USubsystemActorManager>();
	//				//APersistentPaintablesCppSubSystem* ppSubsystem = SubsystemActorManager->GetSubsystemActor<APersistentPaintablesCppSubSystem>();

	//				//if (ppSubsystem && !ppSubsystem->PlayerCustomizationStructs.IsEmpty())
	//				//{
	//				//	if (auto instigator = buildable->mBuildEffectInstignator)
	//				//	{
	//				//		for (FPlayerCustomizationStruct custData : ppSubsystem->PlayerCustomizationStructs)
	//				//		{
	//				//			if (custData.CharacterPlayer && custData.CustomizationData.IsInitialized() && custData.CharacterPlayer == instigator)
	//				//			{
	//				//				//auto lightweightFoundation = Cast<AFGBuildableFoundationLightweight>(buildable);
	//				//				buildable->SetCustomizationData_Implementation(custData.CustomizationData);
	//				//				buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
	//				//				buildable->SetCustomizationDataLightweightNoApply(custData.CustomizationData);
	//				//				return;
	//				//			}
	//				//		}
	//				//	}
	//				//}
	//			}
	//		}
	//	});
#if !WITH_EDITOR
	AFGBuildableHologram* hg = GetMutableDefault<AFGBuildableHologram>();
	AFGFoundationHologram* fhg = GetMutableDefault<AFGFoundationHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, hg, [=](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			//UE_LOG(PersistentPaintables_Log, Display, TEXT("ConfigureActor"));
			if (self)
			{
				AddBuildable(inBuildable, self);
				return;
				//USubsystemActorManager* SubsystemActorManager = self->GetWorld()->GetSubsystem<USubsystemActorManager>();
				//APersistentPaintablesCppSubSystem* ppSubsystem = SubsystemActorManager->GetSubsystemActor<APersistentPaintablesCppSubSystem>();

				//if (ppSubsystem && !ppSubsystem->PlayerCustomizationStructs.IsEmpty())
				//{
				//	auto instigator = self->GetConstructionInstigator();
				//	if (instigator)
				//	{
				//		for (FPlayerCustomizationStruct custData : ppSubsystem->PlayerCustomizationStructs)
				//		{
				//			if (custData.CharacterPlayer && custData.CustomizationData.IsInitialized() && custData.CharacterPlayer == instigator)
				//			{
				//				//self->mCustomizationData = custData.CustomizationData;
				//				//self->mDefaultSwatch = custData.CustomizationData.SwatchDesc;
				//				inBuildable->SetCustomizationData_Implementation(custData.CustomizationData);
				//				inBuildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
				//				inBuildable->SetCustomizationDataLightweightNoApply(custData.CustomizationData);
				//				//UE_LOGFMT(PersistentPaintables_Log, Display, "Set data");
				//				return;
				//			}
				//		}
				//	}
				//}
			}
		});

	//ConstructHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, hg, [=](auto& returnActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID constructionID)
	//	{
	//		//AActor* actor = const_cast<AActor*>(returnActor);
	//		AFGBuildable* buildable = Cast< AFGBuildable>(returnActor);
	//		if (buildable)
	//		{
	//			//AddBuildable(buildable, self);
	//		}
	//	});

	HookPipes();

#endif
}

void FPersistentPaintablesModule::ShutdownModule()
{
	if (ConstructHook.IsValid())
	{
		UNSUBSCRIBE_METHOD(AFGBuildableHologram::Construct, ConstructHook);
	}
}

void FPersistentPaintablesModule::AddBuildable(AFGBuildable* buildable, const AFGBuildableHologram* hologram)
{
	//UE_LOG(PersistentPaintables_Log, Display, TEXT("AddBuildable"));
	if (buildable)
	{
		if (buildable->GetCanBeColored_Implementation())
		{
			//UE_LOG(PersistentPaintables_Log, Display, TEXT("GetCanBeColored_Implementation"));
			if (auto instigatorPawn = hologram->GetConstructionInstigator())
			{
				//UE_LOG(PersistentPaintables_Log, Display, TEXT("instigatorPawn"));
				auto instigator = Cast <AFGCharacterPlayer>(instigatorPawn);
				if (instigator)
				{
					auto component = instigator->GetComponentByClass<UPP_ActorComponent>();
					if (component)
					{
						//UE_LOG(PersistentPaintables_Log, Display, TEXT("component"));
						//auto lightweightFoundation = Cast<AFGBuildableFoundationLightweight>(buildable);
						buildable->SetCustomizationData_Implementation(component->CustomizationStruct);
						buildable->ApplyCustomizationData_Implementation(component->CustomizationStruct);
						buildable->SetCustomizationDataLightweightNoApply(component->CustomizationStruct);
						return;
					}
				}
			}
			return;
			//USubsystemActorManager* SubsystemActorManager = buildable->GetWorld()->GetSubsystem<USubsystemActorManager>();
			//APersistentPaintablesCppSubSystem* ppSubsystem = SubsystemActorManager->GetSubsystemActor<APersistentPaintablesCppSubSystem>();
			//if (ppSubsystem)
			//{
			//	UE_LOG(PersistentPaintables_Log, Display, TEXT("ppSubsystem"));
			//}
			//if (ppSubsystem && !ppSubsystem->PlayerCustomizationStructs.IsEmpty())
			//{
			//	UE_LOG(PersistentPaintables_Log, Display, TEXT("IsEmpty"));
			//	if (auto instigatorPawn = hologram->GetConstructionInstigator())
			//	{
			//		UE_LOG(PersistentPaintables_Log, Display, TEXT("instigatorPawn"));
			//		auto instigator = Cast <AFGCharacterPlayer>(instigatorPawn);
			//		if (instigator)
			//		{
			//			UE_LOG(PersistentPaintables_Log, Display, TEXT("instigator"));
			//			for (FPlayerCustomizationStruct custData : ppSubsystem->PlayerCustomizationStructs)
			//			{
			//				if (custData.CharacterPlayer && custData.CustomizationData.IsInitialized() && custData.CharacterPlayer == instigator)
			//				{
			//					UE_LOG(PersistentPaintables_Log, Display, TEXT("custData.CharacterPlayer == instigator"));
			//					//auto lightweightFoundation = Cast<AFGBuildableFoundationLightweight>(buildable);
			//					buildable->SetCustomizationData_Implementation(custData.CustomizationData);
			//					buildable->ApplyCustomizationData_Implementation(custData.CustomizationData);
			//					buildable->SetCustomizationDataLightweightNoApply(custData.CustomizationData);
			//					return;
			//				}
			//			}
			//		}
			//	}
			//}
		}
	}
}

void FPersistentPaintablesModule::HookPipes()
{


	//void TrySetFluidDescriptor(TSubclassOf< UFGItemDescriptor > newItemDescriptor
	//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::TrySetFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
	//	{
	//		UE_LOG(PersistentPaintables_Log, Display, TEXT("AFGPipeNetwork::TrySetFluidDescriptor"));
	//		if (self && IsInGameThread())
	//		{
	//			FPersistentPaintables_ConfigStruct config = FPersistentPaintables_ConfigStruct::GetActiveConfig(self->GetWorld());
	//			if (config.PaintPipes)
	//			{
	//				//FTimerHandle FluidColorTimerHandle;
	//				//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
	//				this->UpdateNetworkColor(self);
	//			}
	//		}
	//	});

	//void FlushIntegrant(class IFGFluidIntegrantInterface*);
	//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::FlushIntegrant, [=](AFGPipeNetwork* self, class IFGFluidIntegrantInterface* itf)
	//	{
	//		UE_LOG(PersistentPaintables_Log, Display, TEXT("AFGPipeNetwork::FlushIntegrant"));
	//		if (self && IsInGameThread())
	//		{
	//			FPersistentPaintables_ConfigStruct config = FPersistentPaintables_ConfigStruct::GetActiveConfig(self->GetWorld());
	//			if (config.PaintPipes)
	//			{
	//				//FTimerHandle FluidColorTimerHandle;
	//				//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
	//				this->UpdateNetworkColor(self);
	//			}
	//			//}
	//		}
	//	});

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::UpdateFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			if (optionValue)
			{
				//UE_LOG(PersistentPaintables_Log, Display, TEXT("AFGPipeNetwork::UpdateFluidDescriptor"));
				AsyncTask(ENamedThreads::GameThread, [=]() {
					this->UpdateNetworkColor(self);
					});
			}

			//if (self && IsInGameThread())
			//{
			//	FPersistentPaintables_ConfigStruct config = FPersistentPaintables_ConfigStruct::GetActiveConfig(self->GetWorld());
			//	if (config.PaintPipes)
			//	{
			//		//FTimerHandle FluidColorTimerHandle;
			//		//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
			//		this->UpdateNetworkColor(self);
			//	}
			//}
		});

	SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::AddFluidIntegrant, [=](AFGPipeNetwork* self, class IFGFluidIntegrantInterface* fluidIntegrant)
		{
			if (self && IsInGameThread())
			{
				FPersistentPaintables_ConfigStruct config = FPersistentPaintables_ConfigStruct::GetActiveConfig(self->GetWorld());
				if (config.PaintPipes)
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
									//this->UpdateNetworkColor(self);
								}
							}
						}
					}

					//FTimerHandle FluidColorTimerHandle;
					//self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [=]() { this->UpdateColor(self, descriptor); }, 5.f, false, 5.f);
					//this->UpdateColor(self);
				}
			}
		});
#endif
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);