#pragma once

#include "Modules/ModuleManager.h"
#include "FGPipeNetwork.h"
#include "Resources/FGItemDescriptor.h"

DECLARE_LOG_CATEGORY_EXTERN(PersistentPaintables_Log, Display, All);
class FPersistentPaintablesModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	UClass* wallSupportClass;
	UClass* wallHoleClass;
	UClass* floorHoleClass;
	UClass* swatchClass;

	UFUNCTION()
		void UpdateColor(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);

	UFUNCTION()
		void StartTimer(AFGPipeNetwork* self, TSubclassOf< UFGItemDescriptor > descriptor);
};