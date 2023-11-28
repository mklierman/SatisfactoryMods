#include "NoHoverPackDriftModule.h"
#include "Equipment/FGHoverPack.h"
#include "Equipment/FGEquipment.h"
#include "Patching/NativeHookManager.h"
#include "NoHoverPackDrift_ConfigStruct.h"
#include "FGCharacterPlayer.h"

void FNoHoverPackDriftModule::StartupModule() {
	AFGHoverPack* hpcdo = GetMutableDefault<AFGHoverPack>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHoverPack::Tick, hpcdo, [=](AFGHoverPack* self, float deltaTime)
		{
			this->StopDrift(self, deltaTime);
		});
#endif
}

//#pragma optimize("", off)
void FNoHoverPackDriftModule::StopDrift(AFGHoverPack* self, float deltaTime)
{
	if (self->GetCurrentHoverMode() == EHoverPackMode::HPM_Hover)
	{
		float dTime = deltaTime;
		auto config = FNoHoverPackDrift_ConfigStruct::GetActiveConfig(self->GetWorld());
		float vLength = abs(self->GetInstigatorCharacter()->GetCharacterMovement()->Velocity.Size());
		if (vLength > 0.f && vLength < config.MinSpeed)
		{
			self->GetInstigatorCharacter()->GetCharacterMovement()->Velocity = FVector(0.f, 0.f, 0.f);
		}
	}
}
//#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FNoHoverPackDriftModule, NoHoverPackDrift);