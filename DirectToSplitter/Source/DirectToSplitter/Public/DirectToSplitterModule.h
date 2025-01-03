#pragma once

#include "Modules/ModuleManager.h"
#include "Patching/NativeHookManager.h"
#include "FGFactoryConnectionComponent.h"
#include "Hologram/FGConveyorAttachmentHologram.h"
#include "Hologram/FGFactoryHologram.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildable.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGHologram.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "Buildables/FGBuildableSplitterSmart.h"
#include "Hologram/FGPipelineAttachmentHologram.h"
#include "Hologram/FGPipeAttachmentHologram.h"
#include "FGConstructDisqualifier.h"
#include "Buildables/FGBuildableConveyorLift.h"
#include "FGPipeConnectionComponent.h"
#include "Hologram/FGPipelineJunctionHologram.h"
#include "AbstractInstanceManager.h"
#include "AbstractInstanceInterface.h"
#include "InstanceData.h"
#include "AbstractInstance.h"
#include "SnapOnDisqualifier.h"

DECLARE_LOG_CATEGORY_EXTERN(SnapOn_Log, Display, All);

struct ConveyorAttachmentInfo
{
	UFGFactoryConnectionComponent* otherConnection;
	int8 connectionIndex;
	FVector SnappedLocation;
};

class FDirectToSplitterModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	void DismantleLeftoverBelt(UFGFactoryConnectionComponent* conn);
	void HandleLeftoverBelts(AFGBuildable* conveyorAttachment);
	void HandleExistingSnappedOn(AFGBuildable* conveyorAttachment);
	static bool IsSnappedOn(UFGFactoryConnectionComponent* Connection);
	static void TrySnapToActor_Hook(TCallScope<bool(*)(AFGConveyorAttachmentHologram*, const FHitResult&)>& scope, AFGConveyorAttachmentHologram* self, const FHitResult& hitResult);
	bool TrySnapToActor(AFGConveyorAttachmentHologram* self, const FHitResult& hitResult);
	void CheckValidPlacement(AFGConveyorAttachmentHologram* self, bool& retflag);
	void CheckValidFactoryPlacement(AFGFactoryHologram* self, bool& retflag);
	void CheckValidPipePlacement(AFGPipelineAttachmentHologram* self, bool& retflag);

	void ConfigureComponents(const AFGConveyorAttachmentHologram* self,  bool& retflag);

	virtual bool IsGameModule() const override { return true; }

	void HGConstruct(AFGBuildableHologram* hg, AActor* buildable);

	bool PipeSnap(AFGPipeAttachmentHologram* self, const FHitResult& hitResult);


	TMap < AFGConveyorAttachmentHologram* ,ConveyorAttachmentInfo> AttachmentInfos;
	TMap< AFGConveyorAttachmentHologram*, UFGFactoryConnectionComponent*> mSnappedConnections;
};