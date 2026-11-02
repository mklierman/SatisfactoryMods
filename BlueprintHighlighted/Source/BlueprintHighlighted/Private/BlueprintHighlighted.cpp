#include "BlueprintHighlighted.h"

#include "BHConnectorBuildableLibrary.h"
#include "Engine/SCS_Node.h"
#include "Patching/BlueprintHookManager.h"
#include "UObject/UObjectIterator.h"

namespace
{
	// SML applies BH_Mixin_BuildGun by adding a transient SCS node (plus its component template and an input binding
	// wrapper) to BP_BuildGun_C after the class has loaded. Cooked Blueprint classes sit in GC clusters, and GC does not
	// see references added to a cluster afterwards, so it frees those objects and the saved build gun crashes in
	// USCS_Node::GetSCS when it is respawned on save load. Rooting them before every GC keeps them alive.
	void KeepMixinObjectsAlive()
	{
		for (TObjectIterator<USCS_Node> It; It; ++It)
		{
			USCS_Node* Node = *It;
			if (Node->HasAnyFlags(RF_Transient) && Node->ComponentClass == UBlueprintMixinHostComponent::StaticClass())
			{
				Node->AddToRoot();
				if (Node->ComponentTemplate)
				{
					Node->ComponentTemplate->AddToRoot();
				}
			}
		}
		for (TObjectIterator<UMixinInputDelegateBinding> It; It; ++It)
		{
			if (It->HasAnyFlags(RF_Transient))
			{
				It->AddToRoot();
			}
		}
	}
}

void FBlueprintHighlightedModule::StartupModule()
{
	UBHConnectorBuildableLibrary::InstallDismantleHighlightTracking();

	// The editor strips these nodes before saving assets, so only the game needs them kept alive.
	if (!GIsEditor)
	{
		FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddStatic(&KeepMixinObjectsAlive);
	}
}

IMPLEMENT_MODULE(FBlueprintHighlightedModule, BlueprintHighlighted)
