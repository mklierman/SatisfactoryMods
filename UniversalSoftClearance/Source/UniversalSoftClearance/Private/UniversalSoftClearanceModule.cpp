#include "UniversalSoftClearanceModule.h"
#include "Patching/NativeHookManager.h"

//#pragma optimize("", off)
void FUniversalSoftClearanceModule::StartupModule() {
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();

#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::SetupClearance, hCDO, [=](auto scope, AFGHologram* self, class UFGClearanceComponent* clearanceComponent)
		{
			SetupClearance(self, clearanceComponent);
			scope(self, clearanceComponent);
		});


	SUBSCRIBE_METHOD_AFTER(AFGBlueprintHologram::GenerateCollisionObjects, [=](AFGBlueprintHologram* self, const TArray< AFGBuildable* >& buildables)
		{
			GenerateCollisionObjects(self, buildables);
		});
#endif
}

void FUniversalSoftClearanceModule::SetupClearance(AFGHologram* self, UFGClearanceComponent* clearanceComponent)
{
	auto newthing = self;
	clearanceComponent->mIsSoftClearance = true;
}
void FUniversalSoftClearanceModule::GenerateCollisionObjects(AFGBlueprintHologram* self, const TArray<AFGBuildable*>& buildables)
{
	if (self && self->mClearanceComponents.Num() > 0)
	{
		for (auto cc : self->mClearanceComponents)
		{
			cc->mIsSoftClearance = true;
		}
	}
}
//#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FUniversalSoftClearanceModule, UniversalSoftClearance);