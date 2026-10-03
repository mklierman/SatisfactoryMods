#include "BlueprintHighlighted.h"

#include "BHConnectorBuildableLibrary.h"

void FBlueprintHighlightedModule::StartupModule()
{
	UBHConnectorBuildableLibrary::InstallDismantleHighlightTracking();
}

IMPLEMENT_MODULE(FBlueprintHighlightedModule, BlueprintHighlighted)
