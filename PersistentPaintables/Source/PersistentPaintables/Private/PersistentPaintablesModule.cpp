#include "PersistentPaintablesModule.h"
#include "SessionSettings/SessionSettingsManager.h"
#include <Buildables/FGBuildableFoundation.h>
#include "FGLightweightBuildableSubsystem.h"
#include <Logging/StructuredLog.h>
#include "Hologram/FGFoundationHologram.h"
#include <PP_ActorComponent.h>
#include <Kismet/GameplayStatics.h>
#include "FGPipeSubsystem.h"
#include "Buildables/FGBuildablePoleStackable.h"
#include <FGBuildablePolePipe.h>
#include "Buildables/FGBuildable.h"


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
	outActors.Reserve(8); // small expected number
	TArray<UPrimitiveComponent*> outComponents;
	TArray<FHitResult> outHits;

	float radius = 5.0f;

	UClass* seekClass = AActor::StaticClass(); // NULL;
	UKismetSystemLibrary::SphereOverlapActors(pipe->GetWorld(), connLoc, radius, traceObjectTypes, seekClass, ignoreActors, outActors);

	return outActors;
}

void FPersistentPaintablesModule::AFGBuildableHologramConstruct(AFGBuildableHologram* self)
{
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
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), pipeSupportClass, FoundActors);
		PotentialSupports.Append(FoundActors);
		UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), AFGBuildablePoleStackable::StaticClass(), FoundActors);
		PotentialSupports.Append(FoundActors);

		USessionSettingsManager* SessionSettings = pipeNetwork->GetWorld()->GetSubsystem<USessionSettingsManager>();
		auto PaintAttachments = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintAttachments");
		if (PaintAttachments)
		{
			UGameplayStatics::GetAllActorsOfClass(pipeNetwork->GetWorld(), AFGBuildablePipelineAttachment::StaticClass(), FoundActors);
			PotentialSupports.Append(FoundActors);
		}

		// Avoid repeated reallocations when copying to weak list
		WeakPotentialSupports.Empty();
		WeakPotentialSupports.Reserve(PotentialSupports.Num());
		for (auto& pot : PotentialSupports)
		{
			if (IsValid(pot))
			{
				WeakPotentialSupports.Add(pot);
			}
		}

		for (auto& fi : pipeNetwork->mFluidIntegrants)
		{
			auto buildable = Cast< AFGBuildable>(fi);
			UpdateColorSingle(buildable, pipeNetwork);
		}
	}
}

void FPersistentPaintablesModule::UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork)
{
	if (pipeNetwork)
	{
		auto desc = pipeNetwork->GetFluidDescriptor();
		if (desc != nullptr && buildable)
		{
			// Cheap early-out: if buildable cannot be colored, skip everything
			if (!buildable->GetCanBeColored_Implementation())
			{
				return;
			}

			auto pipe = Cast<AFGBuildablePipeline>(buildable);
			auto junction = Cast<AFGBuildablePipelineJunction>(buildable);
			auto attachment = Cast < AFGBuildablePipelineAttachment>(buildable);

			if (pipe || junction || attachment)
			{
				auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(pipeNetwork->GetFluidDescriptor());

				if (buildable->mDefaultSwatchCustomizationOverride == swatchClass &&
					buildable->mCustomizationData.OverrideColorData.PrimaryColor.Equals(fluidColor, 0.001f))
				{
					return;
				}

				FFactoryCustomizationData newData;
				newData.SwatchDesc = swatchClass;
				newData.ColorSlot = 255;
				newData.OverrideColorData.PrimaryColor = fluidColor;
				newData.OverrideColorData.SecondaryColor = fluidColor;
				newData.Data = buildable->mCustomizationData.Data;
				newData.ExtraData = buildable->mCustomizationData.ExtraData;
				newData.NeedsSkinUpdate = true;

				UWorld* World = buildable->GetWorld();
				USessionSettingsManager* SessionSettings = World->GetSubsystem<USessionSettingsManager>();
				auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipesMetallic");
				if (optionValue)
				{
					static UClass* CachedPaintFinishClass = nullptr;
					if (!CachedPaintFinishClass)
					{
						CachedPaintFinishClass = LoadObject<UClass>(nullptr, TEXT("/PersistentPaintables/PersistentPaintables_CustomFinish.PersistentPaintables_CustomFinish_C"));
					}
					if (CachedPaintFinishClass)
					{
						newData.OverrideColorData.PaintFinish = CachedPaintFinishClass;
					}
				}

				ApplyColor(buildable, swatchClass, newData);
				if (pipe)
				{
					// Use the original support-coloring logic (cheaper than world-overlap queries)
					ColorSupports(pipe, newData);
				}
			}
		}
	}
}

void FPersistentPaintablesModule::ColorConnectedSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData)
{
	if (!pipe)
	{
		return;
	}
	USessionSettingsManager* SessionSettings = pipe->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintSupports");
	if (optionValue)
	{
		for (auto weakActor : WeakPotentialSupports)
		{
			if (weakActor.IsValid())
			{
				auto actor = weakActor.Get();
				for (auto conn : pipe->mPipeConnections)
				{
					if (IsValid(conn) && IsValid(actor))
					{
						auto connLocation = conn->GetConnectorLocation();
						if (!actor)
						{
							break;
						}
						auto actorLocation = actor->GetActorLocation();
						bool isNearActor = FVector::PointsAreNear(connLocation, actorLocation, 5);
						if (isNearActor)
						{
							auto buildable = Cast<AFGBuildable>(actor);
							if (buildable)
							{
								ApplyColor(buildable, swatchClass, newData);
							}
						}
						else
						{
							TSet<UActorComponent*> components = actor->GetComponents();
							TSet<UActorComponent*> componentsCopy(components);
							if (!componentsCopy.IsEmpty())
							{
								for (auto component : componentsCopy)
								{
									if (component && IsValid(component))
									{
										auto pipeConnection = Cast< UFGPipeConnectionComponent>(component);
										if (pipeConnection && IsValid(pipeConnection))
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
			}
		}
	}
}

void FPersistentPaintablesModule::ColorSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData)
{
	for (auto& conn : pipe->mPipeConnections)
	{
		if (conn->mConnectedComponent)
		{
			if (auto owner = conn->mConnectedComponent->GetOwner())
			{
				if (auto junction = Cast<AFGBuildablePipelineJunction>(owner))
				{
					ApplyColor(junction, swatchClass, newData);
				}
				else if (auto pipeSupport = Cast<AFGBuildablePoleStackable>(owner))
				{
					ApplyColor(pipeSupport, swatchClass, newData);
				}
				else if (auto pipeSupport = Cast<AFGBuildablePolePipe>(owner))
				{
					ApplyColor(pipeSupport, swatchClass, newData);
				}
				else if (auto newPipe = Cast<AFGBuildablePipeline>(owner)) // Might be connected to a pipe through a support
				{
					auto supports = FindNearbySupports(pipe, conn);
					if (supports.Num() > 0)
					{
						for (auto& support : supports)
						{
							if (auto fgpipeSupport = Cast<AFGBuildable>(support))
							{
								if (auto pipeSupport2 = Cast<AFGBuildablePoleStackable>(fgpipeSupport) || fgpipeSupport->GetClass() == pipeSupportClass || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
								{
									ApplyColor(fgpipeSupport, swatchClass, newData);
								}
							}
							else if (auto aim = Cast<AAbstractInstanceManager>(support))
							{
								for (auto& instance : aim->InstanceMap)
								{
									FString keyStr = instance.Key.ToString();
									if (keyStr.StartsWith("SM_PipelineSupport"))
									{
										for (auto& ihandle : instance.Value.InstanceHandles)
										{
											for (auto& handle : ihandle.Value)
											{
												if (auto wallSupport = Cast<AFGBuildable>(handle->GetOwner()))
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
		else //Connection isn't connected to anything. May be connected to a support
		{
			auto supports = FindNearbySupports(pipe, conn);
			for (auto& support : supports)
			{
				if (auto fgpipeSupport = Cast<AFGBuildable>(support))
				{
					if (auto pipeSupport = Cast<AFGBuildablePoleStackable>(fgpipeSupport) || fgpipeSupport->GetClass() == pipeSupportClass || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
					{
						ApplyColor(fgpipeSupport, swatchClass, newData);
					}
				}
				else if (auto aim = Cast<AAbstractInstanceManager>(support))
				{
					for (auto& instance : aim->InstanceMap)
					{
						FString keyStr = instance.Key.ToString();
						if (keyStr.StartsWith("SM_PipelineSupport") || keyStr.StartsWith("SM_PipePoleMulti"))
						{
							for (auto& ihandle : instance.Value.InstanceHandles)
							{
								for (auto& handle : ihandle.Value)
								{
									if (auto wallSupport = Cast<AFGBuildable>(handle->GetOwner()))
									{
										auto components = wallSupport->GetComponents();
										for (auto* component : components)
										{
											if (auto pipeConnection = Cast<UFGPipeConnectionComponent>(component))
											{
												//Make sure it is actually near the connection
												auto connLocation = conn->GetConnectorLocation();
												auto supportLocation = pipeConnection->GetConnectorLocation();
												// Use squared distance to avoid sqrt
												float distSq = FVector::DistSquared(connLocation, supportLocation);
												if (distSq <= 25.0f) // 5 units squared
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


void FPersistentPaintablesModule::StartupModule() {

	wallSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWall/Build_PipelineSupportWall.Build_PipelineSupportWall_C"));
	wallHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupportWallHole/Build_PipelineSupportWallHole.Build_PipelineSupportWallHole_C"));
	floorHoleClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundationPassthrough/Build_FoundationPassthrough_Pipe.Build_FoundationPassthrough_Pipe_C"));
	pipeSupportClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/PipelineSupport/Build_PipelineSupport.Build_PipelineSupport_C"));
	swatchClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/-Shared/Customization/Swatches/SwatchDesc_Custom.SwatchDesc_Custom_C"));

#if !WITH_EDITOR
	AFGBuildableHologram* hg = GetMutableDefault<AFGBuildableHologram>();
	AFGFoundationHologram* fhg = GetMutableDefault<AFGFoundationHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, hg, [this](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			//UE_LOG(PersistentPaintables_Log, Display, TEXT("ConfigureActor"));
			if (self)
			{
				USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
				auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
				if (optionValue)
				{
					AddBuildable(inBuildable, self);
				}
				return;
			}
		});

	// This is needed for Painted Beams
	ConstructHook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, hg, [this](auto& returnActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID constructionID)
		{
			AFGBuildable* buildable = Cast< AFGBuildable>(returnActor);
			if (buildable)
			{
				USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
				auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
				if (optionValue)
				{
					AddBuildable(buildable, self);
				}
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
	if (buildable)
	{
		if (auto pipe = Cast<AFGBuildablePipeline>(buildable))
		{
			USessionSettingsManager* SessionSettings = buildable->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			if (optionValue)
			{
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), wallHoleClass, FoundActors);
				PotentialSupports = FoundActors;
				UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), floorHoleClass, FoundActors);
				PotentialSupports.Append(FoundActors);
				UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), wallSupportClass, FoundActors);
				PotentialSupports.Append(FoundActors);
				UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), pipeSupportClass, FoundActors);
				PotentialSupports.Append(FoundActors);
				UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), AFGBuildablePoleStackable::StaticClass(), FoundActors);
				PotentialSupports.Append(FoundActors);

				auto PaintAttachments = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintAttachments");
				if (PaintAttachments)
				{
					UGameplayStatics::GetAllActorsOfClass(pipe->GetWorld(), AFGBuildablePipelineAttachment::StaticClass(), FoundActors);
					PotentialSupports.Append(FoundActors);
				}

				WeakPotentialSupports.Empty();
				for (auto pot : PotentialSupports)
				{
					if (IsValid(pot))
					{
						WeakPotentialSupports.Add(pot);
					}
				}

				auto connComp = Cast<UFGPipeConnectionComponent>(pipe->GetConnection0());
				auto pipeNetworkId = connComp->GetPipeNetworkID();
				auto pipeSubsystem = AFGPipeSubsystem::Get(buildable->GetWorld());
				auto network = pipeSubsystem->FindPipeNetwork(pipeNetworkId);
				UpdateColorSingle(pipe, network);
			}
		}
		else if (auto attachment = Cast<AFGBuildablePipelineAttachment>(buildable))
		{
			USessionSettingsManager* SessionSettings = buildable->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			auto PaintAttachments = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintAttachments");
			if (optionValue && PaintAttachments)
			{
				
				auto connComps = attachment->GetPipeConnections();
				if (connComps.Num() > 0)
				{
					auto pipeNetworkId = connComps[0]->GetPipeNetworkID();
					auto pipeSubsystem = AFGPipeSubsystem::Get(buildable->GetWorld());
					auto network = pipeSubsystem->FindPipeNetwork(pipeNetworkId);
					UpdateColorSingle(attachment, network);
				}
			}
		}
	}
}

void FPersistentPaintablesModule::HookPipes()
{
#if !WITH_EDITOR
	//UFGPipeConnectionComponent::OnFluidDescriptorUpdated_GameThread
	SUBSCRIBE_METHOD_AFTER(UFGPipeConnectionComponent::OnFluidDescriptorUpdated_GameThread, [this](UFGPipeConnectionComponent* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			auto paintNetwork = SessionSettings->GetBoolOptionValue("PersistentPaintables.PaintNetwork");
			if (optionValue && paintNetwork)
			{
				auto buildable = Cast<AFGBuildable>(self->GetOwner());
				if (buildable)
				{
					auto pipeNetworkId = self->GetPipeNetworkID();
					auto pipeSubsystem = AFGPipeSubsystem::Get(self->GetWorld());
					if (pipeSubsystem)
					{
						auto network = pipeSubsystem->FindPipeNetwork(pipeNetworkId);
						if (network)
						{
							AsyncTask(ENamedThreads::GameThread, [=, this]() {
								UpdateColorSingle(buildable, network);
								});
						}
					}
				}
			}
		});
	UFGPipeConnectionComponent* pcb = GetMutableDefault<UFGPipeConnectionComponent>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGPipeConnectionComponent::BeginPlay, pcb, [this](UFGPipeConnectionComponent* self)
		{
			/*if (self)
			{
				USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
				auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
				if (optionValue)
				{
					auto PipeConnComp = Cast<UFGPipeConnectionComponent>(self);
					if (PipeConnComp)
					{
						auto pipeNetworkId = PipeConnComp->GetPipeNetworkID();
						auto pipeSubsystem = AFGPipeSubsystem::Get(self->GetWorld());
						if (pipeSubsystem)
						{
							auto network = pipeSubsystem->FindPipeNetwork(pipeNetworkId);
							if (network)
							{
								AsyncTask(ENamedThreads::GameThread, [=, this]() {
									this->UpdateNetworkColor(network);
									});
							}
						}
					}
				}
			}*/
		});
	//TrySetFluidDescriptor(TSubclassOf< UFGItemDescriptor > newItemDescriptor);
	//SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::TrySetFluidDescriptor, [this](AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > newItemDescriptor)
	//	{
	//		USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
	//		auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
	//		if (optionValue)
	//		{
	//			AsyncTask(ENamedThreads::GameThread, [=, this]() {
	//				this->UpdateNetworkColor(self);
	//				});
	//		}
	//	});

	AFGPipeNetwork* pn = GetMutableDefault<AFGPipeNetwork>();

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipeNetwork::BeginPlay, pn, [this](AFGPipeNetwork* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			if (optionValue)
			{
				//UE_LOG(PersistentPaintables_Log, Display, TEXT("AFGPipeNetwork::UpdateFluidDescriptor"));
				AsyncTask(ENamedThreads::GameThread, [=, this]() {
					this->UpdateNetworkColor(self);
					});
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGPipeNetwork::AddFluidIntegrant, [this](AFGPipeNetwork* self, class IFGFluidIntegrantInterface* fluidIntegrant)
		{
			//if (self)
			//{
			//	USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			//	auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			//	//FPersistentPaintables_ConfigStruct config = FPersistentPaintables_ConfigStruct::GetActiveConfig(self->GetWorld());
			//	if (optionValue)
			//	{
			//		//auto startTime = FDateTime::Now();
			//		auto pipeBase = Cast<UFGPipeConnectionComponentBase>(fluidIntegrant);
			//		if (pipeBase && pipeBase->GetPipeConnectionType() != EPipeConnectionType::PCT_CONSUMER && pipeBase->GetPipeConnectionType() != EPipeConnectionType::PCT_PRODUCER)
			//		{
			//			auto pipeCons = fluidIntegrant->GetPipeConnections();
			//			if (pipeCons.Num() > 0)
			//			{
			//				if (auto owner = pipeCons[0]->GetOwner())
			//				{
			//					if (auto buildable = Cast<AFGBuildablePipeline>(owner))
			//					{

			//						AsyncTask(ENamedThreads::GameThread, [=, this]() {
			//							UpdateColorSingle(buildable, self);
			//							});
			//					}
			//				}
			//			}
			//		}

			//		//auto endTime = FDateTime::Now();
			//		//auto elapsedTime = endTime - startTime;
			//		//UE_LOGFMT(PersistentPaintables_Log, Display, "AFGPipeNetwork::AddFluidIntegrant time elapsed: {0}", elapsedTime);
			//	}
			//}
		});

	AFGBuildableHologram* hg = GetMutableDefault<AFGBuildableHologram>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::Construct, hg, [this](auto& scope, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID constructionID)
		{
			AActor* upgraded = self->GetUpgradedActor();
			AFGBuildable* buildable = Cast<AFGBuildable>(upgraded);
			if (buildable)
			{
				auto customization = buildable->GetCustomizationData_Implementation();
				AActor* constructedActor = scope(self, out_children, constructionID);
				AFGBuildable* constructedBuildable = Cast<AFGBuildable>(constructedActor);
				if (constructedBuildable)
				{
					constructedBuildable->ApplyCustomizationData_Implementation(customization);
					constructedBuildable->SetCustomizationData_Implementation(customization);
				}
			}
		});
#endif
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);