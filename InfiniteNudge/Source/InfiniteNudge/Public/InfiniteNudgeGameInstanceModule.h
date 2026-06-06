#pragma once

#include <Module/GameInstanceModule.h>
#include <Components/TextBlock.h>
#include "Input/FGInputMappingContext.h"
#include "FGCharacterPlayer.h"
#include "Hologram/FGHologram.h"
#include "CoreMinimal.h"
#include "InfiniteNudgeGameInstanceModule.generated.h"


UCLASS(Blueprintable)
class INFINITENUDGE_API UInfiniteNudgeGameInstanceModule : public UGameInstanceModule {
    GENERATED_BODY()

protected:
    AFGHologram* GetHologram(); 
    bool IsHologramLocked();

    void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

    /* Map of vanilla hologram classes to the nudgeable ones that override CanNudgeHologram. Default values set in the BP */
    UPROPERTY(EditAnywhere)
    TMap<TSubclassOf<class AFGHologram>, TSubclassOf<class AFGHologram>> mHologramOverrides;

    //INPUT
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* mScrollModeInputAction;

	/* Callbacks for IA_InfiniteNudge_CycleMode.*/
    void ShowModeMenu(); 
    void CloseModeMenu(); 
    void CycleScrollMode();
   
    //MENU
    TArray<TFunction<void()>> mMenuActions = {};
    FTSTicker::FDelegateHandle mTickHandle;
    bool mMenuOpened = false;

    void ToggleLockPivot();
    void ToggleGizmo();
    void ResetTransform();
    bool Tick(float delta);

    /* Widget class, set in BP */
    UPROPERTY(EditAnywhere)
    TSubclassOf<UFGUserWidget> mModeMenuClass;
    /* Mapping context to capture clicks and prevent buildable placement while menu is open. Should be MC_QuickSwitch, set in BP. */
    UPROPERTY(EditAnywhere)
    UFGInputMappingContext* mModeMenuMappingContext;
    /* Reference to the widget */
    UPROPERTY()
    UFGUserWidget* mScrollModeMenu;

    

    //HINT EDIT

    /* Cached TextBlock of the hint for the current scroll mode */
    TWeakObjectPtr<UTextBlock> mHintTextBlock;
    /* Caches the hint TextBlock for the current scroll mode. Called from BP after Widget_BuildMode->SetKeybindingHints() */
    UFUNCTION(BlueprintCallable)
    void CacheHintTextBlock(UFGUserWidget* container);
    /* Updates the keybind hint based on current scroll mode.*/
    void UpdateHint();

    //DEBUG FUNCTION

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogWidgetHierarchy(UUserWidget* widget, int32 depth = 0);

    
    // RADIAL MENU


    //DELAY
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UInputTriggerPulse* mTriggerPulse;
    
public:
    UFUNCTION(BlueprintCallable)
    TArray<FText> GetMenuEntries(FText& outSelected);

    UFUNCTION(BlueprintCallable)
    void SelectMode(int index);
}; 