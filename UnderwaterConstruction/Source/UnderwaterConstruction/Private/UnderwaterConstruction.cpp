// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnderwaterConstruction.h"
#include "FGCharacterPlayer.h"
#include "Patching/NativeHookManager.h"

#define LOCTEXT_NAMESPACE "FUnderwaterConstructionModule"

void FUnderwaterConstructionModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGCharacterPlayer::CanEquipBuildGun, [this](auto& scope, const AFGCharacterPlayer* self)
		{
			auto moveComp = self->GetCharacterMovement();
			if (moveComp->MovementMode == EMovementMode::MOVE_Swimming)
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD(AFGCharacterPlayer::CanEquipBuildGunForDismantle, [this](auto& scope, const AFGCharacterPlayer* self)
		{
			auto moveComp = self->GetCharacterMovement();
			if (moveComp->MovementMode == EMovementMode::MOVE_Swimming)
			{
				scope.Override(true);
			}
		});
	SUBSCRIBE_METHOD(AFGCharacterPlayer::CanEquipBuildGunForPaint, [this](auto& scope, const AFGCharacterPlayer* self)
		{
			auto moveComp = self->GetCharacterMovement();
			if (moveComp->MovementMode == EMovementMode::MOVE_Swimming)
			{
				scope.Override(true);
			}
		});
#endif
}

void FUnderwaterConstructionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUnderwaterConstructionModule, UnderwaterConstruction)