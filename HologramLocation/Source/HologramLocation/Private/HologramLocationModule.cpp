#include "HologramLocationModule.h"
#include "Patching/NativeHookManager.h"
#include "Misc/CoreDelegates.h"

DEFINE_LOG_CATEGORY(LocAndRot_Log);
void FHologramLocationModule::StartupModule() {
	TArray<FString> newLocs;
	newLocs.Add("../../../FactoryGame/Mods/HologramLocation/Localization");
	newLocs.Add("../../../FactoryGame/Mods/HologramLocation/Content/Localization");
	newLocs.Add("FactoryGame/Mods/HologramLocation/Localization");
	FCoreDelegates::GatherAdditionalLocResPathsCallback.AddRaw(this, &FHologramLocationModule::AddLocRes);
	//FCoreDelegates::GatherAdditionalLocResPathsCallback.Broadcast(newLocs);
}

void FHologramLocationModule::AddLocRes(TArray<FString> &newLocs)
{
	UE_LOG(LocAndRot_Log, Display, TEXT("AddLocRes"));
	newLocs.Add("../../../FactoryGame/Mods/HologramLocation/Localization");
	newLocs.Add("FactoryGame/Mods/HologramLocation/Localization");
	newLocs.Add("../../../FactoryGame/Mods/HologramLocation/Content/Localization");

	for (auto loc : newLocs)
	{
		UE_LOG(LocAndRot_Log, Display, TEXT("%s"), *loc);
	}

}


IMPLEMENT_GAME_MODULE(FHologramLocationModule, HologramLocation);