#include "StackableDistance.h"
#include "SD_ConfigStruct.h"
#include "SDRangeMath.h"
#include "SDRangeVisualizer.h"
#include "Buildables/FGBuildablePole.h"
#include "Buildables/FGBuildablePoleBase.h"
#include "Buildables/FGBuildablePoleStackable.h"
#include "FGBuildablePolePipe.h"
#include "Buildables/FGBuildablePoleConveyor.h"
#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildablePipeline.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Hologram/FGBuildableHologram.h"
#include "Hologram/FGStackablePoleHologram.h"
#include "Hologram/FGPoleHologram.h"
#include "Hologram/FGPipelinePoleHologram.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include "FGCharacterPlayer.h"
#include "FGRecipe.h"
#include "Resources/FGBuildDescriptor.h"
#include <Patching/NativeHookManager.h>
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"

#define LOCTEXT_NAMESPACE "FStackableDistanceModule"
DEFINE_LOG_CATEGORY(StackableDistance_Log);

void FStackableDistanceModule::StartupModule()
{
	AFGBuildableHologram* bh = GetMutableDefault<AFGBuildableHologram>();
	AFGStackablePoleHologram* sph = GetMutableDefault<AFGStackablePoleHologram>();
	AFGPoleHologram* ph = GetMutableDefault<AFGPoleHologram>();
	AFGPipelinePoleHologram* pph = GetMutableDefault<AFGPipelinePoleHologram>();
	UFGBuildGunStateBuild* bgsb = GetMutableDefault<UFGBuildGunStateBuild>();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableHologram::ConfigureActor, bh, [this](const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			if (IsValid(self) && IsValid(inBuildable) && IsLocalHologram(self))
			{
				OnBuildableConstructed(self, inBuildable);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGStackablePoleHologram::SetHologramLocationAndRotation, sph, [this](AFGStackablePoleHologram* self, const FHitResult& hitResult)
		{
			if (IsValid(self))
			{
				HandleHologramPlacement(self, hitResult);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPoleHologram::SetHologramLocationAndRotation, ph, [this](AFGPoleHologram* self, const FHitResult& hitResult)
		{
			if (IsValid(self))
			{
				HandleHologramPlacement(self, hitResult);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGPipelinePoleHologram::SetHologramLocationAndRotation, pph, [this](AFGPipelinePoleHologram* self, const FHitResult& hitResult)
		{
			if (IsValid(self))
			{
				HandleHologramPlacement(self, hitResult);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::PrimaryFire_Implementation, bgsb, [this](auto& scope, UFGBuildGunStateBuild* self)
		{
			AFGHologram* hologram = nullptr;
			UClass* buildClass = nullptr;
			FVector location = FVector::ZeroVector;
			FRotator rotation = FRotator::ZeroRotator;
			bool commit = false;

			if (IsLocalBuildState(self) && IsValid(self))
			{
				hologram = self->GetHologram();
				if (IsLocalHologram(hologram) && IsPoleBuildClass(hologram->GetBuildClass()) &&
					(hologram->CanConstruct() || hologram->GetHologramMaterialState() == EHologramMaterialState::HMS_OK))
				{
					location = hologram->GetActorLocation();
					rotation = hologram->GetActorRotation();
					buildClass = hologram->GetBuildClass();
					commit = true;
				}
			}

			scope(self);

			if (commit)
			{
				SetAnchorFromPole(location, rotation, buildClass);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGBuildGunStateBuild::EndState_Implementation, bgsb, [this](UFGBuildGunStateBuild* self)
		{
			if (IsLocalBuildState(self))
			{
				ResetAnchor();
			}
		});

	SUBSCRIBE_METHOD_AFTER(UFGBuildGunStateBuild::SetActiveRecipe, [this](UFGBuildGunStateBuild* self, TSubclassOf<UFGRecipe> recipe)
		{
			if (IsLocalBuildState(self) && HasAnchor && !IsRecipeForAnchor(recipe))
			{
				ResetAnchor();
			}
		});
#endif
}

void FStackableDistanceModule::ShutdownModule()
{
	ResetAnchor();
}

float FStackableDistanceModule::GetSplineMaxLengthForClass(UClass* buildableClass)
{
	UClass* targetBuildableClass = AFGBuildableConveyorBelt::StaticClass();

	if (buildableClass)
	{
		FString className = buildableClass->GetName();
		if (className.Contains(TEXT("Hyper")))
		{
			targetBuildableClass = AFGBuildablePipeHyper::StaticClass();
		}
		else if (className.Contains(TEXT("Pipe")) || buildableClass->IsChildOf(AFGBuildablePolePipe::StaticClass()))
		{
			targetBuildableClass = AFGBuildablePipeline::StaticClass();
		}
	}

	if (targetBuildableClass)
	{
		if (auto cdo = Cast<AFGBuildable>(targetBuildableClass->GetDefaultObject()))
		{
			if (auto prop = targetBuildableClass->FindPropertyByName(TEXT("mHologramClass")))
			{
				if (auto classProp = CastField<FClassProperty>(prop))
				{
					if (auto holoClass = Cast<UClass>(classProp->GetPropertyValue_InContainer(cdo)))
					{
						if (auto lengthProp = holoClass->FindPropertyByName(TEXT("mMaxSplineLength")))
						{
							if (auto floatProp = CastField<FFloatProperty>(lengthProp))
							{
								if (auto holoCDO = holoClass->GetDefaultObject())
								{
									float length = floatProp->GetPropertyValue_InContainer(holoCDO);
									if (length > 100.0f)
									{
										return length;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (buildableClass && buildableClass->GetName().Contains(TEXT("Hyper")))
	{
		return 10000.0f;
	}
	return 5600.0f;
}

void FStackableDistanceModule::OnBuildableConstructed(const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
{
	if (!inBuildable || !IsPoleBuildClass(inBuildable->GetClass()))
	{
		return;
	}

	SetAnchorFromPole(inBuildable->GetActorLocation(), inBuildable->GetActorRotation(), inBuildable->GetClass());
}

void FStackableDistanceModule::ResetAnchor()
{
	HasAnchor = false;
	LastPoleLocation = FVector::ZeroVector;
	LastPoleRotation = FRotator::ZeroRotator;
	LastPoleClass = nullptr;
	DestroyRangeVisualizer();
}

void FStackableDistanceModule::DestroyRangeVisualizer()
{
	if (auto draw = RangeDraw.Get())
	{
		draw->DestroyComponent();
	}
	RangeDraw.Reset();
}

bool FStackableDistanceModule::IsLocalPawn(APawn* pawn)
{
	return IsValid(pawn) && pawn->IsLocallyControlled();
}

bool FStackableDistanceModule::IsLocalHologram(const AFGHologram* hologram)
{
	if (!IsValid(hologram) || !hologram->GetWorld() || hologram->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return false;
	}

	if (auto pawn = hologram->GetConstructionInstigator())
	{
		return pawn->IsLocallyControlled();
	}

	return true;
}

bool FStackableDistanceModule::IsLocalBuildState(UFGBuildGunState* state)
{
	if (!IsValid(state))
	{
		return false;
	}

	auto buildGun = state->GetBuildGun();
	if (!buildGun)
	{
		return false;
	}

	if (auto character = Cast<APawn>(buildGun->GetInstigatorCharacter()))
	{
		return character->IsLocallyControlled();
	}

	if (auto ownerPawn = Cast<APawn>(buildGun->GetOwner()))
	{
		return ownerPawn->IsLocallyControlled();
	}

	return buildGun->GetWorld() && buildGun->GetWorld()->GetNetMode() != NM_DedicatedServer;
}

bool FStackableDistanceModule::IsPoleBuildClass(UClass* buildClass)
{
	return buildClass && (buildClass->IsChildOf(AFGBuildablePoleBase::StaticClass()) || buildClass->IsChildOf(AFGBuildablePoleStackable::StaticClass()));
}

void FStackableDistanceModule::SetAnchorFromPole(FVector location, FRotator rotation, UClass* buildClass)
{
	if (!IsPoleBuildClass(buildClass))
	{
		return;
	}

	LastPoleLocation = location;
	LastPoleRotation = rotation;
	LastPoleClass = buildClass;
	CurrentTargetMaxLength = GetSplineMaxLengthForClass(buildClass);
	HasAnchor = true;
}

bool FStackableDistanceModule::IsHologramForAnchor(AFGHologram* hologram)
{
	return IsValid(hologram) && LastPoleClass.Get() && hologram->GetBuildClass() == LastPoleClass.Get();
}

bool FStackableDistanceModule::IsRecipeForBuildClass(TSubclassOf<UFGRecipe> recipe, UClass* buildClass)
{
	if (!recipe || !buildClass)
	{
		return false;
	}

	TArray<FItemAmount> products = UFGRecipe::GetProducts(recipe);
	for (auto& product : products)
	{
		if (product.ItemClass && product.ItemClass->IsChildOf(UFGBuildDescriptor::StaticClass()))
		{
			TSubclassOf<UFGBuildDescriptor> buildDesc(product.ItemClass);
			if (UFGBuildDescriptor::GetBuildClass(buildDesc) == buildClass)
			{
				return true;
			}
		}
	}

	return false;
}

bool FStackableDistanceModule::IsRecipeForAnchor(TSubclassOf<UFGRecipe> recipe)
{
	return IsRecipeForBuildClass(recipe, LastPoleClass.Get());
}

void FStackableDistanceModule::UpdateRangeVisualizer(AFGHologram* hologram, float baseDistance, float minCurveScale)
{
	if (!IsValid(hologram) || !hologram->GetWorld() || hologram->GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	auto draw = RangeDraw.Get();
	if (!draw || draw->GetOwner() != hologram)
	{
		if (draw)
		{
			draw->DestroyComponent();
		}

		draw = NewObject<USDRangeDrawComponent>(hologram, TEXT("SDRangeDraw"));
		if (auto root = hologram->GetRootComponent())
		{
			draw->SetupAttachment(root);
			draw->SetAbsolute(true, true, true);
		}
		draw->RegisterComponent();
		RangeDraw = draw;
	}

	draw->SetWorldLocation(LastPoleLocation);
	draw->SetWorldRotation(FRotator::ZeroRotator);
	draw->SetRange(
		baseDistance,
		minCurveScale,
		LastPoleRotation.Vector().GetSafeNormal2D(),
		hologram->GetActorRotation().Vector().GetSafeNormal2D());
}

void FStackableDistanceModule::HandleHologramPlacement(AFGHologram* self, const FHitResult& hitResult)
{
	if (!IsLocalHologram(self))
	{
		return;
	}

	if (!HasAnchor)
	{
		return;
	}

	if (!IsHologramForAnchor(self))
	{
		ResetAnchor();
		return;
	}

	auto world = self->GetWorld();
	if (!world)
	{
		return;
	}

	auto instigatorPawn = self->GetConstructionInstigator();
	auto pc = instigatorPawn ? Cast<APlayerController>(instigatorPawn->GetController()) : nullptr;
	if (!pc || !pc->IsLocalController())
	{
		pc = world->GetFirstPlayerController();
		if (pc && !pc->IsLocalController())
		{
			pc = nullptr;
		}
	}

	FVector targetHitPoint = self->GetActorLocation();
	if (!hitResult.ImpactPoint.IsNearlyZero())
	{
		targetHitPoint = FVector(hitResult.ImpactPoint);
	}
	FVector dir2D = (targetHitPoint - LastPoleLocation).GetSafeNormal2D();
	if (dir2D.IsNearlyZero())
	{
		dir2D = self->GetActorForwardVector().GetSafeNormal2D();
	}

	auto config = FSD_ConfigStruct::GetActiveConfig(world);
	float safetyBufferCm = FMath::Clamp(config.SafetyBuffer, 0.0f, 10.0f) * 100.0f;
	float minCurveScale = FMath::Clamp(config.CurveCompensation, 0.0f, 1.0f);
	float baseDistance = FMath::Max(100.0f, CurrentTargetMaxLength - safetyBufferCm);

	UpdateRangeVisualizer(self, baseDistance, minCurveScale);

	bool modifierDown = pc && (pc->IsInputKeyDown(EKeys::LeftShift) || pc->IsInputKeyDown(EKeys::RightShift));
	if (!modifierDown)
	{
		return;
	}

	FVector anchorForward = LastPoleRotation.Vector().GetSafeNormal2D();
	FVector currentForward = self->GetActorRotation().Vector().GetSafeNormal2D();
	float effectiveMaxLength = SDRangeMath::ComputeReach(dir2D, anchorForward, currentForward, baseDistance, minCurveScale);

	FVector targetXY = LastPoleLocation + (dir2D * effectiveMaxLength);

	FVector traceStart = targetXY + FVector(0.f, 0.f, 2500.f);
	FVector traceEnd = targetXY - FVector(0.f, 0.f, 6000.f);

	FHitResult groundHit;
	FCollisionQueryParams traceParams(SCENE_QUERY_STAT(StackableDistanceTerrainTrace), false);
	traceParams.AddIgnoredActor(self);
	if (instigatorPawn)
	{
		traceParams.AddIgnoredActor(instigatorPawn);
	}

	FVector finalLocation = targetXY;
	if (world->LineTraceSingleByChannel(groundHit, traceStart, traceEnd, ECC_Visibility, traceParams))
	{
		finalLocation = FVector(groundHit.ImpactPoint);

		float deltaZ = finalLocation.Z - LastPoleLocation.Z;
		float maxDistSq = FMath::Square(effectiveMaxLength);
		float deltaZSq = FMath::Square(deltaZ);

		if (deltaZSq < maxDistSq)
		{
			float maxAllowedHoriz = FMath::Sqrt(maxDistSq - deltaZSq);
			finalLocation = LastPoleLocation + (dir2D * maxAllowedHoriz);
			finalLocation.Z = groundHit.ImpactPoint.Z;
		}
	}

	self->SetActorLocation(finalLocation);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStackableDistanceModule, StackableDistance)
