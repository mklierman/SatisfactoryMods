#include "PersistentPaintablesModule.h"

#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildablePoleStackable.h"
#include "FGLightweightBuildableSubsystem.h"
#include "FGPipeSubsystem.h"
#include "Hologram/FGFoundationHologram.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PP_ActorComponent.h"
#include "Resources/FGItemDescriptor.h"
#include "SessionSettings/SessionSettingsManager.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Buildables/FGBuildable.h"
#include <Logging/StructuredLog.h>

#include <FGBuildablePolePipe.h>
#include <Buildables/FGBuildableFoundation.h>

DEFINE_LOG_CATEGORY(PersistentPaintables_Log);

// Local constants to avoid magic numbers
static constexpr float SUPPORT_DIST = 5.0f;                 // distance threshold used for "nearby" checks
static constexpr float SUPPORT_DIST_SQ = SUPPORT_DIST * SUPPORT_DIST; // squared threshold so we don't call sqrt
static constexpr float NEARBY_SUPPORT_SEARCH_RADIUS = 5.0f;

// Helper: apply color to a buildable
// Sets the swatch on the actor and calls the buildable's apply/set methods.
void FPersistentPaintablesModule::ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData)
{
	if (buildable && buildable->GetCanBeColored_Implementation())
	{
		// Mark this as a custom-painted buildable and push the data down to the actor
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

	UClass* seekClass = AActor::StaticClass();
	UKismetSystemLibrary::SphereOverlapActors(pipe->GetWorld(), connLoc, NEARBY_SUPPORT_SEARCH_RADIUS, traceObjectTypes, seekClass, ignoreActors, outActors);

	return outActors;
}

void FPersistentPaintablesModule::UpdateNetworkColor(AFGPipeNetwork* pipeNetwork)
{
	if (!pipeNetwork || pipeNetwork->mFluidIntegrants.Num() == 0)
	{
		return;
	}

	// Collect potential supports once per network update so we don't repeatedly call GetAllActorsOfClass
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

	WeakPotentialSupports.Empty();
	WeakPotentialSupports.Reserve(PotentialSupports.Num());
	for (auto& pot : PotentialSupports)
	{
		if (IsValid(pot))
		{
			WeakPotentialSupports.Add(pot);
		}
	}

	// Color every fluid integrant in this network
	for (auto& fi : pipeNetwork->mFluidIntegrants)
	{
		auto buildable = Cast<AFGBuildable>(fi);
		UpdateColorSingle(buildable, pipeNetwork);
	}
}

void FPersistentPaintablesModule::UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork)
{
	if (!pipeNetwork || !buildable)
	{
		return;
	}

	if (!buildable->GetCanBeColored_Implementation())
	{
		return;
	}

	auto desc = pipeNetwork->GetFluidDescriptor();
	if (!desc)
	{
		return;
	}

	auto pipe = Cast<AFGBuildablePipeline>(buildable);
	auto junction = Cast<AFGBuildablePipelineJunction>(buildable);
	auto attachment = Cast<AFGBuildablePipelineAttachment>(buildable);

	if (!(pipe || junction || attachment))
	{
		return;
	}

	auto fluidColor = UFGItemDescriptor::GetFluidColorLinear(pipeNetwork->GetFluidDescriptor());

			// Build the customization data to apply
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

	// Apply to the buildable itself first
	ApplyColor(buildable, swatchClass, newData);

	// If this is a pipeline actor, try to color connected supports as well
	if (pipe)
	{
		ColorSupports(pipe, newData);
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
	if (!optionValue)
	{
		return;
	}

	for (auto weakActor : WeakPotentialSupports)
	{
		if (!weakActor.IsValid())
		{
			continue;
		}

		auto actor = weakActor.Get();

		for (auto conn : pipe->mPipeConnections)
		{
			if (!IsValid(conn) || !IsValid(actor))
			{
				continue;
			}

			auto connLocation = conn->GetConnectorLocation();

			// Fast check against actor origin
			float actorDistSq = FVector::DistSquared(connLocation, actor->GetActorLocation());
			if (actorDistSq <= SUPPORT_DIST_SQ)
			{
				auto buildable = Cast<AFGBuildable>(actor);
				if (buildable)
				{
					ApplyColor(buildable, swatchClass, newData);
				}
				continue;
			}

			// Inspect actor components for pipe connection components and compare connector positions
			TArray<UActorComponent*> components;
			actor->GetComponents(components);
			for (auto* component : components)
			{
				if (!component || !IsValid(component))
				{
					continue;
				}

				auto pipeConnection = Cast<UFGPipeConnectionComponent>(component);
				if (!pipeConnection || !IsValid(pipeConnection))
				{
					continue;
				}

				float distSq = FVector::DistSquared(connLocation, pipeConnection->GetConnectorLocation());
				if (distSq <= SUPPORT_DIST_SQ)
				{
					auto buildable = Cast<AFGBuildable>(actor);
					if (buildable)
					{
						ApplyColor(buildable, swatchClass, newData);
					}
					break;
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
				else if (auto newPipe = Cast<AFGBuildablePipeline>(owner))
				{
					auto supports = FindNearbySupports(pipe, conn);
					for (auto& support : supports)
					{
						if (auto fgpipeSupport = Cast<AFGBuildable>(support))
						{
							if (Cast<AFGBuildablePoleStackable>(fgpipeSupport) || fgpipeSupport->GetClass() == pipeSupportClass || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
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
		else
		{
			auto supports = FindNearbySupports(pipe, conn);
			for (auto& support : supports)
			{
				if (auto fgpipeSupport = Cast<AFGBuildable>(support))
				{
					if (Cast<AFGBuildablePoleStackable>(fgpipeSupport) || fgpipeSupport->GetClass() == pipeSupportClass || fgpipeSupport->GetClass() == wallHoleClass || fgpipeSupport->GetClass() == floorHoleClass || fgpipeSupport->GetClass() == wallSupportClass)
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
												auto connLocation = conn->GetConnectorLocation();
												auto supportLocation = pipeConnection->GetConnectorLocation();
												float distSq = FVector::DistSquared(connLocation, supportLocation);
												if (distSq <= SUPPORT_DIST_SQ)
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

	AFGPipeNetwork* pn = GetMutableDefault<AFGPipeNetwork>();

	// When a pipe network starts we want to color it once if the option is enabled.
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipeNetwork::BeginPlay, pn, [this](AFGPipeNetwork* self)
		{
			USessionSettingsManager* SessionSettings = self->GetWorld()->GetSubsystem<USessionSettingsManager>();
			auto optionValue = SessionSettings->GetBoolOptionValue("PersistentPaintables.AutoPaintPipes");
			if (optionValue)
			{
				AsyncTask(ENamedThreads::GameThread, [=, this]() {
					this->UpdateNetworkColor(self);
					});
			}
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

IMPLEMENT_GAME_MODULE(FPersistentPaintablesModule, PersistentPaintables);