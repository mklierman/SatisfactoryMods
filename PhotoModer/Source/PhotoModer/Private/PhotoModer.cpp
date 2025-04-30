// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhotoModer.h"
#include <FGPhotoModeComponent.h>
#include <NativeHookManager.h>
#include "GameFramework/PlayerState.h"
#include "FGPlayerState.h"
#include "FGInputLibrary.h"

#define LOCTEXT_NAMESPACE "FPhotoModerModule"

void FPhotoModerModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGPhotoModeComponent::ExitPhotoMode, [this](auto& scope, UFGPhotoModeComponent* self)
		{
			AFGPlayerState* state = self->GetOuterAFGPlayerState();
			APlayerState* pState = Cast<APlayerState>(state);
			auto controller = pState->GetPlayerController();
			if (controller->IsInputKeyDown(EKeys::LeftShift) || controller->IsInputKeyDown(EKeys::RightShift))
			{
				if (self->GetIsDecoupledCameraOn())
				{
					self->ToggleDecoupledCamera();
				}
				self->SetPlayerVisibilityInPhotoMode(true);
				self->GetOwnerPlayerCharacter()->HandlePhotoModeOnCharacter(false);
				self->mIsPhotoModeOn = false;
				scope.Override(true);
			}
		});

	//SUBSCRIBE_METHOD(UFGPhotoModeComponent::EnterPhotoMode, [this](auto& scope, UFGPhotoModeComponent* self)
	//	{
	//		AFGPlayerState* state = self->GetOuterAFGPlayerState();
	//		APlayerState* pState = Cast<APlayerState>(state);
	//		auto controller = pState->GetPlayerController();
	//		if (controller->IsInputKeyDown(EKeys::Escape))
	//		{
	//			scope.Override(true);
	//		}
	//	});

	//SUBSCRIBE_METHOD(UFGPhotoModeComponent::TogglePhotoMode, [this](auto& scope, UFGPhotoModeComponent* self)
	//	{
	//		AFGPlayerState* state = self->GetOuterAFGPlayerState();
	//		APlayerState* pState = Cast<APlayerState>(state);
	//		auto controller = pState->GetPlayerController();
	//		if (controller->IsInputKeyDown(EKeys::Escape))
	//		{
	//			self->ExitPhotoMode();
	//		}
	//		if (self->GetIsPhotoModeOn() && controller->IsInputKeyDown(EKeys::LeftShift))
	//		{
	//			scope.Override(true);
	//		}
	//		else if (!controller->IsInputKeyDown(EKeys::Escape))
	//		{
	//			scope.Override(false);
	//		}
	//	});
#endif
}

void FPhotoModerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhotoModerModule, PhotoModer)