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
#include "FGConstructDisqualifier.h"
#include "Buildables/FGBuildableConveyorLift.h"
#include "SnapOnDisqualifier.h"

struct ConveyorAttachmentInfo
{
	UFGFactoryConnectionComponent* otherConnection;
	int8 connectionIndex;
	FVector SnappedLocation;
};

class FDirectToSplitterModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	void CheckValidPlacement(AFGConveyorAttachmentHologram* self, bool& retflag);
	void CheckValidFactoryPlacement(AFGFactoryHologram* self, bool& retflag);
	void CheckValidPipePlacement(AFGPipelineAttachmentHologram* self, bool& retflag);

	void ConfigureComponents(const AFGConveyorAttachmentHologram* self,  bool& retflag);

	virtual bool IsGameModule() const override { return true; }

	void HGConstruct(AFGBuildableHologram* hg, AActor* buildable);

	TMap < AFGConveyorAttachmentHologram* ,ConveyorAttachmentInfo> AttachmentInfos;
	TMap< AFGConveyorAttachmentHologram*, UFGFactoryConnectionComponent*> mSnappedConnections;
};