#pragma once

#include "Modules/ModuleManager.h"
#include "FGVehicleSubsystem.h"
#include "WheeledVehicles/FGWheeledVehicle.h"
#include "WheeledVehicles/FGWheeledVehicleInfo.h"

DECLARE_LOG_CATEGORY_EXTERN(UniversalVehiclePaths_Log, Display, All);
class FUniversalVehiclePathsModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void FindSavedPaths(AFGVehicleSubsystem* self, const AFGWheeledVehicleInfo* vehicle);
};