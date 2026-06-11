// Copyright Epic Games, Inc. All Rights Reserved.

#include "SetBlueprintOrigin.h"
#include "Patching/NativeHookManager.h"
#include "FGBlueprintSubsystem.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "SetBlueprintOrigin_Subsytem.h"
#include "Subsystem/SubsystemActorManager.h"

#define LOCTEXT_NAMESPACE "FSetBlueprintOriginModule"

DEFINE_LOG_CATEGORY(SetBlueprintOrigin_Log);
void FSetBlueprintOriginModule::StartupModule()
{
	//FBlueprintHeader WriteBlueprintToArchive(const FBlueprintRecord& record, const FTransform& blueprintOrigin, const TArray< class AFGBuildable* >& buildables, FIntVector dimensions );

	SUBSCRIBE_METHOD(AFGBlueprintSubsystem::WriteBlueprintToArchive, [this](auto& scope, AFGBlueprintSubsystem* self, const FBlueprintRecord& record, const FTransform& blueprintOrigin, const TArray< class AFGBuildable* >& buildables, FIntVector dimensions)
		{
			auto subsystem = self->GetWorld()->GetSubsystem<USubsystemActorManager>()->GetSubsystemActor<ASetBlueprintOrigin_Subsytem>();
			if (subsystem && subsystem->ShouldUseHighest)
			{
				//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "subsystem && subsystem->ShouldUseHighest");
				float highestZ = -999999999.f;
				for (auto buildable : buildables)
				{
					if (buildable)
					{
						//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "name: {0}", buildable->GetName());
						//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "class: {0}", buildable->GetClass());
						auto loc = buildable->GetActorLocation();
						//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "loc Z: {0}", loc.Z);
						if (loc.Z > highestZ)
						{

							highestZ = loc.Z;
							auto foundation = Cast<AFGBuildableFoundation>(buildable);
							auto foundationLw = Cast<AFGBuildableFoundationLightweight>(buildable);
							if (foundation || foundationLw)
							{
								//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "foundation");
								highestZ = highestZ - (foundation->mHeight / 2.0);
							}
							else if (auto attachment = Cast<AFGBuildableConveyorAttachment>(buildable))
							{
								//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "attachment");
								highestZ = highestZ - 100;
							}
						}
					}
				}
				//UE_LOGFMT(SetBlueprintOrigin_Log, Display, "highestZ: {0}", highestZ);

				if (highestZ != -999999999.f)
				{
					FTransform newTransform = blueprintOrigin;
					FVector newLoc = newTransform.GetLocation();
					newLoc.Z = highestZ;
					newTransform.SetLocation(newLoc);
					scope(self, record, newTransform, buildables, dimensions);
				}
			}
		});
#if !WITH_EDITOR
#endif
}

void FSetBlueprintOriginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSetBlueprintOriginModule, SetBlueprintOrigin)