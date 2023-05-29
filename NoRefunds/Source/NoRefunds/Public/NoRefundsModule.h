#pragma once

#include "Modules/ModuleManager.h"
#include "FGInventoryComponent.h"

class FNoRefundsModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	TArray<FInventoryStack> AllowDismantleRefund(TArray< FInventoryStack > refund);
};