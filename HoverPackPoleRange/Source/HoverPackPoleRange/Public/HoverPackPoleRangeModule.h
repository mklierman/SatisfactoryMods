#pragma once

#include "Equipment/FGHoverPack.h"
#include "Modules/ModuleManager.h"
#include "HPPR_Subsystem.h"
#include "FGPowerConnectionComponent.h"
#include "Equipment/FGHoverPack.h"
#include "Patching/NativeHookManager.h"
#include "FGCircuitConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildablePowerPole.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "HPPR_ConfigStruct.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Configuration/ConfigProperty.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Subsystem/SubsystemActorManager.h"
#include "HPPR_Subsystem.h"
#include "FGGameMode.h"
#include "AbstractInstanceManager.h"
#include "AbstractInstanceInterface.h"
#include "InstanceData.h"
#include "AbstractInstance.h"
#include "Buildables/FGBuildableRailroadTrack.h"

DECLARE_LOG_CATEGORY_EXTERN(HoverPackPoleRange_Log, Display, All);
class FHoverPackPoleRangeModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual bool IsGameModule() const override { return true; }

	UPROPERTY()
	int32 mMk1Range;
	UPROPERTY()
	int32 mMk2Range;
	UPROPERTY()
	int32 mMk3Range;
	UPROPERTY()
	int32 mRailRange;
	UPROPERTY()
	int32 mElseRange;

	UPROPERTY()
	TMap<AFGHoverPack*, bool> mAddedConnections;
	UPROPERTY()
	TMap<AFGHoverPack*, bool> mAllowRemove;

	UPROPERTY()
	AHPPR_Subsystem* mHPSubsystem;


	void Loggit(FString myString);

	void FindNearestConnection(AFGHoverPack* self);

	UFGPowerConnectionComponent* GetBuildablePowerConnectionComponent(AFGBuildable* hitBuildable);

	UPROPERTY()
		bool debugLogging = false;
};