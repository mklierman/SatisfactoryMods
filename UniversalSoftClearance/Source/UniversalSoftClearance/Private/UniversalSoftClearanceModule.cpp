#include "UniversalSoftClearanceModule.h"
#include "Patching/NativeHookManager.h"

#pragma optimize("", off)
void FUniversalSoftClearanceModule::StartupModule() {
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();

	//SUBSCRIBE_METHOD_VIRTUAL(AFGBuildable::BeginPlay, bCDO, [this](auto scope, AFGBuildable* self)
	//{
	//		BeginPlay(self);
	//});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [this](auto scope, AFGHologram* self)
	//	{
	//		self->mClearanceDetector = nullptr;
	//		//HBeginPlay(self);
	//	});
	////EClearanceOverlapResult TestClearanceOverlap(const FFGClearanceData & clearanceData, const FFGClearanceData & otherClearanceData, AActor * otherActor) const;
	//SUBSCRIBE_METHOD(AFGHologram::TestClearanceOverlap, [this](auto scope, const AFGHologram* self, const FFGClearanceData& clearanceData, const FFGClearanceData& otherClearanceData, AActor* otherActor)
	//	{
	//		auto bld = Cast<AFGBuildable>(otherActor);
	//		if (bld)
	//		{
	//			for (auto cd : bld->mClearanceData)
	//			{
	//				cd.Type = EClearanceType::CT_Soft;
	//			}
	//		}
	//		auto hg = const_cast<AFGHologram*>(self);
	//		if (hg)
	//		{
	//			for (auto cdh : hg->mClearanceData)
	//			{
	//				cdh.Type = EClearanceType::CT_Soft;
	//			}
	//		}
	//		scope.Override(EClearanceOverlapResult::COR_Soft);
	//	});

#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGHologram::SetupClearanceDetector, [this](auto scope, AFGHologram* self)
		{
			auto cd = self->mClearanceData;
			if (cd.Num() > 0)
			{
				TArray<FFGClearanceData> SavedCD;
				for (auto cdata : cd)
				{
					FFGClearanceData newData = cdata;
					newData.Type = EClearanceType::CT_Soft;
					SavedCD.Add(newData);
				}

				self->mClearanceData.Empty();
				for (auto newcdata : SavedCD)
				{
					self->mClearanceData.Add(newcdata);
				}

				//auto cd1 = cd[0];
				//self->mClearanceData.Empty();
				//cd1.Type = EClearanceType::CT_Soft;
				//self->mClearanceData.Add(cd1);
			}

		});
	// 
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [this](auto scope, AFGHologram* self)
	//	{
	//		auto cd = self->mClearanceData;
	//		if (cd.Num() > 0)
	//		{
	//			auto cd1 = cd[0];
	//			self->mClearanceData.Empty();
	//			cd1.Type = EClearanceType::CT_Soft;
	//			self->mClearanceData.Add(cd1);
	//		}
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::GetClearanceData, hCDO, [this](auto scope, const AFGHologram* self, TArray< const FFGClearanceData* >& out_ClearanceData)
	//	{
	//		auto cd = self->mClearanceData;
	//		for (FFGClearanceData c : cd)
	//		{
	//			c.Type = EClearanceType::CT_Soft;
	//		}
	//	});

//
//	SUBSCRIBE_METHOD_AFTER(AFGBlueprintHologram::GenerateCollisionObjects, [this](AFGBlueprintHologram* self, const TArray< AFGBuildable* >& buildables)
//		{
//			GenerateCollisionObjects(self, buildables);
//		});
#endif
}

//void FUniversalSoftClearanceModule::SetupClearance(AFGHologram* self)
//{
//	auto newthing = self;
//	self->mClearanceData
//	//clearanceComponent->mIsSoftClearance = true;
//}
void FUniversalSoftClearanceModule::GenerateCollisionObjects(AFGBlueprintHologram* self, const TArray<AFGBuildable*>& buildables)
{
	//if (self && self->mClearanceComponents.Num() > 0)
	//{
	//	for (auto cc : self->mClearanceComponents)
	//	{
	//		cc->mIsSoftClearance = true;
	//	}
	//}
}
void FUniversalSoftClearanceModule::BeginPlay(AFGBuildable* self)
{
	auto cd = self->mClearanceData;
	for (FFGClearanceData c : cd)
	{
		c.Type = EClearanceType::CT_Soft;
	}
}
void FUniversalSoftClearanceModule::HBeginPlay(AFGHologram* self)
{

	TArray<UClass*> Subclasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if ((It->IsChildOf(AFGBuildable::StaticClass()) || It->IsChildOf(AFGHologram::StaticClass())) && !It->HasAnyClassFlags(CLASS_Abstract))
		{
			Subclasses.Add(*It);
		}
	}

	for (auto subclass : Subclasses)
	{
		auto cdo = subclass->GetDefaultObject();
		auto hg = Cast<AFGBuildable>(cdo);
		if (hg)
		{
			if (hg->mClearanceData.Num() > 0)
			{
				for (auto cd : hg->mClearanceData)
				{
					cd.Type = EClearanceType::CT_Soft;
				}
			}
			else
			{
				FFGClearanceData ncd;
				ncd.Type = EClearanceType::CT_Soft;
				hg->mClearanceData.Add(ncd);
			}
			//buildableClass->GetClearanceData(clearanceData);
		}
		else
		{
			auto holo = Cast<AFGHologram>(cdo);
			if (holo)
			{
				if (holo->mClearanceData.Num() > 0)
				{
					for (auto cd : holo->mClearanceData)
					{
						cd.Type = EClearanceType::CT_Soft;
					}
				}
				else
				{
					FFGClearanceData ncd;
					ncd.Type = EClearanceType::CT_Soft;
					holo->mClearanceData.Add(ncd);
				}
			}
		}
	}

}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FUniversalSoftClearanceModule, UniversalSoftClearance);