#include "ConstructionPreferencesModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGConveyorBeltHologram.h"
#include "Subsystem/SubsystemActorManager.h"
#include "CP_Subsystem.h"

void FConstructionPreferencesModule::StartupModule() {
#if !WITH_EDITOR
	AFGConveyorBeltHologram* CBH = GetMutableDefault<AFGConveyorBeltHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGConveyorBeltHologram::SpawnChildren, CBH, [](auto scope, AFGConveyorBeltHologram* self, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
		//UWorld* WorldObject = self->GetWorld();
		//USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		//ACP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACP_Subsystem>();
		//auto recipe = subsystem->mConveyorPoleRecipe;
		//self->SpawnChildHologramFromRecipe(self, recipe, hologramOwner, spawnLocation, hologramInstigator);
		scope.Cancel();
		});
#endif
}


IMPLEMENT_GAME_MODULE(FConstructionPreferencesModule, ConstructionPreferences);