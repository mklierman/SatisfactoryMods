#include "DirectToSplitterModule.h"
#include "DTS_ConfigStruct.h"
#include "Registry/ModContentRegistry.h"
#include "Logging/StructuredLog.h"
#include <Kismet/KismetMathLibrary.h>
#include <Buildables/FGBuildableStorage.h>
#include <Buildables/FGBuildableTrainPlatform.h>
#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "FGFactoryConnectionComponent.h"
#include <Buildables/FGBuildableDockingStation.h>
#include <Kismet/GameplayStatics.h>
#include "Equipment/FGBuildGunDismantle.h"
#include <Hologram/FGSplineHologram.h>
#include <Hologram/FGConveyorPoleHologram.h>
#include "FGInputLibrary.h"
//#include <Hologram/FGPipelineSupportHologram.h>
#include <Hologram/FGWallAttachmentHologram.h>
#include <Hologram/FGWireHologram.h>

DEFINE_LOG_CATEGORY(SnapOn_Log);
#pragma optimize("", off)
//#endif
void FDirectToSplitterModule::StartupModule() {
	AFGConveyorAttachmentHologram* cah = GetMutableDefault<AFGConveyorAttachmentHologram>();
	AFGFactoryHologram* fh = GetMutableDefault<AFGFactoryHologram>();
	AFGBuildable* fgb = GetMutableDefault<AFGBuildable>();
	AFGBuildableConveyorAttachment* bca = GetMutableDefault<AFGBuildableConveyorAttachment>();

#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::ClearConnection, [this](auto& scope, UFGFactoryConnectionComponent* self)
		{
			if (self)
			{
				auto conn = self->GetConnection();
				DismantleLeftoverBelt(conn);
			}
		});

	//SUBSCRIBE_METHOD(AFGBuildableConveyorBelt::Merge, [this](auto& scope, TArray< AFGBuildableConveyorBelt* > conveyors)
	//	{
	//		auto recipe1 = conveyors[0]->GetBuiltWithRecipe();
	//		auto recipe2 = conveyors[1]->GetBuiltWithRecipe();
	//		if (recipe1 && recipe1->GetName() == "SnapOn_Recipe_C")
	//		{
	//			scope.Override(nullptr);
	//		}
	//		else if (recipe2 && recipe2->GetName() == "SnapOn_Recipe_C")
	//		{
	//			scope.Override(nullptr);
	//		}
	//		scope.Override(nullptr);
	//	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::CheckValidFloor, cah, [this](auto& scope, AFGConveyorAttachmentHologram* self)
		{
			CheckValidFloor(self);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::BeginPlay, fgb, [this](auto& scope, AFGBuildable* self)
		{
			if (auto attachment = Cast<AFGBuildableConveyorAttachment>(self))
			{
				HandleExistingSnappedOn(attachment);
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::TrySnapToActor, cah, [this](auto& scope, AFGConveyorAttachmentHologram* self, const FHitResult& hitResult)
		{
			bool scopeResult = scope(self, hitResult);
			if (scopeResult)
			{
				return;
			}
			self->mSnappedConnection = nullptr;
			for (auto conn : self->mConnections)
			{
				conn->ClearConnection();
			}
			bool result = TrySnapToActor(self, hitResult);
			if (!result)
			{
				self->mSnappedConnection = nullptr;
				for (auto conn : self->mConnections)
				{
					conn->ClearConnection();
				}
				return;
			}
			scope.Override(result);
			bool retflag;
			if (result)
				CheckValidPlacement(self, retflag);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::ConfigureComponents, cah, [this](auto& scope, const AFGConveyorAttachmentHologram* self, AFGBuildable* inBuildable)
		{
			bool shouldCancel;
			ConfigureComponents(self, shouldCancel);
			if (shouldCancel) scope.Cancel();
		});


	AFGBuildableHologram* bhg = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, bhg, [this](auto& outActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID netConstructionID)
		{
			HGConstruct(self, outActor);

			if (auto attachment = Cast<AFGBuildable>(outActor))
			{
				HandleExistingSnappedOn(attachment);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::IsValidHitResult, bhg, [this](auto& scope, const AFGConveyorAttachmentHologram* self, const FHitResult& hitResult)
		{
			if (IsValidHitResult(self, hitResult))
			{
				scope.Override(true);
			}
			//scope.Override(true);
		});

	AFGPipeAttachmentHologram* pahg = GetMutableDefault<AFGPipeAttachmentHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::IsValidHitResult, pahg, [this](auto& scope, const AFGPipeAttachmentHologram* self, const FHitResult& hitResult)
		{
			if (auto junction = Cast<AFGPipelineJunctionHologram>(self))
			{
				AFGBuildable* hitBuildable = nullptr;
				auto hitInstanceMgr = Cast<AAbstractInstanceManager>(hitResult.GetActor());
				if (hitInstanceMgr)
				{
					FInstanceHandle outHandle;
					auto hitResolved = hitInstanceMgr->ResolveHit(hitResult, outHandle);
					if (hitResolved)
					{
						if (outHandle.GetOwner())
						{
							auto handleBuildable = Cast<AFGBuildable>(outHandle.GetOwner());
							if (handleBuildable)
							{
								hitBuildable = handleBuildable;
							}
						}
					}
				}
				if (!hitBuildable)
				{
					hitBuildable = Cast<AFGBuildable>(hitResult.GetActor());
				}
				if (hitBuildable)
				{
					auto components = hitBuildable->GetComponents();
					if (components.Num() > 0)
					{
						for (auto comp : components)
						{
							UFGPipeConnectionComponent* fccomp = Cast< UFGPipeConnectionComponent>(comp);
							if (fccomp)
							{
								if (fccomp->IsConnected())
								{
									continue;
								}
								auto compLoc = fccomp->GetComponentLocation();
								auto distance = FVector::Dist(hitResult.Location, compLoc);
								if (distance <= junction->mAttachmentPointSnapDistanceThreshold)
								{
									scope.Override(true);
								}
							}
						}
					}
				}
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::TrySnapToActor, pahg, [this](auto& scope, AFGPipeAttachmentHologram* self, const FHitResult& hitResult)
		{
			bool result = (bool)scope(self, hitResult);
			auto pipeAttachHolo = Cast<AFGPipeAttachmentHologram>(self);
			auto hg = Cast<AFGHologram>(self);
			auto contr = Cast<APlayerController>(hg->GetConstructionInstigator()->GetController());
			if ((!result || contr->IsInputKeyDown(EKeys::LeftShift)) && pipeAttachHolo && hitResult.GetActor())
			{
				scope.Override(PipeSnap(pipeAttachHolo, hitResult));

				return;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::CheckValidPlacement, pahg, [this](auto& scope, AFGPipeAttachmentHologram* self)
		{
			auto junction = Cast<AFGPipelineJunctionHologram>(self);
			if (junction && self->mSnappedConnectionComponent)
			{
				self->ResetConstructDisqualifiers();
				scope.Cancel();
			}
		});
//#if !WITH_EDITOR
#endif
}

void FDirectToSplitterModule::DismantleLeftoverBelt(UFGFactoryConnectionComponent* conn)
{
	if (conn && conn->GetConnector() == EFactoryConnectionConnector::FCC_CONVEYOR)
	{
		auto outer = conn->GetOuterBuildable();
		if (outer)
		{
			auto recipe = outer->GetBuiltWithRecipe();
			if (recipe && recipe->GetName() == "SnapOn_Recipe_C")
			{
				//conn->ClearConnection();
				outer->Dismantle_Implementation();
			}
		}
	}
}

void FDirectToSplitterModule::HandleLeftoverBelts(AFGBuildable* conveyorAttachment)
{

	auto& components = conveyorAttachment->GetComponents();
	for (auto component : components)
	{
		auto factoryConn = Cast < UFGFactoryConnectionComponent>(component);
		if (factoryConn && factoryConn->IsConnected())
		{
			auto conn = factoryConn->GetConnection();
			auto comp = conn->GetConnection();
			if (comp && comp->GetName() == "ConveyorAny0")
			{
				auto outer = comp->GetOuterBuildable();
				if (outer)
				{
					auto recipe = outer->GetBuiltWithRecipe();
					if (recipe && recipe->GetName() == "SnapOn_Recipe_C")
					{
						//conn->ClearConnection();
						outer->Dismantle_Implementation();
					}
				}
			}
		}
	}
	
}

void FDirectToSplitterModule::HandleExistingSnappedOn(AFGBuildable* conveyorAttachment)
{
	if (conveyorAttachment)
	{
		auto& components = conveyorAttachment->GetComponents();
		for (auto component : components)
		{
			auto factoryConn = Cast < UFGFactoryConnectionComponent>(component);
			if (factoryConn && IsSnappedOn(factoryConn))
			{
				auto otherConn = factoryConn->GetConnection();
				auto buildingName = otherConn->GetOuterBuildable()->GetName();
				factoryConn->ClearConnection();
				auto connLoc = factoryConn->GetComponentLocation();
				auto otherLoc = otherConn->GetComponentLocation();
				auto distance = FVector::Distance(connLoc, otherLoc);
				distance = FMath::RoundToDouble(distance);
				auto offset = FDTS_ConfigStruct::GetActiveConfig(conveyorAttachment->GetWorld()).SnapOffset * 100.f / 2;




				//Mk6 belt blueprint class
				UClass* beltClass = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Buildable/Factory/ConveyorBeltMk6/Build_ConveyorBeltMk6.Build_ConveyorBeltMk6_C"));
				auto Transform1 = conveyorAttachment->GetActorLocation();
				auto OtherLocation = otherConn->GetOwner()->GetActorLocation();
				auto Location = Transform1 - OtherLocation; // Has to be moved physically away so it doesn't get auto-merged
				FVector Midpoint = UKismetMathLibrary::VLerp(Transform1, OtherLocation, 0.5f);
				FTransform TF = conveyorAttachment->GetActorTransform();
				TF.SetLocation(Midpoint);

				AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(conveyorAttachment);
				auto beltActor = Subsystem->BeginSpawnBuildable(beltClass, TF);

				beltActor->FinishSpawning(TF);
				//auto beltActor = conveyorAttachment->GetWorld()->SpawnActor(beltClass, &Transform);
				auto belt = Cast<AFGBuildableConveyorBase>(beltActor);
				if (belt)
				{
					belt->mLength = 200;
					UClass* recipeClass = LoadObject<UClass>(NULL, TEXT("/DirectToSplitter/SnapOn_Recipe.SnapOn_Recipe_C"));
					belt->SetBuiltWithRecipe(recipeClass);
					if (factoryConn->CanConnectTo(belt->GetConnection0()))
					{
						factoryConn->SetConnection(belt->GetConnection0());
						otherConn->SetConnection(belt->GetConnection1());
					}
					else if (factoryConn->CanConnectTo(belt->GetConnection1()))
					{
						factoryConn->SetConnection(belt->GetConnection1());
						otherConn->SetConnection(belt->GetConnection0());
					}
				}
			}
		}
	}
}

bool FDirectToSplitterModule::IsSnappedOn(UFGFactoryConnectionComponent* Connection)
{
	if (!IsValid(Connection)) {
		return false;
	}
	if (UFGFactoryConnectionComponent* Other = Connection->GetConnection()) {
		// Snapped on means neither connection is from a conveyor belt/lift
		return !(Connection->GetOwner()->IsA<AFGBuildableConveyorBase>() || Other->GetOwner()->IsA<AFGBuildableConveyorBase>());
	}
	return false;
}

void FDirectToSplitterModule::TrySnapToActor_Hook(TCallScope<bool(*)
	(AFGConveyorAttachmentHologram*, const FHitResult&)>& scope ,
	AFGConveyorAttachmentHologram* self, const FHitResult& hitResult)
{
	
}

bool FDirectToSplitterModule::TrySnapToActor(AFGConveyorAttachmentHologram* self, const FHitResult& hitResult)
{

	self->mSnappedConnection = nullptr;
	//Get the (possibly) hit buildable
	AFGBuildable* hitBuildable = nullptr;
	auto hitInstanceMgr = Cast<AAbstractInstanceManager>(hitResult.GetActor());
	if (hitInstanceMgr)
	{
		FInstanceHandle outHandle;
		auto hitResolved = hitInstanceMgr->ResolveHit(hitResult, outHandle);
		if (hitResolved)
		{
			if (outHandle.GetOwner())
			{
				auto handleBuildable = Cast<AFGBuildable>(outHandle.GetOwner());
				if (handleBuildable)
				{
					hitBuildable = handleBuildable;
				}
			}
		}
	}
	if (!hitBuildable)
	{
		hitBuildable = Cast<AFGBuildable>(hitResult.GetActor());
	}

	if (hitBuildable)
	{
		if (auto convAttachment = Cast<AFGBuildableConveyorAttachment>(hitBuildable))
		{
			return false;
		}
		else if (self->mBuildClass == hitBuildable->GetClass())
		{
			return false;
		}
		else if (self->mBuildClass->GetName().Contains("ModularLoadBalancer") && hitBuildable->GetClass()->GetName().Contains("ModularLoadBalancer"))
		{
			return false;
		}
		//else if (self->mBuildClass->GetName().Contains("ModularLoadBalancer"))
		//{
		//	return false;
		//}
		//else if (self->mBuildClass->GetName().Contains("CounterLimiter"))
		//{
		//	return false;
		//}
		else if (auto lift = Cast<AFGBuildableConveyorLift>(hitBuildable))
		{
			return false;
		}
		FVector loc = self->GetActorLocation();
		FVector normal = self->GetActorForwardVector();

		auto components = hitBuildable->GetComponents();
		if (components.Num() > 0)
		{
			UFGFactoryConnectionComponent* closestConnection = nullptr;
			FVector closestLocation = FVector(0,0,0);
			for (auto component : components)
			{
				TArray< UFGFactoryConnectionComponent*> connections;
				UFGFactoryConnectionComponent* conn = Cast<UFGFactoryConnectionComponent>(component);
				if (conn)
				{
					if (conn->IsConnected())
					{
						continue;
					}
					connections.Add(conn);
				}
				for (auto connection : connections)
				{
					if (closestConnection == nullptr)
					{
						closestConnection = connection;
						closestLocation = connection->GetConnectorLocation();
					}
					else
					{
						auto myDistance = FVector::Distance(loc, connection->GetConnectorLocation());
						auto closestDistance = FVector::Distance(loc, closestLocation);
						if (myDistance < closestDistance)
						{
							closestConnection = connection;
							closestLocation = connection->GetConnectorLocation();
						}
					}
				}
			}

				//UFGFactoryConnectionComponent* conn = Cast<UFGFactoryConnectionComponent>(component);
				if (closestConnection)
				{
					auto direction = closestConnection->GetDirection();
					if (direction == EFactoryConnectionDirection::FCD_OUTPUT)
					{
						UFGFactoryConnectionComponent* myConnection = nullptr;
						for (auto myComp : self->mConnections)
						{
							if (myComp->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
							{
								myConnection = myComp;
								break;
							}
						}

						if (myConnection)
						{
							self->SnapToConnection(closestConnection, myConnection, closestLocation);
							self->mSnappedConnection = closestConnection;
							self->mSnappingConnectionIndex = 0;
							return true;
						}
						
					}
					else if (direction == EFactoryConnectionDirection::FCD_INPUT)
					{
						UFGFactoryConnectionComponent* myConnection = nullptr;
						for (auto myComp : self->mConnections)
						{
							if (myComp->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
							{
								myConnection = myComp;
								break;
							}
						}

						if (myConnection)
						{
							self->SnapToConnection(closestConnection, myConnection, closestLocation);
							self->mSnappedConnection = closestConnection;
							self->mSnappingConnectionIndex = 0;
							return true;
						}
					}
				
			}
		}
	}
	return false;
}

void FDirectToSplitterModule::CheckValidPlacement(AFGConveyorAttachmentHologram* self, bool& retflag)
{

	retflag = true;
	TArray< TSubclassOf<  UFGConstructDisqualifier > >out_constructResults;
	self->GetConstructDisqualifiers(out_constructResults);
	if (self->mSnappedConnection)
	{
		auto snappedConnectionName = self->mSnappedConnection->GetName();
		auto otherConn = self->mSnappedConnection;

		if (snappedConnectionName.Contains("ConveyorAny"))
		{
			return;
		}
		auto className = self->mBuildClass.Get()->GetName();
		//if (className == "Build_ConveyorAttachmentSplitterSmart_C" || className == "Build_ConveyorAttachmentSplitterProgrammable_C")
		//{
		//	self->ResetConstructDisqualifiers();
		//	self->AddConstructDisqualifier(USnapOnSplitterDisqualifier::StaticClass());
		//	return;
		//}
		auto snappedBuildable = self->mSnappedConnection->GetOuterBuildable();
		auto buildingName = snappedBuildable->GetName();

		bool shouldDisqualify = false;
		auto direction = self->mSnappedConnection->GetDirection();
		
		self->ResetConstructDisqualifiers();
		retflag = false;
		auto offset = FDTS_ConfigStruct::GetActiveConfig(self->GetWorld()).SnapOffset * 100.f;

		if (buildingName.StartsWith("Build_Manufacturer") && otherConn->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
		{
			// Add additional 0.25 offset for Manufacturer inputs weirdness
			offset += 25.f;
		}
		else if (buildingName.StartsWith("Build_HadronCollider") && otherConn->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
		{
			// Subtract 0.1 offset from Particle Accelerator output
			offset -= 10.f;
		}
		else if (buildingName.StartsWith("Build_Converter"))
		{
			if (otherConn->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
			{
				// Add 0.2 offset to Converter Input
				offset += 20.f;
			}
			else if (otherConn->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
			{
				// Add 0.4 offset to Converter Output
				offset += 40.f;
			}
		}
		else if (buildingName.StartsWith("Build_QuantumEncoder") && otherConn->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
		{
			if (snappedConnectionName == "Input1" || snappedConnectionName == "Input2")
			{
				// Add 0.4 offset to Encoder input
				offset += 40.f;
			}
		}

		auto compLocation = self->mSnappedConnection->GetConnectorLocation();
		
		FVector addVector = self->mSnappedConnection->GetForwardVector() * offset;
		auto newLoc = compLocation + addVector;
		self->SetActorLocation(newLoc);
	}
}

void FDirectToSplitterModule::CheckValidFactoryPlacement(AFGFactoryHologram* self, bool& retflag)
{
	retflag = true;
	auto snappedBuilding = self->GetSnappedBuilding();
	TArray< TSubclassOf<  UFGConstructDisqualifier > >out_constructResults;
	self->GetConstructDisqualifiers(out_constructResults);
}

bool FDirectToSplitterModule::IsValidHitResult(const AFGConveyorAttachmentHologram* self, const FHitResult& hitResult)
{
	auto splineHolo = Cast<AFGSplineHologram>(self);
	if (splineHolo)
	{
		return false;
	}
	auto poleHolo = Cast<AFGPoleHologram>(self);
	if (poleHolo)
	{
		return false;
	}
	//auto pipelineHolo = Cast<AFGPipelineSupportHologram>(self);
	//if (pipelineHolo)
	//{
	//	return false;
	//}
	auto wallHolo = Cast<AFGWallAttachmentHologram>(self);
	if (wallHolo)
	{
		return false;
	}
	auto wireHolo = Cast<AFGWireHologram>(self);
	if (wireHolo)
	{
		return false;
	}
	return true;
}

void FDirectToSplitterModule::CheckValidFloor(AFGConveyorAttachmentHologram* self)
{
	auto building = Cast<AFGBuildableFactory>(self->mSnappedBuilding);
	auto cb = Cast<AFGBuildableConveyorBase>(self->mSnappedBuilding);
	if (!building && !cb)
	{
		self->mSnappedConnection = nullptr;
	}
}

void FDirectToSplitterModule::ConfigureComponents(const AFGConveyorAttachmentHologram* self, bool& retflag)
{
	retflag = false;
	if (self->mSnappedConnection)
	{
		auto snappedConnectionName = self->mSnappedConnection->GetName();
		auto snappedConveyor = Cast<AFGBuildableConveyorBase>(self->mSnappedConnection->GetOwner());
		auto snappedLift = Cast<AFGBuildableConveyorLift>(self->mSnappedConnection->GetOwner());
		if (snappedConveyor || snappedLift)
		{
			return;
		}
		if (snappedConnectionName == "ConveyorAny0")
		{
			return;
		}
		AFGConveyorAttachmentHologram* hg = const_cast<AFGConveyorAttachmentHologram*>(self);
		//Store snapped connection with current HG
		mSnappedConnections.Add(hg, hg->mSnappedConnection);
		ConveyorAttachmentInfo cai;
		cai.otherConnection = hg->mSnappedConnection;
		cai.connectionIndex = hg->mSnappingConnectionIndex;
		AttachmentInfos.Add(hg, cai);

		hg->mSnappingConnectionIndex = -1;
		hg->mSnappedConnection->ClearConnection();
		hg->mSnappedConnection = nullptr;
		retflag = true;
	}
}

void FDirectToSplitterModule::HGConstruct(AFGBuildableHologram* hg, AActor* buildable)
{
	if (auto splitterhg = Cast< AFGConveyorAttachmentHologram>(hg))
	{
		if (AttachmentInfos.Num() > 0 && AttachmentInfos.Contains(splitterhg))
		{
			auto otherConnection = AttachmentInfos[splitterhg].otherConnection;
			auto otherDirection = otherConnection->GetDirection();
			auto connectionIndex = AttachmentInfos[splitterhg].connectionIndex;
			if (auto splitter = Cast< AFGBuildableConveyorAttachment>(buildable))
			{
				if (otherConnection)
				{
					if (otherDirection == EFactoryConnectionDirection::FCD_OUTPUT)
					{
						auto inputs = splitter->mInputs;
						if (inputs.Num() > 0) 
						{
							UFGFactoryConnectionComponent* myConnection = nullptr;

							if (inputs.Num() == 1) //Splitter - only 1 input
							{
								myConnection = inputs[0];
							}
							else // Merger, I guess
							{
								if (connectionIndex == 0)
								{
									myConnection = inputs[0];
								}
								else if (connectionIndex == 2)
								{
									myConnection = inputs[1];
								}
								else if (connectionIndex == 3)
								{
									myConnection = inputs[2];
								}
							}

							if (myConnection && otherConnection->CanConnectTo(myConnection))
							{
								otherConnection->SetConnection(myConnection);
							}
						}
					}
					else if (otherDirection == EFactoryConnectionDirection::FCD_INPUT)
					{
						auto outputs = splitter->mOutputs;
						if (outputs.Num() > 0)
						{
							UFGFactoryConnectionComponent* myConnection = nullptr;

							if (outputs.Num() == 1) //Merger - only 1 input
							{
								myConnection = outputs[0];
							}
							else // Splitter, I guess
							{
								if (connectionIndex == 0)
								{
									myConnection = outputs[0];
								}
								else if (connectionIndex == 2)
								{
									myConnection = outputs[2];
								}
								else if (connectionIndex == 3)
								{
									myConnection = outputs[1];
								}
							}

							if (myConnection && otherConnection->CanConnectTo(myConnection))
							{
								otherConnection->SetConnection(myConnection);
							}
						}
					}
				}
			}
			else
			{
				auto buildableFactory = Cast< AFGBuildableFactory>(buildable);
				if (buildableFactory)
				{
					auto numConnections = buildableFactory->GetNumFactoryConnections();
					if (numConnections > 0 && otherConnection)
					{
						auto connections = buildableFactory->GetConnectionComponents();
						TArray<UFGFactoryConnectionComponent*> inputs;
						TArray<UFGFactoryConnectionComponent*> outputs;
						for (auto conn : connections)
						{
							if (conn->GetConnector() == EFactoryConnectionConnector::FCC_CONVEYOR)
							{
								if (conn->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
								{
									inputs.Add(conn);
								}
								else if (conn->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
								{
									outputs.Add(conn);
								}
							}
						}
						if (otherDirection == EFactoryConnectionDirection::FCD_OUTPUT)
						{
							if (inputs.Num() > 0)
							{
								UFGFactoryConnectionComponent* myConnection = nullptr;

								if (inputs.Num() == 1) //Splitter - only 1 input
								{
									myConnection = inputs[0];
								}

								if (myConnection && otherConnection->CanConnectTo(myConnection))
								{
									otherConnection->SetConnection(myConnection);
								}
							}
						}
						else if (otherDirection == EFactoryConnectionDirection::FCD_INPUT)
						{
							if (outputs.Num() > 0)
							{
								UFGFactoryConnectionComponent* myConnection = nullptr;

								if (outputs.Num() == 1) //Merger - only 1 input
								{
									myConnection = outputs[0];
								}

								if (myConnection && otherConnection->CanConnectTo(myConnection))
								{
									otherConnection->SetConnection(myConnection);
								}
							}
						}
					}
				}
			}

			AttachmentInfos.Remove(splitterhg);
		}
		return;
		if (mSnappedConnections.Contains(splitterhg))
		{
			if (UFGFactoryConnectionComponent* connection = mSnappedConnections[splitterhg])
			{
				auto direction = connection->GetDirection();
				if (direction == EFactoryConnectionDirection::FCD_OUTPUT)
				{
					if (auto splitter = Cast< AFGBuildableConveyorAttachment>(buildable))
					{
						auto inputs = splitter->mInputs;
						if (inputs.Num() > 0)
						{
							auto input = inputs[0];
							if (input && connection->CanConnectTo(input))
							{
								input->SetConnection(connection);
							}
						}
					}
				}
			}
		}
	}

}


bool DoOnce = false;
int i = 0;
bool FDirectToSplitterModule::PipeSnap(AFGPipeAttachmentHologram* self, const FHitResult& hitResult)
{

	
	auto junction = Cast<AFGPipelineJunctionHologram>(self);
	if (junction == nullptr)
	{
		return false;
	}
	//Get the (possibly) hit buildable
	AFGBuildable* hitBuildable = nullptr;
	auto hitInstanceMgr = Cast<AAbstractInstanceManager>(hitResult.GetActor());
	if (hitInstanceMgr)
	{
		FInstanceHandle outHandle;
		auto hitResolved = hitInstanceMgr->ResolveHit(hitResult, outHandle);
		if (hitResolved)
		{
			if (outHandle.GetOwner())
			{
				auto handleBuildable = Cast<AFGBuildable>(outHandle.GetOwner());
				if (handleBuildable)
				{
					hitBuildable = handleBuildable;
				}
			}
		}
	}
	if (!hitBuildable)
	{
		hitBuildable = Cast<AFGBuildable>(hitResult.GetActor());
	}

	if (hitBuildable)
	{
		FVector loc = self->GetActorLocation();
		FVector normal = self->GetActorForwardVector();
		
		auto components = hitBuildable->GetComponents();
		if (components.Num() > 0)
		{
			//Get the pipe connections
			TArray<UFGPipeConnectionComponent*> connections;
			for (auto component : components)
			{
				UFGPipeConnectionComponent* fccomp = Cast< UFGPipeConnectionComponent>(component);
				if (fccomp)
				{
					if (fccomp->IsConnected())
					{
						continue;
					}
					connections.Add(fccomp);
				}
			}
			
			if (connections.Num() > 0)
			{
				//Find the closest pipe connection
				UFGPipeConnectionComponent* closestConnection = nullptr;
				double closestDistance;
				FVector myLoc = self->GetActorLocation();
				for (auto conn : connections)
				{
					FVector connLoc = conn->GetComponentLocation();
					auto distance = FVector::Dist(hitResult.Location, connLoc);

					if (!closestConnection || distance < closestDistance)
					{
						closestConnection = conn;
						closestDistance = distance;
					}
				}
				
				auto myPipeConns = self->GetCachedPipeConnectionComponents();
				if (myPipeConns.Num() > 0 && closestConnection)
				{
					
					auto hg = Cast<AFGHologram>(junction);
					auto contr = Cast<APlayerController>(hg->GetConstructionInstigator()->GetController());

									if (contr->IsInputKeyDown(EKeys::Down) && !DoOnce)
				{
					
					if (i + 1 < myPipeConns.Num())
					{
						i++;
						UE_LOG(LogNativeHookManager, Display, TEXT("Incrementing"));
					}
					else
					{
						i = 0;
						UE_LOG(LogNativeHookManager, Display, TEXT("Rolling Over"));
					}
					DoOnce = true;
				}
				else if (!contr->IsInputKeyDown(EKeys::Up) && !contr->IsInputKeyDown(EKeys::Down))
				{
					if (DoOnce)
					{
						DoOnce = false;
					}
				}
				else if (contr->IsInputKeyDown(EKeys::Up) && !DoOnce)
				{
					
					if (i > 0)
					{
						i--;
						UE_LOG(LogNativeHookManager, Display, TEXT("Decrementing"));
					}
					else
					{
						i = myPipeConns.Num() - 1;
						UE_LOG(LogNativeHookManager, Display, TEXT("Rolling Over"));
					}
					DoOnce = true;
				}
				else if (!contr->IsInputKeyDown(EKeys::Down) && !contr->IsInputKeyDown(EKeys::Up))
				{
					if (DoOnce)
					{
						DoOnce = false;
					}
				}
				
				UFGPipeConnectionComponent* myCompToSnap = nullptr;
				//for (auto myConn : myPipeConns)
				//{

				auto myType = myPipeConns[i]->GetPipeConnectionType();
				auto closestDirection = closestConnection->GetPipeConnectionType();

				if (myType == EPipeConnectionType::PCT_ANY || myType == EPipeConnectionType::PCT_SNAP_ONLY)
				{
					myCompToSnap = myPipeConns[i];
					//break;
				}
				else if (closestDirection == EPipeConnectionType::PCT_CONSUMER && myType == EPipeConnectionType::PCT_PRODUCER)
				{
					myCompToSnap = myPipeConns[i];
					//break;
				}
				else if (closestDirection == EPipeConnectionType::PCT_PRODUCER && myType == EPipeConnectionType::PCT_CONSUMER)
				{
					myCompToSnap = myPipeConns[i];
					//break;
				}
				//}
				
				if (myCompToSnap)
				{
					self->mSnappedConnectionComponent = closestConnection;
					auto newConnectionLocation = closestConnection->GetComponentLocation();
					auto newConnectionRotation = closestConnection->GetComponentRotation();
					auto myConnectionLocation = myCompToSnap->GetComponentLocation();
					auto myConnectionRotation = myCompToSnap->GetComponentRotation();

					auto rotationDifference = newConnectionRotation - myConnectionRotation;
					auto rotationDiffForwardVector = UKismetMathLibrary::GetForwardVector(rotationDifference) * -1.f;
					auto rotationFromX = UKismetMathLibrary::MakeRotFromX(rotationDiffForwardVector);
					self->AddActorWorldRotation(rotationFromX);

					auto normalizedLocationDiff = newConnectionLocation - myConnectionLocation;
					normalizedLocationDiff.Normalize(0.0001);
					auto distanceBetweenComps = UKismetMathLibrary::Vector_Distance(newConnectionLocation, myConnectionLocation);
					auto newLocation = (normalizedLocationDiff * distanceBetweenComps) + myConnectionLocation;
					auto offset = FDTS_ConfigStruct::GetActiveConfig(self->GetWorld()).PipeSnapOffset * 100.f;
					auto compForward = closestConnection->GetForwardVector();
					FVector addVector = compForward * offset;
					self->SetActorLocation(newLocation + addVector);
					self->mSnapConnectionIndex = i;
					return true;
				}
			}
		}
	}
}
self->mSnappedConnectionComponent = nullptr;
i = 0;
return false;
}

#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FDirectToSplitterModule, DirectToSplitter);