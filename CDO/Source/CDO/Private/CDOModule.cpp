#include "CDOModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "FGClearanceComponent.h"
#include "Patching/NativeHookManager.h"
#include "Creature/FGCreature.h"
#include "Creature/Enemy/FGEnemy.h"
#include "Creature/FGCreatureSpawner.h"
#include "FGHardDriveSettings.h"
#include "Resources/FGItemDescriptor.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "WheeledVehicles/FGWheeledVehicleMovementComponent6W.h"
#include "WheeledVehicles/FGWheeledVehicleMovementComponent4W.h"
#include "FGFactoryConnectionComponent.h"
#include "WheeledVehicleMovementComponent.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "FGResourceSinkSettings.h"
#include "Equipment/FGResourceScanner.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Hologram/FGPoleHologram.h"
#include "FGPillarHologram.h"
#include "Hologram/FGHologram.h"
#include "FGBeamHologram.h"
#include "Hologram/FGBuildableHologram.h"
#include "FGDroneVehicle.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGConveyorLiftHologram.h"
#include "Equipment/FGBuildGun.h"
#include "Hologram/FGPipelineAttachmentHologram.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "Components/StaticMeshComponent.h"
#include <Runtime/Engine/Classes/Components/ProxyInstancedStaticMeshComponent.h>
#include "FGColoredInstanceMeshProxy.h"
#include "Resources/FGItemDescriptor.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "Misc/AssertionMacros.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "FGGameState.h"
#include "FGSplineMeshGenerationLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "FGInstancedSplineMeshComponent.h"
#include "FGVehicleWheel.h"

DEFINE_LOG_CATEGORY(CDO_Log);


//bool MSPlacementThing(T scope, AFGPoleHologram* self, bool isInputFromARelease)
//{
//	return false;
//}


// simple signature scanning helper
//char* scan_signature(char* pattern, char* mask, char* begin, size_t size)
//{
//	MEMORY_BASIC_INFORMATION memInfo;
//	for (char* off = begin; off < (begin + size); off += memInfo.RegionSize)
//	{
//		if (!VirtualQuery(off, &memInfo, sizeof(memInfo))) return nullptr;
//		if (memInfo.State != MEM_COMMIT || memInfo.Protect == PAGE_NOACCESS) continue;
//		size_t patternSize = strlen(mask);
//		for (int i = 0; i < (memInfo.RegionSize - patternSize); i++) {
//			bool valid = true;
//			for (int j = 0; j < patternSize; j++) {
//				if (mask[j] == 'x') {
//					char* ptr = begin + i + j;
//					if (*ptr != pattern[j]) {
//						valid = false;
//						break;
//					}
//				}
//			}
//			if (valid)
//				return begin + i;
//		}
//	}
//	return nullptr;
//}
//void patch_only_connect_to_conveyors() {
//	char* moduleBase = (char*)GetModuleHandleW(L"FactoryGame-FactoryGame-Win64-Shipping.dll");
//	// FactoryGame-FactoryGame-Win64-Shipping.dll+03ca706
//	char* addr = scan_signature("\x74\x00\xe8\x00\x00\x00\x00\x48\x8b\x4b\x00\x48\x8b\xd0\xff\x15\x00\x00\x00\x00\x84\xc0\x75\x00\x48\x8d\x4c\x24", "x?x????xxx?xxxxx????xxx?xxxx", moduleBase, INT_MAX);
//	DWORD oldProtection;
//	VirtualProtect(addr, 2, PAGE_EXECUTE_READWRITE, &oldProtection);
//	// Replace "je 0x18" with "je 0x2b" to bypass the check
//	//*addr = 0x74;
//	*addr = (char)(unsigned char)0xEB;
//	*(addr + 1) = 0x27;
//	FlushInstructionCache(GetCurrentProcess(), addr, 2);
//	DWORD dummy;
//	VirtualProtect(addr, 2, oldProtection, &dummy);
//}

void FCDOModule::StartupModule() {
	//verifyf(true == true, "");
		//SUBSCRIBE_METHOD(FDebug::AssertFailed, [](auto& scope, FDebug self, const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Format, ...)
		//	{
		//		scope.Cancel();
		//	});
		//patch_only_connect_to_conveyors();
	//void AFGHologram::AddConstructDisqualifier(TSubclassOf<  UFGConstructDisqualifier > disqualifier) { }
	//SUBSCRIBE_METHOD(AFGHologram::AddConstructDisqualifier, [](auto scope, AFGHologram* self, TSubclassOf<  UFGConstructDisqualifier > disqualifier)
	//	{
	//		scope.Cancel();
	//	});
	//virtual void ConfigureComponents(class AFGBuildable* inBuildable) const override;
#if !WITH_EDITOR
// 	static void BuildSplineMeshesInstanced(
	//class USplineComponent* spline,
	//	UStaticMesh* mesh,
	//	float meshLength,
	//	UFGInstancedSplineMeshComponent* splineInstances );


	//AFGFoundationHologram* fh = GetMutableDefault<AFGFoundationHologram>();
	////virtual bool AFGFoundationHologram::TrySnapToActor(const FHitResult & hitResult) override;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::TrySnapToActor, fh, [=](auto& scope, AFGFoundationHologram* self, const FHitResult& hitResult)
	//{
	//		auto res = scope(self, hitResult);
	//		if (res)
	//		{
	//			UE_LOG(CDO_Log, Display, TEXT("Snapped to actor: %s"), *hitResult.GetActor()->GetName());
	//		}
	//	//UE_LOG(CDO_Log, Display, TEXT("VerifyDefaults: %s"), scope.Result.ToString());
	//});

#endif
	//AFGBuildableConveyorBelt* cah = GetMutableDefault<AFGBuildableConveyorBelt>();
	////virtual bool VerifyDefaults( FString& out_message )
	////SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableConveyorBelt::VerifyDefaults, cah, [](auto& scope, AFGBuildableConveyorBelt* self, FString& out_message)
	////	{
	////		UE_LOG(CDO_Log, Display, TEXT("VerifyDefaults: %s"), scope.Result.ToString());
	////	});

	//AFGBuildable* bd = GetMutableDefault<AFGBuildable>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::ShouldSkipBuildEffect, bd, [](auto& scope, AFGBuildable* self)
	//	{
	//		scope.Override(true);
	//	});

	//UStaticMesh* GetSplineMesh() const { return mMesh; }

	//AFGConveyorAttachmentHologram* cah = GetMutableDefault<AFGConveyorAttachmentHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::ConfigureComponents, cah, [](auto& scope, const AFGConveyorAttachmentHologram* self, AFGBuildable* inBuildable)
	//		{
	//		if (self->mSnappedConection)
	//		{
	//			auto snappedConveyor = Cast<AFGBuildableConveyorBase>(self->mSnappedConection->GetOwner());
	//			if (snappedConveyor)
	//			{
	//				return;
	//			}
	//			AFGConveyorAttachmentHologram* hg = const_cast<AFGConveyorAttachmentHologram*>(self);

	//			hg->mConnections.Empty();
	//			hg->mSnappingConnectionIndex = -1;
	//			hg->mSnappedConection->ClearConnection();
	//			hg->mSnappedConection = nullptr;
	//			UE_LOG(CDO_Log, Display, TEXT("hg->mSnappedConection = nullptr;"));
	//		}
	//		//snappedConn = nullptr;
	//		//auto snapped = self->mSnappedConveyor;
	//		////UE_LOG(CDO_Log, Display, TEXT("snapped->GetName(): %s"), snapped->GetName());
	//		//	//auto snappedCB = Cast<AFGBuildableConveyorBase>(snapped);
	//		//if (snapped)
	//		//{
	//		//	return;
	//		//}
	//			//scope.Cancel();
	//		});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorAttachmentHologram::CheckValidPlacement, cah, [](auto scope, AFGConveyorAttachmentHologram* self)
	//	{
	//		self->ResetConstructDisqualifiers();
	//		scope.Cancel();
	//	});

	//AFGPipelineAttachmentHologram* pah = GetMutableDefault<AFGPipelineAttachmentHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGPipelineAttachmentHologram::CheckValidPlacement, pah, [](auto scope, AFGPipelineAttachmentHologram* self)
	//	{
	//		self->ResetConstructDisqualifiers();
	//		scope.Cancel();
	//	});
	//SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::CanSnapTo, [](auto scope, const UFGFactoryConnectionComponent* self, UFGFactoryConnectionComponent* otherConnection)
	//{
	//		//self->ResetConstructDisqualifiers();
	//		scope.Override(true);
	//});
	//SUBSCRIBE_METHOD(UFGFactoryConnectionComponent::CanConnectTo, [](auto scope, const UFGFactoryConnectionComponent* self, UFGFactoryConnectionComponent* otherConnection)
	//	{
	//		//self->ResetConstructDisqualifiers();
	//		scope.Override(true);
	//	});
//#endif
		//	{
		//		self->mFineTuneRotationStep = 1;
		//	});
	//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
//#if !WITH_EDITOR
	//AFGDroneVehicle* droneCDO = GetMutableDefault<AFGDroneVehicle>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGDroneVehicle::IsUseable_Implementation, droneCDO, [](auto scope, const AFGDroneVehicle* self)
	//	{
	//		scope.Override(true);
	//	});


	//AFGFoundationHologram* fdnCDO = GetMutableDefault<AFGFoundationHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::BeginPlay, fdnCDO, [](auto scope, AFGFoundationHologram* self)
	//	{
	//		self->mFineTuneRotationStep = 1;
	//	});
	//void SetScrollRotateValue(int32 rotValue) { mScrollRotation = rotValue; }
	//AFGHologram* fCDO = GetMutableDefault<AFGHologram>();
	//SUBSCRIBE_METHOD(AFGHologram::GetScrollRotateValue, [](auto scope, const AFGHologram* self)
	//	{
	//		//self->mScrollRotation = self->mScrollRotation - 9;
	//		float rotfloat = (float)self->mScrollRotation;
	//		UE_LOG(CDO_Log, Display, TEXT("AFGHologram::GetScrollRotateValue: %f"), rotfloat);
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		//scope.Cancel();
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::ScrollRotate, fCDO, [](auto scope, AFGHologram* self, int32 delta, int32 step)
	//	{/*
	//		FRotator newrot = FRotator(0, delta * 5, 0);
	//		newrot = newrot + self->GetActorRotation();
	//		self->SetActorRotation(newrot);*/

	//		float rotfloat = (float)self->mScrollRotation;
	//		//UE_LOG(CDO_Log, Display, TEXT("AFGHologram::ScrollRotate: %f"), rotfloat);
	//		self->mScrollRotation = 5;
	//		//rotfloat = (float)self->mScrollRotation;
	//		UE_LOG(CDO_Log, Display, TEXT("AFGHologram::ScrollRotate: %f"), rotfloat);
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		//scope.Cancel();
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::UpdateRotationValuesFromTransform, fCDO, [](auto scope, AFGHologram* self)
	//	{
	//		UE_LOG(CDO_Log, Display, TEXT("UpdateRotationValuesFromTransform"));
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		scope.Cancel();
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::SetHologramLocationAndRotation, fCDO, [](auto scope, AFGHologram* self, const FHitResult& hitResult)
	//	{
	//		UE_LOG(CDO_Log, Display, TEXT("AFGHologram::SetHologramLocationAndRotation"));
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		scope.Cancel();
	//	});
	//AFGBuildableHologram* bhgCDO = GetMutableDefault<AFGBuildableHologram>();
	////virtual void ConfigureActor(class AFGBuildable* inBuildable) const;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::ConfigureActor, bhgCDO, [](auto scope, const AFGBuildableHologram* self, AFGBuildable* inBuildable)
	//	{
	//			TInlineComponentArray<UMeshComponent*> MeshComps(inBuildable);
	//			for (const auto& MeshComp : MeshComps) {
	//				auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
	//				if (StaticMeshProxy) {
	//					StaticMeshProxy->SetInstanced(false);
	//				}
	//				else {
	//					auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
	//					if (ColoredMeshProxy) {
	//						ColoredMeshProxy->mBlockInstancing = !false;
	//						ColoredMeshProxy->SetInstanced(false);
	//					}
	//					else {
	//						auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
	//						if (ProdIndInst) {
	//							ProdIndInst->SetInstanced(false);
	//						}
	//					}
	//				}
	//				//SetMeshInstanced(MeshComp, false); // false = before swap, true = after restore
	//			}
	//	});
	//AFGBuildable* self = nullptr;
	//auto currentMesh = Cast<UFGColoredInstanceMeshProxy>(self->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass()));
	//if (currentMesh)
	//{
	//	UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(self, UStaticMeshComponent::StaticClass());

	//	if (newComponent->AttachTo(self->GetRootComponent(), NAME_None))
	//	{
	//		FVector newLocation = currentMesh->GetRelativeLocation();
	//		FQuat newRotation = FQuat(currentMesh->GetRelativeRotation().Quaternion());

	//		newComponent->SetRelativeLocationAndRotation(newLocation, newRotation);
	//		newComponent->SetStaticMesh(currentMesh->GetStaticMesh());
	//		newComponent->RegisterComponent();

	//		TArray<USceneComponent*> ChildrenComps;
	//		currentMesh->GetChildrenComponents(true, ChildrenComps);
	//		for (auto comp : ChildrenComps)
	//		{
	//			comp->AttachToComponent(newComponent, FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	//		}

	//		currentMesh->DestroyComponent();
	//	}
	//}

	/*AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::BeginPlay, bCDO, [](auto scope, const AFGBuildable* self)
		{
			auto currentMesh = Cast<UFGColoredInstanceMeshProxy>(self->GetComponentByClass(UFGColoredInstanceMeshProxy::StaticClass()));
			if (currentMesh)
			{
				UStaticMeshComponent* newComponent = NewObject<UStaticMeshComponent>(self->GetOuter(), UStaticMeshComponent::StaticClass());

				if (newComponent->AttachTo(self->GetRootComponent(), NAME_None))
				{
					FVector newLocation = currentMesh->GetRelativeLocation();
					FQuat newRotation = FQuat(currentMesh->GetRelativeRotation().Quaternion());

					newComponent->SetRelativeLocationAndRotation(newLocation, newRotation);
					newComponent->SetStaticMesh(currentMesh->GetStaticMesh());
					newComponent->RegisterComponent();

					TArray<USceneComponent*> ChildrenComps;
					currentMesh->GetChildrenComponents(true, ChildrenComps);
					for (auto comp : ChildrenComps)
					{
						comp->AttachToComponent(newComponent, FAttachmentTransformRules::KeepRelativeTransform, comp->GetAttachSocketName());
					}

					currentMesh->DestroyComponent();
				}
			}*/


			//self->GetRootComponent()->Component
			//TInlineComponentArray<UMeshComponent*> MeshComps(self);
			//for (const auto& MeshComp : MeshComps) {
			//	auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
			//	if (StaticMeshProxy) {
			//		StaticMeshProxy->SetInstanced(false);
			//		StaticMeshProxy->SetInstanced(true);
			//	}
			//	else {
			//		auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
			//		if (ColoredMeshProxy) {
			//			ColoredMeshProxy->mBlockInstancing = !false;
			//			ColoredMeshProxy->SetInstanced(false);
			//			ColoredMeshProxy->mBlockInstancing = !true;
			//			ColoredMeshProxy->SetInstanced(true);
			//		}
			//		else {
			//			auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
			//			if (ProdIndInst) {
			//				ProdIndInst->SetInstanced(false);
			//				ProdIndInst->SetInstanced(true);
			//			}
			//		}
			//	}
			//	//SetMeshInstanced(MeshComp, false); // false = before swap, true = after restore
			//}
		//});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::ScrollRotate, bhgCDO, [](auto scope, AFGBuildableHologram* self, int32 delta, int32 step)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("AFGBuildableHologram::ScrollRotate"));
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	////	});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::SetHologramLocationAndRotation, bhgCDO, [](auto scope, AFGBuildableHologram* self, const FHitResult& hitResult)
	//	{
	//		//scope.Cancel();
	//		//self->mScrollRotation = self->mScrollRotation - 9;
	//		//float rotfloat = (float)self->mScrollRotation;
	//		//UE_LOG(CDO_Log, Display, TEXT("AFGBuildableHologram::SetHologramLocationAndRotation: %f"), rotfloat);
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::SetHologramLocationAndRotation, bhgCDO, [](AFGBuildableHologram* self, const FHitResult& hitResult)
	//	{
	//		//scope.Cancel();
	//		//float rotfloat = (float)self->mScrollRotation;
	//		//UE_LOG(CDO_Log, Display, TEXT("AFGBuildableHologram::SetHologramLocationAndRotation_After: %f"), rotfloat);
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//	});
	//AFGBeamHologram* bhCDO = GetMutableDefault<AFGBeamHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGBeamHologram::SetHologramLocationAndRotation, bhCDO, [](auto scope, AFGBeamHologram* self, const FHitResult& hitResult)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("SetHologramLocationAndRotation"));
	//	});
	//UFGBuildGunStateBuild* bgbCDO = GetMutableDefault<UFGBuildGunStateBuild>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::ScrollDown_Implementation, bgbCDO, [](auto scope, UFGBuildGunStateBuild* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("UFGBuildGunStateBuild::ScrollDown_Implementation"));
	//		//scope.Override(1);
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::ScrollUp_Implementation, bgbCDO, [](auto scope, UFGBuildGunStateBuild* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("UFGBuildGunStateBuild::ScrollUp_Implementation"));
	//		//scope.Override(1);
	//	});

	//SUBSCRIBE_METHOD(AFGBuildGun::Server_ScrollDown, [](auto scope, AFGBuildGun* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("AFGBuildGun::Server_ScrollDown"));
	//		//scope.Override(1);
	//	});
	//SUBSCRIBE_METHOD(AFGBuildGun::Server_ScrollUp, [](auto scope, AFGBuildGun* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("AFGBuildGun::Server_ScrollUp"));
	//		//scope.Override(1);
	//	});
	//SUBSCRIBE_METHOD(UFGBuildGunState::ScrollUp, [](auto scope, UFGBuildGunState* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("UFGBuildGunState::ScrollUp"));
	//		//scope.Override(1);
	//	});
	//SUBSCRIBE_METHOD(UFGBuildGunState::ScrollDown, [](auto scope, UFGBuildGunState* self)
	//	{
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("UFGBuildGunState::ScrollDown"));
	//		//scope.Override(1);
	//	});

	//AFGPillarHologram* pillarCDO = GetMutableDefault<AFGPillarHologram>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGPillarHologram::SetHologramLocationAndRotation, pillarCDO, [](auto scope, AFGPillarHologram* self, const FHitResult& hitResult)
	//	{
	//		//FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		scope.Cancel();
	//		UE_LOG(CDO_Log, Display, TEXT("AFGPillarHologram::SetHologramLocationAndRotation"));
	//		//scope.Override(1);
	//	});

	//	bool SetActorRotation(FRotator NewRotation, ETeleportType Teleport = ETeleportType::None);
	//SUBSCRIBE_METHOD(AActor::SetActorRotation, [](auto scope, AActor* self, FRotator NewRotation, ETeleportType Teleport)
	//	{
	//		UE_LOG(CDO_Log, Display, TEXT("AActor::SetActorRotation(FRotator NewRotation, ETeleportType Teleport = ETeleportType::None)"));
	//		FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		//scope.Override(1);
	//	});
	////bool SetActorRotation(const FQuat& NewRotation, ETeleportType Teleport = ETeleportType::None);
	//SUBSCRIBE_METHOD(AActor::SetActorRotation, [](auto scope, AActor* self, const FQuat& NewRotation, ETeleportType Teleport)
	//	{
	//		UE_LOG(CDO_Log, Display, TEXT("AActor::SetActorRotation(const FQuat& NewRotation, ETeleportType Teleport = ETeleportType::None)"));
	//		FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		//scope.Override(1);
	//	});
	//bool K2_SetActorRotation(FRotator NewRotation, bool bTeleportPhysics);
	//SUBSCRIBE_METHOD(AActor::K2_SetActorRotation, [](auto scope, AActor* self, FRotator NewRotation, bool bTeleportPhysics)
	//	{
	//		UE_LOG(CDO_Log, Display, TEXT("AActor::K2_SetActorRotation"));
	//		FDebug::DumpStackTraceToLog(ELogVerbosity::Display);
	//		//scope.Override(1);
	//	});
//#endif
	//
	//UFGHardDriveSettings* hdsCDO = GetMutableDefault<UFGHardDriveSettings>();
	//hdsCDO->mUniqueItemCount = 6;
	//SUBSCRIBE_METHOD_VIRTUAL(AFGCreatureSpawner::BeginPlay, csCDO, [](auto scope, AFGCreatureSpawner* self)
	//	{
	//		FStringClassReference HogClassReference = FStringClassReference(TEXT("/Game/FactoryGame/Character/Creature/Enemy/Hog/Char_Hog.Char_Hog"));
	//		if (UClass* RecipeClass = HogClassReference.TryLoadClass<AFGEnemy>())
	//		{
	//			self->mCreatureClass = HogClassReference.TryLoadClass<AFGEnemy>()->GetClass();
	//		}
	//		//if (self->mCreatureClass->GetName().Contains("stinger"))
	//		//{
	//
	//			//self->mCreatureClass = HogClassReference.TryLoadClass<AFGEnemy>();
	//		//}

	//	});
	// UFUNCTION( BlueprintCallable, Category = "Vehicle" )
	//void SetMovementComponent(UWheeledVehicleMovementComponent * movementComponent);
	//UFGWheeledVehicleMovementComponent4W* fourWCDO = GetMutableDefault<UFGWheeledVehicleMovementComponent4W>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGWheeledVehicleMovementComponent4W::GenerateTireForces, fourWCDO, [](auto& scope, UFGWheeledVehicleMovementComponent4W* self, class UVehicleWheel* Wheel, const FTireShaderInput& Input, FTireShaderOutput& Output) {

	//	});
	// 	void PrecacheObjects();
	//AFGResourceScanner* rsCDO = GetMutableDefault<AFGResourceScanner>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGResourceScanner::OnScanPressed, rsCDO, [](auto& scope, AFGResourceScanner* self) {
	//	//self->mNodeClusters.Reset();
	//	 self->GenerateNodeClusters();
	// });
	//SUBSCRIBE_METHOD(AFGWheeledVehicle::SetMovementComponent, [](auto& scope, AFGWheeledVehicle* self, UWheeledVehicleMovementComponent* movementComponent) {
		//self->mMaxAssistedAcceleration = self->mMaxAssistedAcceleration * 10;
		/*UFGWheeledVehicleMovementComponent4W* four = Cast< UFGWheeledVehicleMovementComponent4W>(movementComponent);
		if (four)
		{
			UE_LOG(CDO_Log, Display, TEXT("four"));
			UE_LOG(CDO_Log, Display, TEXT("mDSOLMaxSlopeAngleLimit = %f"), four->mDSOLMaxSlopeAngleLimit);
			UE_LOG(CDO_Log, Display, TEXT("mSlopeShiftRatio = %f"), four->mSlopeShiftRatio);
			UE_LOG(CDO_Log, Display, TEXT("MaxEngineRPM = %f"), four->MaxEngineRPM);
			four->MaxEngineRPM = four->MaxEngineRPM * 10;
			UE_LOG(CDO_Log, Display, TEXT("MaxEngineRPM = %f"), four->MaxEngineRPM);
		}
		UFGWheeledVehicleMovementComponent6W* six = Cast< UFGWheeledVehicleMovementComponent6W>(movementComponent);
		if (six)
		{
			UE_LOG(CDO_Log, Display, TEXT("mDSOLMaxSlopeAngleLimit = %f"), six->mDSOLMaxSlopeAngleLimit);
			UE_LOG(CDO_Log, Display, TEXT("mSlopeShiftRatio = %f"), six->mSlopeShiftRatio);
			UE_LOG(CDO_Log, Display, TEXT("mDownShiftLatency = %f"), six->mDownShiftLatency);
			UE_LOG(CDO_Log, Display, TEXT("mUseDSOLGearbox = %s"), six->mUseDSOLGearbox ? "true" : "false");
			six->mSlopeShiftRatio = 1;
		}*/
		//});

//#endif

}

void FCDOModule::SetMeshInstanced(UMeshComponent* MeshComp, bool Instanced)
{
	auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
	if (StaticMeshProxy) {
		StaticMeshProxy->SetInstanced(Instanced);
	}
	else {
		auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
		if (ColoredMeshProxy) {
			ColoredMeshProxy->mBlockInstancing = !Instanced;
			ColoredMeshProxy->SetInstanced(Instanced);
		}
		else {
			auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
			if (ProdIndInst) {
				ProdIndInst->SetInstanced(Instanced);
			}
		}
	}
}

IMPLEMENT_GAME_MODULE(FCDOModule, CDO);