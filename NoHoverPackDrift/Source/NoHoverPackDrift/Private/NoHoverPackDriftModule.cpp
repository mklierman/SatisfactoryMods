#include "NoHoverPackDriftModule.h"
#include "Equipment/FGHoverPack.h"
#include "Equipment/FGEquipment.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"

void FNoHoverPackDriftModule::StartupModule() {
	#if !WITH_EDITOR
	AFGHoverPack* hpcdo = GetMutableDefault<AFGHoverPack>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHoverPack::Tick, hpcdo, [](AFGHoverPack* self, float deltaTime)
		{
			if (self->GetCurrentHoverMode() == EHoverPackMode::HPM_Hover)
			{
				float vLength = self->GetInstigatorCharacter()->GetCharacterMovement()->Velocity.Size();
				if (vLength > 0.0 && vLength < 15.0)
				{
					self->GetInstigatorCharacter()->GetCharacterMovement()->Velocity = FVector(0, 0, 0);
				}
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FNoHoverPackDriftModule, NoHoverPackDrift);