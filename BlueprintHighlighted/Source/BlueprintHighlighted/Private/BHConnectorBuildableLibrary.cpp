#include "BHConnectorBuildableLibrary.h"

#include "FGBuildableSubsystem.h"
#include "FGBlueprintSubsystem.h"
#include "FGRecipeManager.h"
#include "FGRecipe.h"
#include "FGSplineBuildableInterface.h"
#include "FGCircuitConnectionComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "Buildables/FGBuildableConveyorLift.h"
#include "Buildables/FGBuildableRailroadTrack.h"
#include "Buildables/FGBuildableSplitterSmart.h"
#include "Buildables/FGBuildableWire.h"
#include "Resources/FGNoneDescriptor.h"
#include "UObject/UnrealType.h"

namespace
{
	// Components are created from the same construction script when spawning from the same class,
	// so matching by component name finds the equivalent connector on a freshly duplicated buildable.
	template<typename TConnection>
	TConnection* FindMatchingComponentByName(AFGBuildable* NewBuildable, TConnection* OriginalComponent)
	{
		if (!NewBuildable || !OriginalComponent)
		{
			return nullptr;
		}

		const FName TargetName = OriginalComponent->GetFName();

		TArray<TConnection*> Candidates;
		NewBuildable->GetComponents<TConnection>(Candidates);

		for (TConnection* Candidate : Candidates)
		{
			if (Candidate->GetFName() == TargetName)
			{
				return Candidate;
			}
		}

		// Fall back to the first free connector of the same class if the name didn't match -
		// better than failing outright when a subclass renames its components.
		return Candidates.Num() > 0 ? Candidates[0] : nullptr;
	}

	UFGCircuitConnectionComponent* FindMatchingCircuitConnection(AFGBuildable* NewBuildable, UFGCircuitConnectionComponent* OriginalConnection)
	{
		return FindMatchingComponentByName(NewBuildable, OriginalConnection);
	}

	void CopySavedConnectionDirections(AFGBuildableConveyorAttachment* Original, AFGBuildableConveyorAttachment* Copy)
	{
		static const FArrayProperty* SavedDirectionsProperty = CastField<FArrayProperty>(
			AFGBuildableConveyorAttachment::StaticClass()->FindPropertyByName(TEXT("mSavedDirections")));
		if (!SavedDirectionsProperty)
		{
			return;
		}

		FScriptArrayHelper OriginalDirections(SavedDirectionsProperty, SavedDirectionsProperty->ContainerPtrToValuePtr<void>(Original));
		if (OriginalDirections.Num() == 0)
		{
			Original->SaveCurrentDirections();
		}

		SavedDirectionsProperty->CopyCompleteValue_InContainer(Copy, Original);
	}

	void CopySplitterSortRules(AFGBuildable* Original, AFGBuildable* Copy)
	{
		AFGBuildableSplitterSmart* OriginalSplitter = Cast<AFGBuildableSplitterSmart>(Original);
		AFGBuildableSplitterSmart* NewSplitter = Cast<AFGBuildableSplitterSmart>(Copy);
		if (OriginalSplitter && NewSplitter)
		{
			NewSplitter->SetSortRules(OriginalSplitter->GetSortRules());
		}
	}

	struct FBalancerProps
	{
		FObjectPropertyBase* Leader = nullptr;
		FArrayProperty* Modules = nullptr;
		FObjectPropertyBase* Module = nullptr;
		FArrayProperty* Filters = nullptr;
		FClassProperty* FilterClass = nullptr;
		UFunction* SetFilters = nullptr;

		bool IsValid() const
		{
			return Leader && Modules && Module && Filters && FilterClass && SetFilters;
		}
	};

	FBalancerProps FindBalancerProps(UClass* Class)
	{
		FBalancerProps Props;
		if (!Class)
		{
			return Props;
		}

		Props.Leader = CastField<FObjectPropertyBase>(Class->FindPropertyByName(TEXT("GroupLeader")));
		Props.Modules = CastField<FArrayProperty>(Class->FindPropertyByName(TEXT("mGroupModules")));
		Props.Module = Props.Modules ? CastField<FObjectPropertyBase>(Props.Modules->Inner) : nullptr;
		Props.Filters = CastField<FArrayProperty>(Class->FindPropertyByName(TEXT("mFilteredItems")));
		Props.FilterClass = Props.Filters ? CastField<FClassProperty>(Props.Filters->Inner) : nullptr;
		Props.SetFilters = Class->FindFunctionByName(TEXT("SetFilteredItems"));
		return Props;
	}

	void AddModule(const FBalancerProps& Props, AFGBuildable* Leader, AFGBuildable* Module)
	{
		FScriptArrayHelper Modules(Props.Modules, Props.Modules->ContainerPtrToValuePtr<void>(Leader));
		for (int32 Index = 0; Index < Modules.Num(); ++Index)
		{
			if (Props.Module->GetObjectPropertyValue(Modules.GetRawPtr(Index)) == Module)
			{
				return;
			}
		}

		Props.Module->SetObjectPropertyValue(Modules.GetRawPtr(Modules.AddValue()), Module);
	}

	void ApplyFilters(const FBalancerProps& Props, AFGBuildable* Original, AFGBuildable* Copy)
	{
		TArray<UClass*> ItemClasses;
		FScriptArrayHelper Items(Props.Filters, Props.Filters->ContainerPtrToValuePtr<void>(Original));
		for (int32 Index = 0; Index < Items.Num(); ++Index)
		{
			UClass* ItemClass = Cast<UClass>(Props.FilterClass->GetObjectPropertyValue(Items.GetRawPtr(Index)));
			if (ItemClass && !ItemClass->IsChildOf(UFGNoneDescriptor::StaticClass()))
			{
				ItemClasses.Add(ItemClass);
			}
		}

		if (ItemClasses.Num() == 0)
		{
			return;
		}

		struct FParams
		{
			TArray<TSubclassOf<UFGItemDescriptor>> Items;
		};

		FParams Params;
		for (UClass* ItemClass : ItemClasses)
		{
			Params.Items.Add(ItemClass);
		}
		Copy->ProcessEvent(Props.SetFilters, &Params);
	}

	bool IsLowerLocation(const FVector& Location, const FVector& Best)
	{
		if (Location.X != Best.X)
		{
			return Location.X < Best.X;
		}
		if (Location.Y != Best.Y)
		{
			return Location.Y < Best.Y;
		}
		return Location.Z < Best.Z;
	}

	void CopyLoadBalancerSettings(AFGBuildable* Original, AFGBuildable* Copy)
	{
		const FBalancerProps Props = FindBalancerProps(Original->GetClass());
		if (!Props.IsValid() || Original->GetClass() != Copy->GetClass())
		{
			return;
		}

		AFGBuildable* OriginalLeader = Cast<AFGBuildable>(Props.Leader->GetObjectPropertyValue_InContainer(Original));
		if (!OriginalLeader)
		{
			OriginalLeader = Original;
		}

		TArray<AFGBuildable*> OriginalMembers;
		OriginalMembers.Add(OriginalLeader);
		FScriptArrayHelper LeaderModules(Props.Modules, Props.Modules->ContainerPtrToValuePtr<void>(OriginalLeader));
		for (int32 Index = 0; Index < LeaderModules.Num(); ++Index)
		{
			if (AFGBuildable* Module = Cast<AFGBuildable>(Props.Module->GetObjectPropertyValue(LeaderModules.GetRawPtr(Index))))
			{
				OriginalMembers.AddUnique(Module);
			}
		}

		const FTransform Delta = Copy->GetActorTransform() * Original->GetActorTransform().Inverse();
		TArray<TPair<AFGBuildable*, AFGBuildable*>> Copies;
		Copies.Emplace(Original, Copy);

		if (AFGBuildableSubsystem* Subsystem = AFGBuildableSubsystem::Get(Copy))
		{
			for (AFGBuildable* Member : OriginalMembers)
			{
				if (!Member || Member == Original)
				{
					continue;
				}

				const FVector ExpectedLocation = (Delta * Member->GetActorTransform()).GetLocation();
				AFGBuildable* Found = nullptr;
				float BestDistSq = FMath::Square(5.f);
				for (AFGBuildable* Buildable : Subsystem->GetAllBuildablesRef())
				{
					if (!Buildable || Buildable->GetClass() != Member->GetClass() || OriginalMembers.Contains(Buildable))
					{
						continue;
					}

					const float DistSq = FVector::DistSquared(Buildable->GetActorLocation(), ExpectedLocation);
					if (DistSq <= BestDistSq)
					{
						Found = Buildable;
						BestDistSq = DistSq;
					}
				}

				if (Found)
				{
					Copies.Emplace(Member, Found);
				}
			}
		}

		AFGBuildable* NewLeader = Copy;
		FVector BestLocation = Copy->GetActorLocation();
		for (const TPair<AFGBuildable*, AFGBuildable*>& Pair : Copies)
		{
			const FVector Location = Pair.Value->GetActorLocation();
			if (IsLowerLocation(Location, BestLocation))
			{
				NewLeader = Pair.Value;
				BestLocation = Location;
			}
		}

		const FBalancerProps LeaderProps = FindBalancerProps(NewLeader->GetClass());
		for (const TPair<AFGBuildable*, AFGBuildable*>& Pair : Copies)
		{
			const FBalancerProps MemberProps = FindBalancerProps(Pair.Key->GetClass());
			FScriptArrayHelper Modules(MemberProps.Modules, MemberProps.Modules->ContainerPtrToValuePtr<void>(Pair.Value));
			Modules.EmptyValues();
			MemberProps.Leader->SetObjectPropertyValue_InContainer(Pair.Value, NewLeader);
			AddModule(LeaderProps, NewLeader, Pair.Value);
			ApplyFilters(MemberProps, Pair.Key, Pair.Value);
			Pair.Value->ForceNetUpdate();
		}
	}
}

AFGBuildable* UBHConnectorBuildableLibrary::SpawnConnectorBuildableCopy(
	UObject* WorldContextObject,
	AFGBuildable* BuildableToCopy,
	TSubclassOf<AFGBuildable> BuildableClass,
	const FTransform& Transform)
{
	if (!BuildableToCopy || !BuildableClass)
	{
		return nullptr;
	}

	// Wires are AFGBuildable themselves, so they show up in the dismantle-mode selection alongside
	// everything else - but they only make sense once both endpoint buildables already have copies,
	// which this generic path has no way to know. DuplicateWireBetweenNewBuildables is the only thing
	// that should ever create a wire copy; spawning one here too would leave a second, connectionless
	// wire actor sitting on top of it.
	if (BuildableToCopy->IsA<AFGBuildableWire>())
	{
		return nullptr;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContextObject);
	if (!BuildableSubsystem)
	{
		return nullptr;
	}

	AFGBuildable* NewBuildable = BuildableSubsystem->BeginSpawnBuildable(BuildableClass, Transform);
	if (!NewBuildable)
	{
		return nullptr;
	}

	TSubclassOf<UFGRecipe> BuiltWithRecipe = BuildableToCopy->GetBuiltWithRecipe();
	AFGRecipeManager* RecipeManager = AFGRecipeManager::Get(WorldContextObject);
	if (BuiltWithRecipe != nullptr && RecipeManager != nullptr)
	{
		if (!RecipeManager->IsRecipeAvailable(BuiltWithRecipe))
		{
			FString RecipeName = BuiltWithRecipe->GetName();
			RecipeName.RemoveFromEnd(TEXT("_C"));
			if (RecipeName.EndsWith(TEXT("Lift")))
			{
				RecipeName.RemoveFromEnd(TEXT("Lift"));
				FString BaseRecipePath = TEXT("/Game/FactoryGame/Recipes/Buildings/") + RecipeName + TEXT(".") + RecipeName + TEXT("_C");
				TSubclassOf<UFGRecipe> BaseRecipe = LoadClass<UFGRecipe>(nullptr, *BaseRecipePath);
				if (BaseRecipe != nullptr)
				{
					if (RecipeManager->IsRecipeAvailable(BaseRecipe))
					{
						BuiltWithRecipe = BaseRecipe;
					}
				}
			}
		}
	}
	NewBuildable->SetBuiltWithRecipe(BuiltWithRecipe);

	// Belts/pipes/hypertube/rail: copy the spline data before FinishSpawning (BeginPlay flushes it into
	// the spline component and it must not be touched afterwards - see FGSplineBuildableInterface.h).
	IFGSplineBuildableInterface* SourceSpline = Cast<IFGSplineBuildableInterface>(BuildableToCopy);
	IFGSplineBuildableInterface* NewSpline = Cast<IFGSplineBuildableInterface>(NewBuildable);
	if (SourceSpline && NewSpline)
	{
		if (TArray<FSplinePointData>* MutableSplineData = NewSpline->GetMutableSplinePointData())
		{
			*MutableSplineData = SourceSpline->GetSplinePointData();
		}
	}

	if (AFGBuildableConveyorAttachment* OriginalAttachment = Cast<AFGBuildableConveyorAttachment>(BuildableToCopy))
	{
		if (AFGBuildableConveyorAttachment* NewAttachment = Cast<AFGBuildableConveyorAttachment>(NewBuildable))
		{
			CopySavedConnectionDirections(OriginalAttachment, NewAttachment);
		}
	}

	if (AFGBuildableConveyorLift* LiftToCopy = Cast<AFGBuildableConveyorLift>(BuildableToCopy))
	{
		if (AFGBuildableConveyorLift* NewLift = Cast<AFGBuildableConveyorLift>(NewBuildable))
		{
			static const TCHAR* LiftPropertyNames[] =
			{
				TEXT("mTopTransform"),
				TEXT("mIsReversed"),
				TEXT("mIsBeltUsingInputRotation"),
				TEXT("mInputMeshDisplayMode"),
				TEXT("mOutputMeshDisplayMode"),
			};

			for (const TCHAR* PropertyName : LiftPropertyNames)
			{
				if (const FProperty* Property = AFGBuildableConveyorLift::StaticClass()->FindPropertyByName(PropertyName))
				{
					Property->CopyCompleteValue_InContainer(NewLift, LiftToCopy);
				}
			}
		}
	}

	NewBuildable->FinishSpawning(Transform);

	// This is the same hook the blueprint subsystem calls after loading a saved blueprint - it
	// cleans up any stale connection references left over from spawning/serialization. It does
	// NOT create new connections; that happens separately in ReconnectSpawnedBuildable once all
	// buildables in the selection have been duplicated.
	if (!NewBuildable->IsA<AFGBuildableRailroadTrack>())
	{
		NewBuildable->PostSerializedFromBlueprint(/*isBlueprintWorld=*/true);
	}

	return NewBuildable;
}

AFGBuildableWire* UBHConnectorBuildableLibrary::DuplicateWireBetweenNewBuildables(
	UObject* WorldContextObject,
	AFGBuildableWire* WireToCopy,
	AFGBuildable* NewFirstBuildable,
	AFGBuildable* NewSecondBuildable)
{
	if (!WireToCopy || !NewFirstBuildable || !NewSecondBuildable)
	{
		return nullptr;
	}

	UFGCircuitConnectionComponent* OriginalConnection0 = WireToCopy->GetConnection(0);
	UFGCircuitConnectionComponent* OriginalConnection1 = WireToCopy->GetConnection(1);
	if (!OriginalConnection0 || !OriginalConnection1)
	{
		return nullptr;
	}

	UFGCircuitConnectionComponent* NewConnection0 = FindMatchingCircuitConnection(NewFirstBuildable, OriginalConnection0);
	UFGCircuitConnectionComponent* NewConnection1 = FindMatchingCircuitConnection(NewSecondBuildable, OriginalConnection1);
	if (!NewConnection0 || !NewConnection1)
	{
		return nullptr;
	}

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContextObject);
	if (!BuildableSubsystem)
	{
		return nullptr;
	}

	AFGBuildable* NewWireBuildable = BuildableSubsystem->BeginSpawnBuildable(WireToCopy->GetClass(), WireToCopy->GetActorTransform());
	AFGBuildableWire* NewWire = Cast<AFGBuildableWire>(NewWireBuildable);
	if (!NewWire)
	{
		return nullptr;
	}

	if (!NewWire->Connect(NewConnection0, NewConnection1))
	{
		NewWire->Destroy();
		return nullptr;
	}

	NewWire->FinishSpawning(WireToCopy->GetActorTransform());

	return NewWire;
}

void UBHConnectorBuildableLibrary::CopyBuildableSettings(AFGBuildable* OriginalBuildable, AFGBuildable* NewBuildable)
{
	if (!OriginalBuildable || !NewBuildable)
	{
		return;
	}

	CopySplitterSortRules(OriginalBuildable, NewBuildable);
	CopyLoadBalancerSettings(OriginalBuildable, NewBuildable);
}

void UBHConnectorBuildableLibrary::SetPotential(AFGBuildableFactory* building, float newPotential)
{
	building->SetPendingPotential(newPotential);
}

void UBHConnectorBuildableLibrary::AllowLiftAttachmentsInBlueprints(UObject* WorldContextObject)
{
	AFGBlueprintSubsystem* BlueprintSubsystem = AFGBlueprintSubsystem::GetBlueprintSubsystem(WorldContextObject);
	if (BlueprintSubsystem == nullptr)
	{
		return;
	}

	TArray<FString> LiftAttachmentPaths;
	LiftAttachmentPaths.Add(TEXT("/Game/FactoryGame/Buildable/Factory/CA_MergerLift/Build_ConveyorAttachmentMergerLift.Build_ConveyorAttachmentMergerLift_C"));
	LiftAttachmentPaths.Add(TEXT("/Game/FactoryGame/Buildable/Factory/CA_MergerLiftPriority/Build_ConveyorAttachmentMergerPriorityLift.Build_ConveyorAttachmentMergerPriorityLift_C"));
	LiftAttachmentPaths.Add(TEXT("/Game/FactoryGame/Buildable/Factory/CA_SplitterLift/Build_ConveyorAttachmentSplitterLift.Build_ConveyorAttachmentSplitterLift_C"));
	LiftAttachmentPaths.Add(TEXT("/Game/FactoryGame/Buildable/Factory/CA_SplitterLiftProgrammable/Build_ConveyorAttachmentSplitterProgrammableLift.Build_ConveyorAttachmentSplitterProgrammableLift_C"));
	LiftAttachmentPaths.Add(TEXT("/Game/FactoryGame/Buildable/Factory/CA_SplitterLiftSmart/Build_ConveyorAttachmentSplitterSmartLift.Build_ConveyorAttachmentSplitterSmartLift_C"));

	for (int32 i = 0; i < LiftAttachmentPaths.Num(); i++)
	{
		TSubclassOf<AFGBuildable> LiftAttachmentClass = LoadClass<AFGBuildable>(nullptr, *LiftAttachmentPaths[i]);
		if (LiftAttachmentClass != nullptr)
		{
			BlueprintSubsystem->mIgnoreRecipeRequirements.AddUnique(LiftAttachmentClass);
		}
	}
}

void UBHConnectorBuildableLibrary::ReconnectSpawnedBuildable(
	AFGBuildable* OriginalBuildable,
	AFGBuildable* NewBuildable,
	TMap<AFGBuildable*, AFGBuildable*>& OldToNewBuildables)
{
	if (!OriginalBuildable || !NewBuildable)
	{
		return;
	}

	OldToNewBuildables.Add(OriginalBuildable, NewBuildable);

	// Belts (factory connections). Checking every connector - not just ones pointing at
	// already-processed neighbors - means each real connection gets made exactly once,
	// whichever of its two buildables happens to be duplicated second.
	TArray<UFGFactoryConnectionComponent*> FactoryConnections;
	OriginalBuildable->GetComponents<UFGFactoryConnectionComponent>(FactoryConnections);
	for (UFGFactoryConnectionComponent* OriginalConnection : FactoryConnections)
	{
		if (!OriginalConnection->IsConnected())
		{
			continue;
		}

		UFGFactoryConnectionComponent* OriginalNeighborConnection = OriginalConnection->GetConnection();
		AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
		AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
		if (!NewNeighbor)
		{
			continue;
		}

		UFGFactoryConnectionComponent* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
		UFGFactoryConnectionComponent* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
		if (NewConnection && NewNeighborConnection && !NewConnection->IsConnected() && !NewNeighborConnection->IsConnected())
		{
			NewConnection->SetConnection(NewNeighborConnection);
		}
	}

	// Pipes and hypertubes (both derive from UFGPipeConnectionComponentBase).
	TArray<UFGPipeConnectionComponentBase*> PipeConnections;
	OriginalBuildable->GetComponents<UFGPipeConnectionComponentBase>(PipeConnections);
	for (UFGPipeConnectionComponentBase* OriginalConnection : PipeConnections)
	{
		if (!OriginalConnection->IsConnected())
		{
			continue;
		}

		UFGPipeConnectionComponentBase* OriginalNeighborConnection = OriginalConnection->GetConnection();
		AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
		AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
		if (!NewNeighbor)
		{
			continue;
		}

		UFGPipeConnectionComponentBase* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
		UFGPipeConnectionComponentBase* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
		if (NewConnection && NewNeighborConnection && !NewConnection->IsConnected() && !NewNeighborConnection->IsConnected())
		{
			NewConnection->SetConnection(NewNeighborConnection);
		}
	}

	// Rail. Track connections can hold more than one link (switches), so walk all of them.
	TArray<UFGRailroadTrackConnectionComponent*> RailConnections;
	OriginalBuildable->GetComponents<UFGRailroadTrackConnectionComponent>(RailConnections);
	for (UFGRailroadTrackConnectionComponent* OriginalConnection : RailConnections)
	{
		for (UFGRailroadTrackConnectionComponent* OriginalNeighborConnection : OriginalConnection->GetConnections())
		{
			if (!OriginalNeighborConnection)
			{
				continue;
			}

			AFGBuildable* OldNeighbor = Cast<AFGBuildable>(OriginalNeighborConnection->GetOwner());
			AFGBuildable** NewNeighbor = OldToNewBuildables.Find(OldNeighbor);
			if (!NewNeighbor)
			{
				continue;
			}

			UFGRailroadTrackConnectionComponent* NewConnection = FindMatchingComponentByName(NewBuildable, OriginalConnection);
			UFGRailroadTrackConnectionComponent* NewNeighborConnection = FindMatchingComponentByName(*NewNeighbor, OriginalNeighborConnection);
			if (NewConnection && NewNeighborConnection && !NewConnection->GetConnections().Contains(NewNeighborConnection))
			{
				NewConnection->AddConnection(NewNeighborConnection);
			}
		}
	}
}