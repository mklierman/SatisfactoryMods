#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "TestHooker_Subsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TestHooker_Log, Display, All);

UCLASS()
class ATestHooker_Subsystem : public AModSubsystem
{
	GENERATED_BODY()
public:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FDelegateHandle EmoteHook;
	UFUNCTION(BlueprintCallable)
	void HookEmote();

	UFUNCTION(BlueprintCallable)
	void UnhookEmote();

	FDelegateHandle EmoteSFXHook;
	UFUNCTION(BlueprintCallable)
	void HookEmoteSFX();

	UFUNCTION(BlueprintCallable)
	void UnhookEmoteSFX();

	FDelegateHandle JumpHook;
	UFUNCTION(BlueprintCallable)
	void HookJump();

	UFUNCTION(BlueprintCallable)
	void UnhookJump();

	FDelegateHandle FlashlightHook;
	UFUNCTION(BlueprintCallable)
	void HookBPToggleFlashlight();

	UFUNCTION(BlueprintCallable)
	void UnHookBPToggleFlashlight();

	FDelegateHandle InventoryHook;
	UFUNCTION(BlueprintCallable)
	void HookBPToggleInventory();

	UFUNCTION(BlueprintCallable)
	void UnHookBPToggleInventory();

};

