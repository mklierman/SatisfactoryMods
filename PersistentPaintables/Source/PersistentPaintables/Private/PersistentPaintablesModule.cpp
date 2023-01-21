#include "PersistentPaintablesModule.h"


//DEFINE_LOG_CATEGORY(PersistentPaintables_Log);
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

//TArray<AActor*> FindNearbySupports(AFGBuildable* pipe, UFGPipeConnectionComponentBase* pipeConn)
//{
//	FVector connLoc = pipeConn->Bounds.Origin;
//
//	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
//	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
//	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
//
//	TArray<AActor*> ignoreActors;
//	ignoreActors.Init(pipe, 1);
//
//	TArray<AActor*> outActors;
//	TArray<UPrimitiveComponent*> outComponents;
//	TArray<FHitResult> outHits;
//
//	float radius = 5.0f;
//
//	UClass* seekClass = AActor::StaticClass(); // NULL;
//	UKismetSystemLibrary::SphereOverlapActors(pipe->GetWorld(), connLoc, radius, traceObjectTypes, seekClass, ignoreActors, outActors);
//	return outActors;
//}


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


void FPersistentPaintablesModule::StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
{
	if (self)
	{
		FTimerHandle FluidColorTimerHandle;
		self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [&]() { this->UpdateColor(self, descriptor); }, 1.f, false, 1.f);
	}
}

#pragma optimize("", on)

void FPersistentPaintablesModule::StartupModule() {

	wallSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWall/Build_PipelineSupportWall.Build_PipelineSupportWall_C"));
	wallHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWallHole/Build_PipelineSupportWallHole.Build_PipelineSupportWallHole_C"));
	floorHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundationPassthrough/Build_FoundationPassthrough_Pipe.Build_FoundationPassthrough_Pipe_C"));
	swatchClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/-Shared/Customization/Swatches/SwatchDesc_Custom.SwatchDesc_Custom_C"));

#if !WITH_EDITOR
	//AFGBuildablePipeline* BuildablePipeline = GetMutableDefault<AFGBuildablePipeline>();
	//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::UpdateFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
	//	{
	//		if (self)
	//		{
	//			this->UpdateColor(self, descriptor);
	//			//this->StartTimer(self, descriptor);
	//		}
	//	});
		//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::OnFullRebuildCompleted, &UpdateColor)
#endif
}


IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);