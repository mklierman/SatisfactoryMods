#pragma once

#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGVehicleSubsystem.h"
#include "SSP_Subsystem.h"

class FShowSplinePathModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	void ShowPathSpline(AFGDrivingTargetList* targetList, ASSP_Subsystem* subsystem);
};