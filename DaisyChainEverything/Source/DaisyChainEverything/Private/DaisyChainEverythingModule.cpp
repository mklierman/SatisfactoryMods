#include "DaisyChainEverythingModule.h"
#include "Patching/NativeHookManager.h"
#include "Buildables/FGBuildable.h"
#include "FGPowerConnectionComponent.h"
#include "DCE_ConfigurationStruct.h"

DEFINE_LOG_CATEGORY(DaisyChainEverythingModule_Log);

void FDaisyChainEverythingModule::StartupModule() {
#if !WITH_EDITOR
	AFGBuildable* bCDO = GetMutableDefault<AFGBuildable>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildable::BeginPlay, bCDO, [](AFGBuildable* self)
		{
			auto config = FDCE_ConfigurationStruct::GetActiveConfig();
			UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("AFGBuildable::BeginPlay"));

			auto LessThan = config.MinSection.MinLessThanInt;
			auto SetTo = config.MinSection.MinSetToInt;
			auto GlobalNum = config.GlobalSection.GlobalSetToInt;
			auto AddInt = config.AddSection.AddDefaultInt;
			TInlineComponentArray<UFGPowerConnectionComponent*> powerConns(self);
			for (int i = 0; i < powerConns.Num(); i++)
			{
				switch (config.ModesEnum)
				{
				case(0): //Set Minimum Connection Amount
					if (powerConns[i]->mMaxNumConnectionLinks < LessThan)
					{
						powerConns[i]->mMaxNumConnectionLinks = SetTo;
					}
					break;
				case(1): //Set Global Connection Amount
					powerConns[i]->mMaxNumConnectionLinks = GlobalNum;
					break;
				case(2): //Add Additional Connections
					auto cdo = self->GetClass()->GetDefaultObject();
					if (cdo)
					{
						UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("if (cdo)"));
						auto cdoA = Cast<AActor>(cdo);
						if (cdoA)
						{
							UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("if (cdoA)"));
							//auto dpc = cdoA->GetComponentsByClass(UFGPowerConnectionComponent::StaticClass());
							TInlineComponentArray<UFGPowerConnectionComponent*> defaultPowerConns(cdoA);
							if (defaultPowerConns.Num() == powerConns.Num())
							{
								UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("if (dpc.Num() == powerConns.Num())"));
								auto defaultConn = Cast<UFGPowerConnectionComponent>(defaultPowerConns[i]);
								if (defaultConn)
								{
									UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("if (defaultConn)"));
									powerConns[i]->mMaxNumConnectionLinks = defaultConn->mMaxNumConnectionLinks + AddInt;
								}
							}
						}
					}
					break;
				}
			}
			//for (const auto& PowerConn : powerConns)
			//{
			//	switch (config.ModesEnum)
			//	{
			//	case(0): //Set Minimum Connection Amount
			//		UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("case(0)"));
			//		if (PowerConn->mMaxNumConnectionLinks < LessThan)
			//		{
			//			PowerConn->mMaxNumConnectionLinks = SetTo;
			//		}
			//		break;
			//	case(1): //Set Global Connection Amount
			//		UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("case(1)"));
			//		PowerConn->mMaxNumConnectionLinks = GlobalNum;
			//		break;
			//	case(2): //Add Additional Connections
			//		UE_LOG(DaisyChainEverythingModule_Log, Display, TEXT("case(2)"));
			//		//auto cdo = Cast<UClass>(self)->GetDefaultObject();
			//		//Cast<AActor>(cdo)->GetCompo
			//		break;
			//	default:
			//		break;
			//	}
			//}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FDaisyChainEverythingModule, DaisyChainEverything);