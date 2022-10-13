#include "PersistentPaintablesModule.h"
#include "Buildables/FGBuildable.h"
#include "Resources/FGItemDescriptor.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildablePipeline.h"
#include "PersistentPaintablesCppSubSystem.h"
#include "FGFluidIntegrantInterface.h"
#include "Buildables/FGBuildablePipelineJunction.h"
#include "FGBuildablePipelineSupport.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGPipeConnectionComponent.h"
#include "Kismet\KismetSystemLibrary.h"
#include "FGPipeNetwork.h"

DEFINE_LOG_CATEGORY(PersistentPaintables_Log);
#pragma optimize("", off)
void ApplyColor(AFGBuildable* buildable, UClass* swatchClass, FFactoryCustomizationData customizationData)
{
	if (buildable && buildable->Execute_GetCanBeColored(buildable))
	{
		buildable->mColorSlot = 255;
		buildable->mDefaultSwatchCustomizationOverride = swatchClass;
		buildable->Execute_ApplyCustomizationData(buildable, customizationData);
	}
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

	float radius = 5.0f;

	UClass* seekClass = AFGBuildable::StaticClass(); // NULL;
	UKismetSystemLibrary::SphereOverlapActors(pipe->GetWorld(), connLoc, radius, traceObjectTypes, seekClass, ignoreActors, outActors);

	return outActors;
}


void FPersistentPaintablesModule::UpdateColor(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
{
	UE_LOG(PersistentPaintables_Log, Display, TEXT("UpdateFluidDescriptor"));
	if (self && self->mFluidIntegrants.Num() > 0)
	{
		for (auto fi : self->mFluidIntegrants)
		{
			auto buildable = Cast< AFGBuildable>(fi);
			if (buildable)
			{
				auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(descriptor);
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
									//else if (auto newPipe = Cast<AFGBuildablePipeline>(owner)) // Might be connected to a pipe through a support
									//{
									//	auto supports = FindNearbySupports(pipe, conn);
									//	if (supports.Num() > 0)
									//	{
									//		for (auto support : supports)
									//		{
									//			if (auto fgpipeSupport = Cast<AFGBuildable>(support))
									//			{
									//				if (auto pipeSupport2 = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
									//				{
									//					ApplyColor(fgpipeSupport, swatchClass, newData);
									//				}
									//			}
									//		}
									//	}
									//}
								}
							}
							//else //Connection isn't connected to anything. May be connected to a support
							//{
							//	auto supports = FindNearbySupports(pipe, conn);
							//	if (supports.Num() > 0)
							//	{
							//		for (auto support : supports)
							//		{
							//			if (auto fgpipeSupport = Cast<AFGBuildable>(support))
							//			{
							//				if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
							//				{
							//					ApplyColor(fgpipeSupport, swatchClass, newData);
							//				}
							//			}
							//		}
							//	}
							//}
						}
					}
					//if (pipe->mPipeConnections.Num() > 0)
					//{
					//	for (auto conn : pipe->mPipeConnections)
					//	{
					//		if (conn->mConnectedComponent)
					//		{
					//			auto owner = conn->mConnectedComponent->GetOwner();
					//			if (owner)
					//			{
					//				if (auto junction = Cast<AFGBuildablePipelineJunction>(owner))
					//				{
					//					junction->mColorSlot = 255;
					//					junction->mDefaultSwatchCustomizationOverride = swatchClass;
					//					junction->SetCustomizationData_Implementation(newData);
					//					junction->ApplyCustomizationData_Implementation(newData);
					//					junction->SetCustomizationData_Implementation(newData);
					//				}
					//				else if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(owner))
					//				{
					//					pipeSupport->mColorSlot = 255;
					//					pipeSupport->mDefaultSwatchCustomizationOverride = swatchClass;
					//					pipeSupport->SetCustomizationData_Implementation(newData);
					//					pipeSupport->ApplyCustomizationData_Implementation(newData);
					//					pipeSupport->SetCustomizationData_Implementation(newData);
					//				}
					//				else if (auto newPipe = Cast<AFGBuildablePipeline>(owner))
					//				{

					//				}
					//			}
					//		}
					//		else
					//		{
					//			FVector connLoc = conn->Bounds.Origin;
					//			// Set what actors to seek out from it's collision channel
					//			TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
					//			traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
					//			traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

					//			// Ignore any specific actors
					//			TArray<AActor*> ignoreActors;
					//			// Ignore self or remove this line to not ignore any
					//			ignoreActors.Init(buildable, 1);

					//			// Array of actors that are inside the radius of the sphere
					//			TArray<AActor*> outActors;
					//			TArray<UPrimitiveComponent*> outComponents;

					//			// Total radius of the sphere
					//			float radius = 5.0f;
					//			// Sphere's spawn loccation within the world
					//			FVector sphereSpawnLocation = connLoc;


					//			UClass* seekClass = AFGBuildable::StaticClass(); // NULL;
					//			UKismetSystemLibrary::SphereOverlapActors(buildable->GetWorld(), sphereSpawnLocation, radius, traceObjectTypes, seekClass, ignoreActors, outActors);

					//			//// Finally iterate over the outActor array
					//			if (outActors.Num() > 0)
					//			{
					//				for (AActor* overlappedActor : outActors) {
					//					auto fgpipeSupport = Cast<AFGBuildable>(overlappedActor);
					//					if (fgpipeSupport)
					//					{
					//						if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(overlappedActor) || overlappedActor->GetClass() == wallHoleClass || overlappedActor->GetClass() == floorHoleClass || overlappedActor->GetClass() == wallSupportClass)
					//						{
					//							fgpipeSupport->mColorSlot = 255;
					//							fgpipeSupport->mDefaultSwatchCustomizationOverride = swatchClass;
					//							
					//							fgpipeSupport->Execute_ApplyCustomizationData(fgpipeSupport, newData);
					//							//fgpipeSupport->SetCustomizationData_Native(newData);
					//							//fgpipeSupport->ApplyCustomizationData_Native(newData);
					//						}
					//					}
					//				}
					//			}
								//if (outComponents.Num() > 0)
								//{
								//	for (auto comp : outComponents)
								//	{
								//		auto pipeComp = Cast< UFGPipeConnectionComponent>(comp);
								//		if (pipeComp && pipeComp->GetPipeConnectionType() == EPipeConnectionType::PCT_SNAP_ONLY)
								//		{
								//			auto owner = pipeComp->GetOwner();
								//			if (owner)
								//			{
								//				auto pipeBuildable = Cast<AFGBuildable>(owner);
								//				if (pipeBuildable)
								//				{
								//					pipeBuildable->mColorSlot = 255;
								//					pipeBuildable->mDefaultSwatchCustomizationOverride = swatchClass;
								//					pipeBuildable->SetCustomizationData_Implementation(newData);
								//					pipeBuildable->ApplyCustomizationData_Implementation(newData);
								//					pipeBuildable->SetCustomizationData_Implementation(newData);
								//				}
								//			}
								//		}
								//	}
								//}
			//				}
			//			}
			//		}

			//	}
			//}
				}
			}
		}
	}
}


void FPersistentPaintablesModule::StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
{
	FTimerHandle FluidColorTimerHandle;
	if (self)
	{
		self->GetWorld()->GetTimerManager().SetTimer(FluidColorTimerHandle, [&]() { this->UpdateColor(self, descriptor); }, 2.f, false, 2.f);
	}
}

#pragma optimize("", on)

void FPersistentPaintablesModule::StartupModule() {

	wallSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWall/Build_PipelineSupportWall.Build_PipelineSupportWall_C"));
	wallHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWallHole/Build_PipelineSupportWallHole.Build_PipelineSupportWallHole_C"));
	floorHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundationPassthrough/Build_FoundationPassthrough_Pipe.Build_FoundationPassthrough_Pipe_C"));
	swatchClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/-Shared/Customization/Swatches/SwatchDesc_Custom.SwatchDesc_Custom_C"));

#if !WITH_EDITOR
	AFGBuildablePipeline* BuildablePipeline = GetMutableDefault<AFGBuildablePipeline>();
	SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::UpdateFluidDescriptor, [=](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor)
		{
			if (self)
			{
				this->StartTimer(self, descriptor);
			}
		});
		//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::OnFullRebuildCompleted, &UpdateColor)
#endif
}


IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);