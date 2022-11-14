#include "CP_BPFL.h"

//void CP_BPFL::ExampleFunction() {}

TArray<TSubclassOf<class AFGBuildableWire>> UCP_BPFL::GetPowerWireClasses()
{
	 TArray<TSubclassOf<class AFGBuildableWire>> Subclasses;
    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (It->IsChildOf(AFGBuildableWire::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
        {
            Subclasses.Add(*It);
        }
    }
    return Subclasses;
}
