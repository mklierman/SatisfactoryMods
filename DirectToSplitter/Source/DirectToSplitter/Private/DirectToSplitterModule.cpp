#include "DirectToSplitterModule.h"
#include "DTS_ConfigStruct.h"
#include "Registry/ModContentRegistry.h"
#include <Kismet/KismetMathLibrary.h>

//#pragma optimize("", off)
//#if !WITH_EDITOR
//#endif
void FDirectToSplitterModule::StartupModule() {
	AFGConveyorAttachmentHologram* cah = GetMutableDefault<AFGConveyorAttachmentHologram>();
	AFGFactoryHologram* fh = GetMutableDefault<AFGFactoryHologram>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::ConfigureComponents, cah, [=](auto& scope, const AFGConveyorAttachmentHologram* self, AFGBuildable* inBuildable)
		{
			bool shouldCancel;
			ConfigureComponents(self, shouldCancel);
			if (shouldCancel) scope.Cancel();
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::CheckValidPlacement, cah, [=](auto& scope, AFGConveyorAttachmentHologram* self)
		{
			bool retflag;
			CheckValidPlacement(self, retflag);
			if (retflag) return;
			scope.Cancel();
		});

	AFGBuildableHologram* bhg = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::Construct, bhg, [=](auto& outActor, AFGBuildableHologram* self, TArray< AActor* >& out_children, FNetConstructionID netConstructionID)
		{
			HGConstruct(self, outActor);
		});

	AFGPipeAttachmentHologram* pahg = GetMutableDefault<AFGPipeAttachmentHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::IsValidHitResult, pahg, [=](auto& scope, const AFGPipeAttachmentHologram* self, const FHitResult& hitResult)
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
					auto components = hitBuildable->GetComponentsByClass(UFGPipeConnectionComponent::StaticClass());
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
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::TrySnapToActor, pahg, [=](auto& scope, AFGPipeAttachmentHologram* self, const FHitResult& hitResult)
		{
			bool result = (bool)scope(self, hitResult);
			auto pipeAttachHolo = Cast<AFGPipeAttachmentHologram>(self);
			if (!result && pipeAttachHolo && hitResult.GetActor())
			{
				scope.Override(PipeSnap(pipeAttachHolo, hitResult));

				return;
			}
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGPipeAttachmentHologram::CheckValidPlacement, pahg, [=](auto& scope, AFGPipeAttachmentHologram* self)
		{
			auto junction = Cast<AFGPipelineJunctionHologram>(self);
			if (junction && self->mSnappedConnectionComponent)
			{
				self->ResetConstructDisqualifiers();
				scope.Cancel();
			}
		});
#if !WITH_EDITOR
#endif
}

void FDirectToSplitterModule::CheckValidPlacement(AFGConveyorAttachmentHologram* self, bool& retflag)
{
	retflag = true;
	TArray< TSubclassOf<  UFGConstructDisqualifier > >out_constructResults;
	self->GetConstructDisqualifiers(out_constructResults);
	if (self->mSnappedConection)
	{
		auto snappedConnectionName = self->mSnappedConection->GetName();
		if (snappedConnectionName.Contains("ConveyorAny"))
		{
			return;
		}
		auto className = self->mBuildClass.Get()->GetName();
		if (className == "Build_ConveyorAttachmentSplitterSmart_C" || className == "Build_ConveyorAttachmentSplitterProgrammable_C")
		{
			self->ResetConstructDisqualifiers();
			self->AddConstructDisqualifier(USnapOnSplitterDisqualifier::StaticClass());
			return;
		}
		auto snappedBuildable = self->mSnappedConection->GetOuterBuildable();
		
		//if (snappedBuildable->mDecoratorClass)
		//{
		//	auto name = snappedBuildable->mDecoratorClass->GetName();
		//	if (name.Equals("Deco_StorageContainerMk2_C"))
		//	{
		//		self->AddConstructDisqualifier(USnapOnDisqualifier::StaticClass());
		//		return;
		//	}
		//}
		auto snappedInventory = self->mSnappedConection->GetInventory();
		auto direction = self->mSnappedConection->GetDirection();
		auto inventoryIndex = self->mSnappedConection->GetInventoryAccessIndex();

		for (UActorComponent* ComponentsByClass : snappedBuildable->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass()))
		{
			if (UFGFactoryConnectionComponent* ConnectionComponent = Cast<UFGFactoryConnectionComponent>(ComponentsByClass))
			{
				if (ConnectionComponent != self->mSnappedConection 
					&& ConnectionComponent->GetConnector() == EFactoryConnectionConnector::FCC_CONVEYOR
					&& ConnectionComponent->GetInventory() == snappedInventory
					&& ConnectionComponent->GetDirection() == EFactoryConnectionDirection::FCD_OUTPUT)
				{
					self->ResetConstructDisqualifiers();
					self->AddConstructDisqualifier(USnapOnDisqualifier::StaticClass());
					return;
				}
			}
		}

		//Currently broken
		//auto modRegistry = UModContentRegistry::Get(self->GetWorld());
		//if (!modRegistry->IsRecipeVanilla(snappedBuildable->GetBuiltWithRecipe()))
		//{
		//	return;
		//}

		//auto pathName = snappedBuildable->GetBuiltWithRecipe()->GetPathName();
		//if (!pathName.Contains("FactoryGame"))
		//{
		//	return;
		//}
		
		self->ResetConstructDisqualifiers();
		retflag = false;
		auto offset = FDTS_ConfigStruct::GetActiveConfig(self->GetWorld()).SnapOffset * 100.f;

		auto compLocation = self->mSnappedConection->GetConnectorLocation();
		
		FVector addVector = self->mSnappedConection->GetForwardVector() * offset;
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
	//if (self->mSnappedConection)
	//{
	//	auto snappedConnectionName = self->mSnappedConection->GetName();
	//	if (snappedConnectionName.Contains("ConveyorAny"))
	//	{
	//		return;
	//	}
	//	auto snappedBuildable = self->mSnappedConection->GetOuterBuildable();

	//	if (snappedBuildable->mDecoratorClass)
	//	{
	//		auto name = snappedBuildable->mDecoratorClass->GetName();
	//		if (name.Equals("Deco_StorageContainerMk2_C"))
	//		{
	//			return;
	//		}
	//	}

	//	Currently broken
	//	auto modRegistry = UModContentRegistry::Get(self->GetWorld());
	//	if (!modRegistry->IsRecipeVanilla(snappedBuildable->GetBuiltWithRecipe()))
	//	{
	//		return;
	//	}

	//	auto pathName = snappedBuildable->GetBuiltWithRecipe()->GetPathName();
	//	if (!pathName.Contains("FactoryGame"))
	//	{
	//		return;
	//	}

	//	self->ResetConstructDisqualifiers();
	//	retflag = false;
	//	auto offset = FDTS_ConfigStruct::GetActiveConfig(self->GetWorld()).SnapOffset * 100.f;

	//	auto compLocation = self->mSnappedConection->GetConnectorLocation();

	//	FVector addVector = self->mSnappedConection->GetForwardVector() * offset;
	//	auto newLoc = compLocation + addVector;
	//	self->SetActorLocation(newLoc);
	//}
}

void FDirectToSplitterModule::ConfigureComponents(const AFGConveyorAttachmentHologram* self, bool& retflag)
{
	retflag = false;
	if (self->mSnappedConection)
	{
		auto snappedConnectionName = self->mSnappedConection->GetName();
		auto snappedConveyor = Cast<AFGBuildableConveyorBase>(self->mSnappedConection->GetOwner());
		auto snappedLift = Cast<AFGBuildableConveyorLift>(self->mSnappedConection->GetOwner());
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
		mSnappedConnections.Add(hg, hg->mSnappedConection);
		ConveyorAttachmentInfo cai;
		cai.otherConnection = hg->mSnappedConection;
		cai.connectionIndex = hg->mSnappingConnectionIndex;
		AttachmentInfos.Add(hg, cai);

		hg->mSnappingConnectionIndex = -1;
		hg->mSnappedConection->ClearConnection();
		hg->mSnappedConection = nullptr;
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

		auto components = hitBuildable->GetComponentsByClass(UFGPipeConnectionComponent::StaticClass());
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
					UFGPipeConnectionComponent* myCompToSnap = nullptr;
					for (auto myConn : myPipeConns)
					{
						auto myType = myConn->GetPipeConnectionType();
						if (myType == EPipeConnectionType::PCT_ANY || myType == EPipeConnectionType::PCT_SNAP_ONLY)
						{
							myCompToSnap = myConn;
							break;
						}

						auto closestDirection = closestConnection->GetPipeConnectionType();
						if (closestDirection == EPipeConnectionType::PCT_CONSUMER && myType == EPipeConnectionType::PCT_PRODUCER)
						{
							myCompToSnap = myConn;
							break;
						}
						else if (closestDirection == EPipeConnectionType::PCT_PRODUCER && myType == EPipeConnectionType::PCT_CONSUMER)
						{
							myCompToSnap = myConn;
							break;
						}
					}

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
						return true;
					}
				}
			}
		}
	}
	self->mSnappedConnectionComponent = nullptr;
	return false;
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FDirectToSplitterModule, DirectToSplitter);