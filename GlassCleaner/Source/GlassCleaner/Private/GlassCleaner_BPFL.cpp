


#include "GlassCleaner_BPFL.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

void UGlassCleaner_BPFL::GetAllSubclassesOfWithCDO(TSubclassOf<UObject> ParentClass, TArray<UObject*>& OutCDOs)
{
    if (!ParentClass) return;

    TArray<UClass*> Results;
    GetDerivedClasses(ParentClass, Results, true);

    for (UClass* Class : Results)
    {
        if (!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
        {
            OutCDOs.Add(Class->GetDefaultObject());
        }
    }
}

void UGlassCleaner_BPFL::GetAllSubclassesOf(TSubclassOf<UObject> ParentClass, TArray<UClass*>& OutClassess)
{
    if (!ParentClass) return;

    GetDerivedClasses(ParentClass, OutClassess, true);
}

UObject* UGlassCleaner_BPFL::GetCDOQuiet(UClass* Class)
{
    return Class->GetDefaultObject();
}
