#include "UninterruptedTrainBeltsModule.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Buildables/FGBuildableTrainPlatform.h"

void FUninterruptedTrainBeltsModule::StartupModule() {
#if !WITH_EDITOR
	AFGBuildableTrainPlatformCargo* tpc = GetMutableDefault<AFGBuildableTrainPlatformCargo>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableTrainPlatformCargo::Factory_GrabOutput_Implementation, tpc, [=](auto& scope, AFGBuildableTrainPlatformCargo* self, class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type)
		{
			auto currentStatus = self->mPlatformDockingStatus;
			self->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self, connection, out_item, out_OffsetBeyond, type);
			self->mPlatformDockingStatus = currentStatus;
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableTrainPlatformCargo::SetIsInLoadMode, tpc, [=](auto& scope, AFGBuildableTrainPlatformCargo* self, bool isInLoadMode)
		{
			AFGBuildableTrainPlatformCargo* platform = const_cast<AFGBuildableTrainPlatformCargo*>(self);
			auto currentStatus = platform->mPlatformDockingStatus;
			platform->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self, isInLoadMode);
			platform->mPlatformDockingStatus = currentStatus;
		});
	SUBSCRIBE_METHOD(AFGBuildableTrainPlatformCargo::OnBeginLoadSequence, [=](auto& scope, AFGBuildableTrainPlatformCargo* self)
		{
			//AFGBuildableTrainPlatformCargo* platform = const_cast<AFGBuildableTrainPlatformCargo*>(self);
			AFGBuildableTrainPlatformCargo* platform = self;
			auto currentStatus = platform->mPlatformDockingStatus;
			platform->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self);
			platform->mPlatformDockingStatus = currentStatus;
		});
	SUBSCRIBE_METHOD(AFGBuildableTrainPlatformCargo::OnBeginUnloadSequence, [=](auto& scope, AFGBuildableTrainPlatformCargo* self)
		{
			//AFGBuildableTrainPlatformCargo* platform = const_cast<AFGBuildableTrainPlatformCargo*>(self);
			AFGBuildableTrainPlatformCargo* platform = self;
			auto currentStatus = platform->mPlatformDockingStatus;
			platform->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self);
			platform->mPlatformDockingStatus = currentStatus;
		});
#endif
}


IMPLEMENT_GAME_MODULE(FUninterruptedTrainBeltsModule, UninterruptedTrainBelts);