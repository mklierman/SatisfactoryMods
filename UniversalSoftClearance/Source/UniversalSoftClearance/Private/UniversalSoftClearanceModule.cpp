#include "UniversalSoftClearanceModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGElevatorHologram.h"

#pragma optimize("", off)
void FUniversalSoftClearanceModule::StartupModule() 
{
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();
	AFGElevatorHologram* eCDO = GetMutableDefault<AFGElevatorHologram>();
	AFGBuildableElevator* evCDO = GetMutableDefault<AFGBuildableElevator>();


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
			}
		});


	SUBSCRIBE_METHOD_AFTER(AFGElevatorHologram::UpdateClearance, [this](AFGElevatorHologram* self)
	{
		FFGClearanceData cd = self->mClearanceData;
		FFGClearanceData new_cd = cd;
		new_cd.Type = EClearanceType::CT_Soft;
		new_cd.SoftClearanceResponse = ESoftClearanceOverlapResponse::SCOR_Default;
		new_cd.ClearanceBox.IsValid = true;
		self->mClearanceData = new_cd;
	});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableElevator::GetChildClearanceOutlineActors_Implementation, evCDO, [this](const AFGBuildableElevator* self, TArray<AActor*>& out_actors)
		{
			GetChildClearanceOutlineActors(self, out_actors);
		});
#endif
}

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

void FUniversalSoftClearanceModule::GetChildClearanceOutlineActors(const AFGBuildableElevator* self, TArray<AActor*>& actors)
{
	for (auto actor : actors)
	{
		auto buildable = Cast<AFGBuildable>(actor);
		if (buildable)
		{
			TArray<FFGClearanceData> out_data;
			TArray<FFGClearanceData> new_data;
			buildable->GetClearanceData_Implementation(out_data);
			for (auto data : out_data)
			{
				data.Type = EClearanceType::CT_Soft;
				data.SoftClearanceResponse = ESoftClearanceOverlapResponse::SCOR_Default;
				data.ClearanceBox.IsValid = true;
				new_data.Add(data);
			}
			buildable->mClearanceData = new_data;
		}
	}
}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FUniversalSoftClearanceModule, UniversalSoftClearance);