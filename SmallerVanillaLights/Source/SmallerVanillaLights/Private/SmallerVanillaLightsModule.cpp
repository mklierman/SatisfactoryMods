#include "SmallerVanillaLightsModule.h"
#include "Patching/NativeHookManager.h"
#include "FGRecipe.h"
#include "Hologram/FGHologram.h"

void FSmallerVanillaLightsModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGHologram::SpawnHologramFromRecipe, [](AFGHologram* self, TSubclassOf< class UFGRecipe > inRecipe, AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
		auto recipeName = UFGRecipe::GetRecipeName(inRecipe);
		if (recipeName.ToString().Contains("SmallerVanillaLights"))
		{
			self->SetActorRelativeScale3D(FVector(.5, .5, .5));
		}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FSmallerVanillaLightsModule, SmallerVanillaLights);