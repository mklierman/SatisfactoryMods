#include "BlueprintHighlighted.h"

#include "BHConnectorBuildableLibrary.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DynamicBlueprintBinding.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Patching/BlueprintHookBlueprint.h"
#include "Patching/BlueprintHookManager.h"

namespace
{
	const TCHAR* BuildGunMixinClassPath = TEXT("/BlueprintHighlighted/BH_Mixin_BuildGun.BH_Mixin_BuildGun_C");

	FDelegateHandle PreGarbageCollectHandle;

	// SML applies BH_Mixin_BuildGun by adding a transient SCS node (plus its component template and an input binding
	// wrapper) to BP_BuildGun_C after the class has loaded. Cooked Blueprint classes sit in GC clusters, and GC does not
	// see references added to a cluster afterwards, so it frees those objects and the saved build gun crashes in
	// USCS_Node::GetSCS when it is respawned on save load. Rooting them keeps them alive.
	//
	// Only the build gun class is touched: it is found through our own mixin's MixinTargetClass, so no other class or
	// mod's mixins are walked. SML creates these objects once per class and reuses them, so rooting once is enough.
	// Returns false until SML has installed the mixin.
	bool RootBuildGunMixinObjects()
	{
		const UHookBlueprintGeneratedClass* MixinClass = FindObject<UHookBlueprintGeneratedClass>(nullptr, BuildGunMixinClassPath);
		UBlueprintGeneratedClass* BuildGunClass = MixinClass ? MixinClass->MixinTargetClass.Get() : nullptr;
		if (!BuildGunClass || !BuildGunClass->SimpleConstructionScript)
		{
			return false;
		}

		USCS_Node* HostNode = nullptr;
		for (USCS_Node* Node : BuildGunClass->SimpleConstructionScript->GetRootNodes())
		{
			if (Node && Node->HasAnyFlags(RF_Transient) && Node->ComponentClass == UBlueprintMixinHostComponent::StaticClass())
			{
				HostNode = Node;
				break;
			}
		}
		if (!HostNode)
		{
			return false;
		}

		HostNode->AddToRoot();
		if (HostNode->ComponentTemplate)
		{
			HostNode->ComponentTemplate->AddToRoot();
		}
		for (UDynamicBlueprintBinding* Binding : BuildGunClass->DynamicBindingObjects)
		{
			if (Binding && Binding->IsA<UMixinInputDelegateBinding>())
			{
				Binding->AddToRoot();
			}
		}
		return true;
	}

	void OnPreGarbageCollect()
	{
		// Runs before each GC until the objects are rooted, then unregisters itself. Every GC before that point is
		// a single FindObject lookup.
		if (RootBuildGunMixinObjects())
		{
			FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(PreGarbageCollectHandle);
			PreGarbageCollectHandle.Reset();
		}
	}
}

void FBlueprintHighlightedModule::StartupModule()
{
	UBHConnectorBuildableLibrary::InstallDismantleHighlightTracking();

	// The editor strips these nodes before saving assets, so only the game needs them kept alive.
	if (!GIsEditor)
	{
		PreGarbageCollectHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddStatic(&OnPreGarbageCollect);
	}
}

IMPLEMENT_MODULE(FBlueprintHighlightedModule, BlueprintHighlighted)
