// Copyright Epic Games, Inc. All Rights Reserved.

#include "SinkWaste.h"
#include <FGResourceSinkSubsystem.h>
#include "Patching/NativeHookManager.h"
#include <FGResourceSinkSettings.h>
#include "Resources/FGItemDescriptor.h"
#include <Logging/StructuredLog.h>

#define LOCTEXT_NAMESPACE "FSinkWasteModule"

void FSinkWasteModule::StartupModule()
{
	//auto sinkSettings = GetMutableDefault<UFGResourceSinkSettings>();
	//sinkSettings->mExcludedDescriptors.Empty();
	// int32 GetResourceSinkPointsForItem( TSubclassOf< class UFGItemDescriptor > itemDescriptor );
	//SUBSCRIBE_METHOD(AFGResourceSinkSubsystem::GetResourceSinkPointsForItem, [this](auto& scope, AFGResourceSinkSubsystem* self, TSubclassOf< class UFGItemDescriptor > itemDescriptor)
	//	{
	//		//UFGResourceSinkSettings* rss = GetMutableDefault<UFGResourceSinkSettings>();
	//		//rss->mExcludedDescriptors.Empty();
	//		scope.Override(-1000);
	//	});

#if !WITH_EDITOR

	//SUBSCRIBE_METHOD(AFGResourceSinkSubsystem::FindResourceSinkPointsForItem, [this](auto& scope, AFGResourceSinkSubsystem* self, TSubclassOf< class UFGItemDescriptor > itemDescriptor, int32& out_numPoints, EResourceSinkTrack& out_itemTrack)
	//	{
	//		//UFGResourceSinkSettings* rss = GetMutableDefault<UFGResourceSinkSettings>();
	//		//rss->mExcludedDescriptors.Empty();
	//		out_numPoints = -10000;
	//		out_itemTrack = EResourceSinkTrack::RST_Default;
	//		scope.Override(true);
	//	});
	SUBSCRIBE_METHOD(AFGResourceSinkSubsystem::AddPoints_ThreadSafe, [this](auto& scope, AFGResourceSinkSubsystem* self, TSubclassOf< class UFGItemDescriptor > item)
		{
			//FResourceSinkValuePair32 rsvp;
			//rsvp.TrackType = EResourceSinkTrack::RST_Default;
			//rsvp.Value = -100;
			//self->mQueuedPoints.Enqueue(rsvp);
			//UE_LOGFMT(LogScript, Error, "item->GetName(): {0}", item->GetName());
			if (item->GetName() == "Desc_NuclearWaste_C")
			{
				self->mTotalPoints[0] -= 500;
			}
			else if (item->GetName() == "Desc_PlutoniumWaste_C")
			{
				self->mTotalPoints[0] -= 1000;
			}
			//self->mCurrentPointLevels[0] -= 100;
			//scope.Override(true);
		});
#endif
}

void FSinkWasteModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSinkWasteModule, SinkWaste)