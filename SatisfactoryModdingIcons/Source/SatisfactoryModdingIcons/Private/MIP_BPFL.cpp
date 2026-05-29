


#include "MIP_BPFL.h"
#include "Engine/AssetManager.h"
#include "FGIconLibrary.h"

void UMIP_BPFL::RegisterIcons()
{
    UAssetManager& Manager = UAssetManager::Get();

    TArray<FString> Paths;
    Paths.Add(TEXT("/SatisfactoryModdingIcons")); // your plugin content path
    Paths.Add(TEXT("/SatisfactoryModdingIcons/Content")); // your plugin content path

    Manager.ScanPathsForPrimaryAssets(
        FPrimaryAssetType(TEXT("MIP_Icons")),  // the type name
        Paths,
        UFGIconLibrary::StaticClass(),     // base class to filter by
        false   // bHasBlueprintClasses
    );
}
