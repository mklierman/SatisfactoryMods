#include "InfiniteNudgeGameInstanceModule.h" 
#include "FGCharacterPlayer.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include <Kismet/GameplayStatics.h>
#include "EnhancedInputSubsystems.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include <EnhancedInputComponent.h>
#include <Configuration/ConfigManager.h>

#include "InfiniteNudgeModule.h"
#include "InfiniteGizmo.h"

#pragma optimize("", off)

static const FName HOLOGRAM_MESH_TAG = FName("HologramMesh"); //Ideally AFGHologram::HOLOGRAM_MESH_TAG but it's protected.

void UInfiniteNudgeGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {

    Super::DispatchLifecycleEvent(Phase);

    if (Phase != ELifecyclePhase::POST_INITIALIZATION)
        return;

    //send the overrides to the main module.
    FInfiniteNudgeModule::mHologramOverrides = this->mHologramOverrides;

    //register the InputAction
    AFGCharacterPlayer::OnPlayerInputInitialized.AddLambda([this](AFGCharacterPlayer* character, UInputComponent* inputComponent)
        {
            if (auto* EIC = Cast<UEnhancedInputComponent>(inputComponent)) {
                //Triggered when pressed longer than the threshold in the IA.
                EIC->BindAction(mScrollModeInputAction, ETriggerEvent::Triggered, this, &UInfiniteNudgeGameInstanceModule::ShowModeMenu);
                //When released after Triggered occured.
                EIC->BindAction(mScrollModeInputAction, ETriggerEvent::Completed, this, &UInfiniteNudgeGameInstanceModule::CloseModeMenu);
                //When released before the threshold.
                EIC->BindAction(mScrollModeInputAction, ETriggerEvent::Canceled, this, &UInfiniteNudgeGameInstanceModule::CycleScrollMode);
            }
        });
}

//UTILITY FUNCTIONS
AFGHologram* UInfiniteNudgeGameInstanceModule::GetHologram() {
    auto* world = GetWorld();
    if (!world)
    {
        return nullptr;
    }
    auto* character = Cast<AFGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(world, 0));
    if (!character)
    {
        return nullptr;
    }
    auto* buildgun = character->GetBuildGun();
    if (!buildgun)
    {
        return nullptr;
    }
    auto* currentState = buildgun->GetCurrentState();
    if (!currentState)
    {
        return nullptr;
    }
    if (auto* buildState = Cast<UFGBuildGunStateBuild>(currentState))
    {
        return buildState->GetHologram();
    }
    return nullptr;
}

bool UInfiniteNudgeGameInstanceModule::IsHologramLocked() {
	auto* hologram = GetHologram();
    return hologram && hologram->IsHologramLocked();
}



//SCROLL MODE MENU
//Called from BP. Populates the radial menu.
TArray<FText> UInfiniteNudgeGameInstanceModule::GetMenuEntries(FText& outSelected) {
	auto* hologram = GetHologram();
	auto useGizmo = !hologram->mParent && hologram->mChildren.Num() <= 0;

	mMenuActions.Empty();
    	for (EScrollMode mode : TEnumRange<EScrollMode>())            
            mMenuActions.Add([this, mode]() { ScrollMode::Set(mode); this->UpdateHint(); });
        if(useGizmo)
            mMenuActions.Add([this]() { ToggleLockPivot(); });
        mMenuActions.Add([this]() { ResetTransform(); });
        if (useGizmo)
            mMenuActions.Add([this]() { ToggleGizmo(); });

    //pre-select current mode
	outSelected = ScrollMode::GetName();

	//order needs to match the callbacks in mMenuActions
    TArray<FText> entries;
    for (EScrollMode mode : TEnumRange<EScrollMode>())
        entries.Add(ScrollMode::GetName(mode));
    if(useGizmo)
        entries.Add(FText::FromString("Unlock Pivot"));
    entries.Add(FText::FromString("Reset Transform"));
    if (useGizmo)
        entries.Add(FText::FromString(UInfiniteGizmo::IsGizmoVisible() ? "Hide Gizmo" : "Show Gizmo"));
    return entries;
}

//Called from BP when a menu entry is selected.
void UInfiniteNudgeGameInstanceModule::SelectMode(int index) {
    if (index < 0 || index >= mMenuActions.Num())
        return;

    mMenuActions[index]();
    UpdateHint();
}

void UInfiniteNudgeGameInstanceModule::ToggleGizmo() {
    if (!IsHologramLocked())
        return;

    //Update config
    UConfigManager* ConfigManager = GetWorld()->GetGameInstance()->GetSubsystem<UConfigManager>();
    auto root = ConfigManager->GetConfigurationRootSection({ "InfiniteNudge", "" });
    CastChecked<UConfigPropertyBool>(root->SectionProperties.FindChecked("ShowScrollModeGizmo"))->Value = UInfiniteGizmo::ToggleVisibility();
    root->MarkDirty();
}

void UInfiniteNudgeGameInstanceModule::ToggleLockPivot() {
    if (!IsHologramLocked())
        return;

    if (mTickHandle.IsValid()) {
        FTSTicker::GetCoreTicker().RemoveTicker(mTickHandle);
        return;
    }

    mTickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UInfiniteNudgeGameInstanceModule::Tick));
    return;
}


void UInfiniteNudgeGameInstanceModule::ResetTransform() {
    if (!IsHologramLocked())
    {
        return;
    }

    auto* player = Cast<AFGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!player)
    {
        return;
    }

    auto* buildGun = player->GetBuildGun();
    auto* state = Cast<UFGBuildGunStateBuild>(buildGun->GetCurrentState());
	auto* hologram = state->GetHologram();
    if (!state)
    {
        return;
    }

	hologram->SetNudgeOffset(FVector::ZeroVector);
    hologram->SetActorScale3D(FVector(1, 1, 1));
    hologram->SetActorRotation(FRotator(0, 0, 0));
    for (auto c : hologram->mChildren) {
		c->SetNudgeOffset(FVector::ZeroVector);
        c->SetActorScale3D(FVector(1, 1, 1));
        c->SetActorRotation(FRotator(0, 0, 0));
    }
}


FVector GetClosestCorner(FTransform transform, FHitResult hitResult, const FBox& box)
{
    //TODO? maybe add edge midpoints if the box is big enough
    FVector corners[9] = {
        FVector::ZeroVector, //center
        FVector(box.Min.X, box.Min.Y, box.Min.Z),
        FVector(box.Max.X, box.Min.Y, box.Min.Z),
        FVector(box.Min.X, box.Max.Y, box.Min.Z),
        FVector(box.Max.X, box.Max.Y, box.Min.Z),
        FVector(box.Min.X, box.Min.Y, box.Max.Z),
        FVector(box.Max.X, box.Min.Y, box.Max.Z),
        FVector(box.Min.X, box.Max.Y, box.Max.Z),
        FVector(box.Max.X, box.Max.Y, box.Max.Z)
    };

    FVector closest(EForceInit::ForceInit);
    float closestDist = 999999.f;

    //Origin and end in the local space of the hologram
	FVector origin = transform.InverseTransformPosition(hitResult.TraceStart);
	FVector end = transform.InverseTransformPosition(hitResult.TraceEnd);

    for(auto corner : corners)  {
        float dist = FVector::DistSquared(corner, FMath::ClosestPointOnLine(origin, end, corner));
        if(dist < closestDist) {
            closestDist = dist;
            closest = corner;
        }
    }
    return closest;
}

//Update the unlocked pivot to the closet corner.
bool UInfiniteNudgeGameInstanceModule::Tick(float delta) {
	if (!IsHologramLocked()) {
        mTickHandle = nullptr;
        return false;
    }

    auto* player = Cast<AFGCharacterPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    auto* buildGun = player->GetBuildGun();
	auto* hologram = Cast<UFGBuildGunStateBuild>(buildGun->GetCurrentState())->GetHologram();
    auto hitResult = buildGun->GetHitResult();

    //Box calculated from the HOLOGRAM_MESH_TAG meshes. There might be a better way.
    FBox combinedBox(EForceInit::ForceInit);
    hologram->ForEachComponent<UStaticMeshComponent>(false, [this, &combinedBox, &hologram](UStaticMeshComponent* mesh) {
        if (!mesh->ComponentHasTag(HOLOGRAM_MESH_TAG))
            return;
        auto box = mesh->CalcLocalBounds().TransformBy(mesh->GetRelativeTransform()).GetBox();
        combinedBox += box;
    });
    
	FVector position = GetClosestCorner(hologram->GetActorTransform(), hitResult, combinedBox);
    FInfiniteNudgeModule::mPivot = position;
    UInfiniteGizmo::UpdatePivot(position);  
    
    return true;
}


//SHOW/HIDE MENU
UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem(UWorld* world) {
    auto controller = UGameplayStatics::GetPlayerController(world, 0);
    return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(controller->GetLocalPlayer());
}

void UInfiniteNudgeGameInstanceModule::ShowModeMenu() {
    if (mMenuOpened || !IsHologramLocked())
        return;

   if (!mModeMenuClass) {
        UE_LOG(InfiniteNudge, Error, TEXT("ScrollMode menu class not set!"));
        return;
    }   
   
    mMenuOpened = true;
    mScrollModeMenu = CreateWidget<UFGUserWidget>(GetWorld(), mModeMenuClass);
    mScrollModeMenu->AddToViewport(); 
    //block clicks    
    GetInputSubsystem(GetWorld())->AddMappingContext(mModeMenuMappingContext, 1005, FModifyContextOptions());

    //LogWidgetHierarchy(mScrollModeMenu);
}

void UInfiniteNudgeGameInstanceModule::CloseModeMenu() {
    mMenuOpened = false;
    if (!mScrollModeMenu)
        return;

    GetInputSubsystem(GetWorld())->RemoveMappingContext(mModeMenuMappingContext, FModifyContextOptions());
    mScrollModeMenu->RemoveFromParent();
    mScrollModeMenu = nullptr;
}

void UInfiniteNudgeGameInstanceModule::CycleScrollMode() {
    if (!IsHologramLocked())
        return;
    
    ScrollMode::Cycle();
    UpdateHint();   
}



//HINT EDIT

void UInfiniteNudgeGameInstanceModule::CacheHintTextBlock(UFGUserWidget*  container) {
    if (!container) {
        UE_LOG(InfiniteNudge, Error , TEXT("Container is null."));
        return;
    }

    //Container contains a Widget_Hint for each keybind hint.
    container->WidgetTree->ForEachWidget([this](UWidget* child) {
        auto widgetHint= Cast<UUserWidget>(child);
        if (!widgetHint)
            return;

		//Widget_Hint has 2 UTextBlocks, one for the keybind and one for the hint.
        auto tbDesc = widgetHint->WidgetTree->FindWidget<UTextBlock>(FName("mHintDescription"));
        if (tbDesc && tbDesc->GetText().ToString().Contains("Rotate")) { //Needs to match the default text in BP mKeybindHint[0].Action
            mHintTextBlock = tbDesc;
        }
     });
   
   //UE_LOG(InfiniteNudge, Display, TEXT("Textblock found : %s"), *mHintTextBlock->GetName());
    UpdateHint();
}

void UInfiniteNudgeGameInstanceModule::UpdateHint() {
    if (mHintTextBlock.IsValid()) 
        mHintTextBlock->SetText(ScrollMode::GetName());
}


//DEBUG FUNCTION

void UInfiniteNudgeGameInstanceModule::LogWidgetHierarchy(UUserWidget* widget, int32 depth)
{
    if (!widget) {
        UE_LOG(InfiniteNudge, Display, TEXT("No widget (%i)"), depth);
        return;
    }

    FString indent = FString::ChrN(depth * 2, ' ');
    UE_LOG(InfiniteNudge, Verbose, TEXT("%s[%s] %s"), *indent, *widget->GetClass()->GetName(), *widget->GetName());

    widget->WidgetTree->ForEachWidget([this, depth](UWidget* child)
        {
			if (auto uw = Cast<UUserWidget>(child))
                LogWidgetHierarchy(uw, depth + 1);            
        });
}
#pragma optimize("", on)