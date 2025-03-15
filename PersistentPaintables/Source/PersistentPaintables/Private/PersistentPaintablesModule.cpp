#include "PersistentPaintablesModule.h"
#include "SessionSettings/SessionSettingsManager.h"
#include <Buildables/FGBuildableFoundation.h>
#include "FGLightweightBuildableSubsystem.h"
#include <Logging/StructuredLog.h>
#include "Hologram/FGFoundationHologram.h"
#include <PP_ActorComponent.h>
#include <Kismet/GameplayStatics.h>


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

void FPersistentPaintablesModule::UpdateNetworkColor(AFGPipeNetwork* pipeNetwork)
{
	if (pipeNetwork && pipeNetwork->mFluidIntegrants.Num() > 0)
	{

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), wallHoleClass, FoundActors);
		PotentialSupports = FoundActors;
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), floorHoleClass, FoundActors);
		PotentialSupports.Append(FoundActors);
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), wallSupportClass, FoundActors);
		PotentialSupports.Append(FoundActors);
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), AFGBuildablePipelineSupport::StaticClass(), FoundActors);
		PotentialSupports.Append(FoundActors);

		for (auto fi : pipeNetwork->mFluidIntegrants)
		{
			auto buildable = Cast< AFGBuildable>(fi);
			UpdateColorSingle(buildable, pipeNetwork);
		}
	}
}

void FPersistentPaintablesModule::UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork)
{
	auto desc = pipeNetwork->GetFluidDescriptor();
	if (buildable && pipeNetwork && pipeNetwork->mFluidForm != EResourceForm::RF_INVALID)
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

		USessionSettingsManager* SessionSettings = buildable->GetWorld()->GetSubsystem<USessionSettingsManager>();
		auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipesMetallic");
		if (optionValue)
		{
			UClass* SomeClass = LoadObject<UClass>(nullptr, TEXT("/PersistentPaintables/PersistentPaintables_CustomFinish.PersistentPaintables_CustomFinish_C"));
			newData.OverrideColorData.PaintFinish = SomeClass;
		}

		ApplyColor(buildable, swatchClass, newData);
		//return;

		if (auto pipe = Cast<AFGBuildablePipeline>(buildable))
		{

			ColorConnectedSupports(pipe, newData);

			//ColorSupports(pipe, newData);
		}
	}
}

void FPersistentPaintablesModule::ColorConnectedSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData)
{

	USessionSettingsManager* SessionSettings = pipe->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintSupports");
	if (optionValue)
	{
		for (auto actor : PotentialSupports)
		{
			for (auto conn : pipe->mPipeConnections)
			{
				auto connLocation = conn->GetConnectorLocation();
				auto actorLocation = actor->GetActorLocation();
				bool isNearActor = FVector::PointsAreNear(connLocation, actorLocation, 5);
				if (isNearActor)
				{
					auto buildable = Cast<AFGBuildable>(actor);
					ApplyColor(buildable, swatchClass, newData);
				}
				else
				{
					auto components = actor->GetComponents();
					for (auto component : components)
					{
						auto pipeConnection = Cast< UFGPipeConnectionComponent>(component);
						if (pipeConnection)
						{
							//Make sure it is actually near the connection
							auto supportLocation = pipeConnection->GetConnectorLocation();
							bool isNear = FVector::PointsAreNear(connLocation, supportLocation, 5);
							auto dist = FVector::Distance(connLocation, supportLocation);
							if (isNear)
							{
								auto buildable = Cast<AFGBuildable>(actor);
								ApplyColor(buildable, swatchClass, newData);
							}
						}
					}
				}
			}
		}
	}
}

void FPersistentPaintablesModule::ColorSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData)
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
								for (auto instance : aim->InstanceMap)
								{
									if (instance.Key.ToString().StartsWith("SM_PipelineSupport"))
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
		else //Connection isn't connected to anything. May be connected to a support
		{
			auto supports = FindNearbySupports(pipe, conn);
			for (auto support : supports)
			{
				if (auto fgpipeSupport = Cast<AFGBuildable>(support))
				{
					if (auto pipeSupport = Cast<AFGBuildablePipelineSupport>(fgpipeSupport) || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
					{
						ApplyColor(fgpipeSupport, swatchClass, newData);
					}
				}
				else if (auto aim = Cast<AAbstractInstanceManager>(support))
				{
					for (auto instance : aim->InstanceMap)
					{
						if (instance.Key.ToString().StartsWith("SM_PipelineSupport") || instance.Key.ToString().StartsWith("SM_PipePoleMulti"))
						{
							for (auto ihandle : instance.Value.InstanceHandles)
							{
								if (auto wallSupport = Cast<AFGBuildable>(ihandle->GetOwner()))
								{
									auto components = wallSupport->GetComponents();
									for (auto component : components)
									{
										if (auto pipeConnection = Cast<UFGPipeConnectionComponent>(component))
										{
											//Make sure it is actually near the connection
											auto connLocation = conn->GetConnectorLocation();
											auto supportLocation = pipeConnection->GetConnectorLocation();
											bool isNear = FVector::PointsAreNear(connLocation, supportLocation, 5);
											auto dist = FVector::Distance(connLocation, supportLocation);
											if (isNear)
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


void FPersistentPaintablesModule::StartupModule() {

	wallSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWall/Build_PipelineSupportWall.Build_PipelineSupportWall_C"));
	wallHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWallHole/Build_PipelineSupportWallHole.Build_PipelineSupportWallHole_C"));
	floorHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundationPassthrough/Build_FoundationPassthrough_Pipe.Build_FoundationPassthrough_Pipe_C"));
	swatchClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/-Shared/Customization/Swatches/SwatchDesc_Custom.SwatchDesc_Custom_C"));

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
			}
		});

	// This is needed for Painted Beams
	ConstructHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, hg, [=](auto& returnActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID constructionID)
		{
			AFGBuildable* buildable = Cast< AFGBuildable>(returnActor);
			if (buildable)
			{
				AddBuildable(buildable, self);
			}
		});

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
						buildable->SetCustomizationData_Implementation(component->CustomizationStruct);
						buildable->ApplyCustomizationData_Implementation(component->CustomizationStruct);
						buildable->SetCustomizationDataLightweightNoApply(component->CustomizationStruct);
						if (auto pipe = Cast<AFGBuildablePipeline>(buildable))
						{
							TArray<AActor*> FoundActors;
							UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), wallHoleClass, FoundActors);
							PotentialSupports = FoundActors;
							UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), floorHoleClass, FoundActors);
							PotentialSupports.Append(FoundActors);
							UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), wallSupportClass, FoundActors);
							PotentialSupports.Append(FoundActors);
							UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), AFGBuildablePipelineSupport::StaticClass(), FoundActors);
							PotentialSupports.Append(FoundActors);


							ColorConnectedSupports(pipe, component->CustomizationStruct);
							//ColorSupports(pipe, component->CustomizationStruct);
						}
						return;
					}
				}
			}
			return;
		}
	}
}

void FPersistentPaintablesModule::HookPipes()
{
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
		});

	AFGPipeNetwork* pn = GetMutableDefault<AFGPipeNetwork>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipeNetwork::BeginPlay, pn, [=](AFGPipeNetwork* self)
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
								}
							}
						}
					}
				}
			}
		});
#endif
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);