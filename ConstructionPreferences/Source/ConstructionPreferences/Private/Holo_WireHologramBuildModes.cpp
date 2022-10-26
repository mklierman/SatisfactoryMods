#include "Holo_WireHologramBuildModes.h"
#include "FGRecipeManager.h"
#include "FGUnlockSubsystem.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGPowerPoleWallHologram.h"
#include "FGPlayerController.h"
#include "Patching/NativeHookManager.h"


#pragma optimize("", off)
AHolo_WireHologramBuildModes::AHolo_WireHologramBuildModes()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHolo_WireHologramBuildModes::BeginPlay()
{
	GetRecipeManager();
	if (HasAuthority())
	{
		OnBuildModeChanged();
	}
//#if !WITH_EDITOR
	//Hook void SetActiveAutomaticPoleHologram( class AFGPowerPoleHologram* poleHologram );
//#endif

	Super::BeginPlay();
}

void AHolo_WireHologramBuildModes::OnBuildModeChanged()
{
	//Super::OnBuildModeChanged();
	if (RecipeManager)
	{
		int32 PoleIndex = PoleBuildModes.Find(GetCurrentBuildMode());
		TSubclassOf<UFGRecipe> PoleRecipe = DefaultPowerPoleRecipe;
		if (PowerPoleRecipes.IsValidIndex(PoleIndex))
		{
			const TSubclassOf<UFGRecipe> SelectedPoleRecipe = PowerPoleRecipes[PoleIndex];
			if (IsValid(SelectedPoleRecipe))
			{
				if (RecipeManager->IsRecipeAvailable(SelectedPoleRecipe))
				{
					PoleRecipe = SelectedPoleRecipe;
				}
			}
		}

		int32 WallIndex = WallBuildModes.Find(GetCurrentBuildMode());
		TSubclassOf<UFGRecipe> WallSocketRecipe = DefaultWallSocketRecipe;
		if (WallSocketRecipes.IsValidIndex(WallIndex))
		{
			const TSubclassOf<UFGRecipe> SelectedWallSocketRecipe = WallSocketRecipes[WallIndex];
			if (IsValid(SelectedWallSocketRecipe))
			{
				if (RecipeManager->IsRecipeAvailable(SelectedWallSocketRecipe))
				{
					WallSocketRecipe = SelectedWallSocketRecipe;
				}
			}
		}

		mDefaultPowerPoleRecipe = PoleRecipe;
		mDefaultPowerPoleWallRecipe = WallSocketRecipe;
		SwitchPoleType();
	}
}

void AHolo_WireHologramBuildModes::GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const
{
	//Super::GetSupportedBuildModes_Implementation(out_buildmodes);
	bool MustSnap = GetActiveAutomaticPoleHologram() != nullptr;
	bool SnapToWallHolo = false;
	if (MustSnap)
	{
		SnapToWallHolo = Cast<AFGPowerPoleWallHologram>(GetActiveAutomaticPoleHologram()) != nullptr;
	}

	if (AFGRecipeManager* lRecipeManager = AFGRecipeManager::Get(GetWorld()))
	{
		if (!SnapToWallHolo)
		{
			out_buildmodes.Empty();
			for (int32 Idx = 0; Idx < PowerPoleRecipes.Num(); ++Idx)
			{
				if (PoleBuildModes.IsValidIndex(Idx) && lRecipeManager->IsRecipeAvailable(PowerPoleRecipes[Idx]))
				{
					out_buildmodes.AddUnique(PoleBuildModes[Idx]);
				}
			}
		}
		else
		{
			out_buildmodes.Empty();
			for (int32 Idx = 0; Idx < WallSocketRecipes.Num(); ++Idx)
			{
				if (WallBuildModes.IsValidIndex(Idx) && lRecipeManager->IsRecipeAvailable(WallSocketRecipes[Idx]))
				{
					out_buildmodes.AddUnique(WallBuildModes[Idx]);
				}
			}
		}
	}
}

void AHolo_WireHologramBuildModes::SwitchPoleType()
{
	if (HasAuthority() && RecipeManager)
	{
		auto activePoleHG = GetActiveAutomaticPoleHologram();
		bool MustSnap = GetActiveAutomaticPoleHologram() != nullptr;
		bool SnapToWallHolo = false;
		if (MustSnap)
		{
			SnapToWallHolo = Cast<AFGPowerPoleWallHologram>(GetActiveAutomaticPoleHologram()) != nullptr;
		}
		auto constInst = GetConstructionInstigator();


		if (SnapToWallHolo && RecipeManager->IsRecipeAvailable(mDefaultPowerPoleWallRecipe))
		{
			for (AFGHologram* HologramChild : GetHologramChildren())
			{
				if (IsValid(HologramChild) && Cast<AFGPowerPoleWallHologram>(HologramChild))
				{
					mChildren.Remove(HologramChild);
					HologramChild->Destroy();
				}
			}
			mPowerPoleWall = Cast<AFGPowerPoleWallHologram>(SpawnChildHologramFromRecipe(this, mDefaultPowerPoleWallRecipe, constInst, FVector(), constInst));
			if (mPowerPoleWall)
			{
				mPowerPoleWall->SetDisabled(true);
			}
		}


		if (!SnapToWallHolo && RecipeManager->IsRecipeAvailable(mDefaultPowerPoleRecipe))
		{
			for (AFGHologram* HologramChild : GetHologramChildren())
			{
				if (IsValid(HologramChild) && !Cast<AFGPowerPoleWallHologram>(HologramChild) && Cast<AFGPowerPoleHologram>(HologramChild))
				{
					mChildren.Remove(HologramChild);
					HologramChild->Destroy();
				}
			}
			auto spawnedHG = AFGHologram::SpawnChildHologramFromRecipe(this, mDefaultPowerPoleRecipe, constInst, FVector(), constInst);
			mPowerPole = Cast<AFGPowerPoleHologram>(spawnedHG);
			if (mPowerPole)
			{
				mPowerPole->SetDisabled(true);
			}
		}

		if (MustSnap)
		{
			SetActiveAutomaticPoleHologram(SnapToWallHolo ? mPowerPoleWall : mPowerPole);
		}
	}
}

void AHolo_WireHologramBuildModes::GetRecipeManager()
{
	if (!RecipeManager)
	{
		RecipeManager = AFGRecipeManager::Get(GetWorld());
	}
}
#pragma optimize("", on)
