#include "UninterruptedTrainBeltsModule.h"
#include "Patching/NativeHookManager.h"

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
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableTrainPlatformCargo::Factory_CollectInput_Implementation, tpc, [=](auto& scope, AFGBuildableTrainPlatformCargo* self)
		{
			auto currentStatus = self->mPlatformDockingStatus;
			self->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self);
			self->mPlatformDockingStatus = currentStatus;
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableTrainPlatformCargo::Factory_PullPipeInput_Implementation, tpc, [=](auto& scope, AFGBuildableTrainPlatformCargo* self, float dt)
		{
			auto currentStatus = self->mPlatformDockingStatus;
			self->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self, dt);
			self->mPlatformDockingStatus = currentStatus;
		});
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableTrainPlatformCargo::Factory_PushPipeOutput_Implementation, tpc, [=](auto& scope, AFGBuildableTrainPlatformCargo* self, float dt)
		{
			auto currentStatus = self->mPlatformDockingStatus;
			self->mPlatformDockingStatus = ETrainPlatformDockingStatus::ETPDS_IdleWaitForTime;
			scope(self, dt);
			self->mPlatformDockingStatus = currentStatus;
		});
#endif
}


IMPLEMENT_GAME_MODULE(FUninterruptedTrainBeltsModule, UninterruptedTrainBelts);