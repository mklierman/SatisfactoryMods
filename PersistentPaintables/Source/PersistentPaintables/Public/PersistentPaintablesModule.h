#pragma once

#include "AbstractInstanceManager.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildablePipelineJunction.h"
//#include "FGBuildablePipelineSupport.h"
#include "FGFluidIntegrantInterface.h"
#include "FGPipeConnectionComponent.h"
#include "FGPipeNetwork.h"
#include "Hologram/FGBuildableHologram.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "PersistentPaintablesCppSubSystem.h"
#include "Resources/FGItemDescriptor.h"
#include "AbstractInstanceManager.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "PersistentPaintables_ConfigStruct.h"

//USTRUCT()
//struct FPipeColorStruct
//{
//	AFGBuildable* buildable;
//	UClass* inSwatchClass;
//	FFactoryCustomizationData customizationData;
//};

DECLARE_LOG_CATEGORY_EXTERN(PersistentPaintables_Log, Display, All);
class FPersistentPaintablesModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }

	UClass* wallSupportClass;
	UClass* wallHoleClass;
	UClass* floorHoleClass;
	UClass* pipeSupportClass;
	UClass* swatchClass;
	UClass* subsystemClass;

	FDelegateHandle ConstructHook;

	//TArray<PipeColorStruct> PipesToColor;

	UFUNCTION()
	void UpdateColor(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);

	void UpdateNetworkColor(AFGPipeNetwork* pipeNetwork);

	void UpdateColorSingle(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork);

	void ColorConnectedSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData);

	void ColorSupports(AFGBuildablePipeline* pipe, FFactoryCustomizationData& newData);

	bool IsPipeSupport(const AFGBuildable* buildable) const;

	bool IsSupportTouchingPipe(AFGBuildablePipeline* pipe, AFGBuildable* support) const;

	FVector GetSupportSnapLocation(AFGBuildable* support) const;

	void BuildSupportMap(AFGPipeNetwork* pipeNetwork);

	UFUNCTION()
	void StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);

	void ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData);

	void AddBuildable(class AFGBuildable* buildable, const AFGBuildableHologram* hologram);

	void HookPipes();

	void QueuePipeColorUpdate(AFGBuildable* buildable, AFGPipeNetwork* pipeNetwork);

	void FlushPipeColorUpdates();

	TArray<AActor*> FindNearbySupports(AFGBuildable* pipe, UFGPipeConnectionComponentBase* pipeConn);

	TArray<AActor*> PotentialSupports;
	TArray<TWeakObjectPtr<AActor>> WeakPotentialSupports;
	TMap<TWeakObjectPtr<AFGBuildablePipeline>, TArray<TWeakObjectPtr<AFGBuildable>>> SupportsByPipe;
	TMap<FIntVector, TArray<TWeakObjectPtr<AFGBuildable>>> SupportSpatialCells;
	TWeakObjectPtr<UWorld> SupportSpatialWorld;
	bool bSupportSpatialIndexDirty = true;
	TMap<TWeakObjectPtr<AFGBuildable>, TWeakObjectPtr<AFGPipeNetwork>> PendingPipeColorUpdates;
	bool bPipeColorUpdateScheduled = false;
	bool bApplyingColor = false;

	void AFGBuildableHologramConstruct(AFGBuildableHologram* self);
};
