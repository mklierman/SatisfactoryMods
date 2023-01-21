#pragma once

#include "Modules/ModuleManager.h"
#include "FGPipeNetwork.h"
#include "Resources/FGItemDescriptor.h"
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
#include "AbstractInstanceManager.h"
#include "FGPipeNetwork.h"

//USTRUCT()
//struct FPipeColorStruct
//{
//	AFGBuildable* buildable;
//	UClass* inSwatchClass;
//	FFactoryCustomizationData customizationData;
//};

//DECLARE_LOG_CATEGORY_EXTERN(PersistentPaintables_Log, Display, All);
class FPersistentPaintablesModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	UClass* wallSupportClass;
	UClass* wallHoleClass;
	UClass* floorHoleClass;
	UClass* swatchClass;

	//TArray<PipeColorStruct> PipesToColor;

	UFUNCTION()
		void UpdateColor(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);

	UFUNCTION()
		void StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);

	void ApplyColor(AFGBuildable* buildable, UClass* inSwatchClass, FFactoryCustomizationData customizationData);
};