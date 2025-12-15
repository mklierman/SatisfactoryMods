#include "CP_GameInstanceModule.h"
#include "Algo/ForEach.h"
#include "Algo/Transform.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Module/GameInstanceModuleManager.h"
#include "Resources/FGBuildingDescriptor.h"

#if WITH_EDITOR
EDataValidationResult UCP_GameInstanceModule::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult ValidationResult = EDataValidationResult::Valid;

	if (!IsValid(Recipe_ConveyorPoleStackable)) {
		ValidationErrors.Add(INVTEXT("'Recipe_ConveyorPoleStackable' is not set to a valid class"));
		ValidationResult = EDataValidationResult::Invalid;
	}
	if (!IsValid(Build_ConveyorPoleStackable)) {
		ValidationErrors.Add(INVTEXT("'Build_ConveyorPoleStackable' is not set to a valid class"));
		ValidationResult = EDataValidationResult::Invalid;
	}
	if (!IsValid(Holo_ConveyorStackable)) {
		ValidationErrors.Add(INVTEXT("'Holo_ConveyorStackable' is not set to a valid class"));
		ValidationResult = EDataValidationResult::Invalid;
	}
	if (!IsValid(Holo_CP_Stackable_Belt)) {
		ValidationErrors.Add(INVTEXT("'Holo_CP_Stackable_Belt' is not set to a valid class"));
		ValidationResult = EDataValidationResult::Invalid;
	}

	return FMath::Min(ValidationResult, Super::IsDataValid(ValidationErrors));
}
#endif

UCP_GameInstanceModule* UCP_GameInstanceModule::Get(UObject* WorldContext)
{
	if (!IsValid(WorldContext)) {
		return nullptr;
	}
	if (UWorld* World = WorldContext->GetWorld()) {
		if (UGameInstance* GameInstance = World->GetGameInstance()) {
			if (UGameInstanceModuleManager* ModuleManager = GameInstance->GetSubsystem<UGameInstanceModuleManager>()) {
				if (UGameInstanceModule* InstanceMod = ModuleManager->FindModule(TEXT("ConstructionPreferences"))) {
					return CastChecked<UCP_GameInstanceModule>(InstanceMod);
				}
			}
		}
	}
	return nullptr;
}

/* Special thanks to Archengius for this snippet of code */
static void FindSubclassesOf(TSet<FTopLevelAssetPath>& out_AllClasses, UClass* BaseClass)
{
	TArray<UClass*> NativeRootClasses;
	GetDerivedClasses(BaseClass, NativeRootClasses);
	NativeRootClasses.Add(BaseClass);

	const auto predicate = [](const UClass* RootClass) { return RootClass && RootClass->HasAnyClassFlags(CLASS_Native); };
	const auto transform = &UClass::GetClassPathName;
	TArray<FTopLevelAssetPath> NativeRootClassPaths;
	Algo::TransformIf(NativeRootClasses, NativeRootClassPaths, predicate, transform);

	IAssetRegistry::Get()->GetDerivedClassNames(NativeRootClassPaths, {}, out_AllClasses);
	out_AllClasses.Append(NativeRootClassPaths);
}

void UCP_GameInstanceModule::ScanOneRecipe(TSubclassOf<UFGRecipe> Recipe)
{
	if (!IsValid(Recipe)) {
		UE_LOG(LogConstructionPreferences, Error, TEXT("Recipe NOT VALID"));
		return;
	}
	if (UFGRecipe* RecipeCDO = Recipe.GetDefaultObject()) {
		if (RecipeCDO->GetProducts().Num() != 1) {
			UE_LOG(LogConstructionPreferences, Error, TEXT("Recipe '%s' products %d != 1"), *RecipeCDO->GetName(), RecipeCDO->GetProducts().Num());
			return;
		}
		const TSubclassOf<UFGBuildingDescriptor> BuildingDesc(RecipeCDO->GetProducts()[0].ItemClass);
		/* Check if class downcast succeeded */
		if (!IsValid(*BuildingDesc)) {
			UE_LOG(LogConstructionPreferences, Error, TEXT("Recipe '%s' product class %s"), *RecipeCDO->GetName(), *RecipeCDO->GetProducts()[0].ItemClass->GetName());
			return;
		}
		TSubclassOf<AFGBuildable> BuildableClass = UFGBuildingDescriptor::GetBuildableClass(BuildingDesc);
		if (!IsValid(BuildableClass)) {
			UE_LOG(LogConstructionPreferences, Error, TEXT("Recipe '%s' bad buildable class"), *RecipeCDO->GetName());
			return;
		}
		if (BuildableClass->IsChildOf<AFGBuildableConveyorBelt>()) {
			UE_LOG(LogConstructionPreferences, Warning, TEXT("Recipe '%s' is a conveyor recipe"), *RecipeCDO->GetName());
			ConveyorBeltRecipes.Add(Recipe);
		}
		UE_LOG(LogConstructionPreferences, Error, TEXT("Recipe '%s' buildable class %s"), *RecipeCDO->GetName(), *BuildableClass->GetName());
	}
}

void UCP_GameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	Super::DispatchLifecycleEvent(Phase);

	UE_LOG(LogConstructionPreferences, Error, TEXT("HELLO"));

	if (Phase == ELifecyclePhase::POST_INITIALIZATION) {
		StackablePoleCDO = Build_ConveyorPoleStackable.GetDefaultObject();

		TSet<FTopLevelAssetPath> AssetPaths;
		FindSubclassesOf(AssetPaths, UFGRecipe::StaticClass());

		TArray<FSoftObjectPath> SoftPaths;
		Algo::Transform(AssetPaths, SoftPaths, [](const FTopLevelAssetPath& AssetPath) { return FSoftObjectPath(AssetPath); });

		UE_LOG(LogConstructionPreferences, Display, TEXT("Scanning %d recipes..."), AssetPaths.Num());
		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(SoftPaths, [this, SoftPaths]() {
			Algo::ForEach(SoftPaths, [this](const FSoftObjectPath& SoftPath) {
				ScanOneRecipe(TSoftClassPtr<UFGRecipe>(SoftPath).Get());
			});
			UE_LOG(LogConstructionPreferences, Display, TEXT("Done, found %d conveyor belt recipes"), ConveyorBeltRecipes.Num());
		}, FStreamableManager::AsyncLoadHighPriority);
		if (Handle) {
			Handle->WaitUntilComplete();
		}
	}
}