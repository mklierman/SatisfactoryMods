#include "Holo_WireHologramBuildModes.h"
#include "FGRecipeManager.h"
#include "FGUnlockSubsystem.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Hologram/FGPowerPoleWallHologram.h"
#include "FGPlayerController.h"
#include "Patching/NativeHookManager.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Equipment/FGBuildGun.h"
#include "Equipment/FGBuildGunBuild.h"
#include "CP_Subsystem.h"


//#pragma optimize("", off)
AHolo_WireHologramBuildModes::AHolo_WireHologramBuildModes()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHolo_WireHologramBuildModes::BeginPlay()
{
	GetRecipeManager();
	if (HasAuthority())
	{
		USubsystemActorManager* SubsystemActorManager = this->GetWorld()->GetSubsystem<USubsystemActorManager>();
		if (SubsystemActorManager)
		{
			ACP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACP_Subsystem>();
			if (subsystem)
			{
				CP_Subsystem = subsystem;
			}
		}

		ConstructionInstigator = GetConstructionInstigator();
		RespawnChildPoleHolograms();
		RespawnChildWallSocketHolograms();

		
		this->SetBuildMode(mDefaultBuildMode);
		OnBuildModeChanged();

	}

	Super::BeginPlay();
}

void AHolo_WireHologramBuildModes::OnBuildModeChanged()
{
	Super::OnBuildModeChanged();
	if (RecipeManager)
	{
		TSubclassOf<UFGRecipe> PoleRecipe = DefaultPowerPoleRecipe;
		TSubclassOf<UFGRecipe> WallSocketRecipe = DefaultWallSocketRecipe;

		if (!RecipeManager->IsRecipeAvailable(DefaultWallSocketRecipe))
		{
			for (auto bmode : WireBuildModes)
			{
				if (bmode.IsWallSocket)
				{
					if (RecipeManager->IsRecipeAvailable(bmode.Recipe))
					{
						WallSocketRecipe = bmode.Recipe;
						mDefaultPowerPoleWallRecipe = WallSocketRecipe;
						break;
					}
				}
			}
		}

		auto currentBuildMode = GetCurrentBuildMode();
		for (auto bm : WireBuildModes)
		{
			if (bm.BuildMode == currentBuildMode)
			{
				if (!bm.IsWallSocket)
				{
					const TSubclassOf<UFGRecipe> SelectedPoleRecipe = bm.Recipe;
					if (IsValid(SelectedPoleRecipe))
					{
						if (RecipeManager->IsRecipeAvailable(SelectedPoleRecipe))
						{
							PoleRecipe = SelectedPoleRecipe;
							if (GetActiveAutomaticPoleHologram())
							{
								CP_Subsystem->LastUsedPoleBuildMode = bm;
							}
						}
					}
				}
				else
				{
					const TSubclassOf<UFGRecipe> SelectedWallSocketRecipe = bm.Recipe;
					if (IsValid(SelectedWallSocketRecipe))
					{
						if (RecipeManager->IsRecipeAvailable(SelectedWallSocketRecipe))
						{
							WallSocketRecipe = SelectedWallSocketRecipe;
							if (GetActiveAutomaticPoleHologram())
							{
								CP_Subsystem->LastUsedWallSockedBuildMode = bm;
							}
						}
					}
				}

				mDefaultPowerPoleRecipe = PoleRecipe;
				mDefaultPowerPoleWallRecipe = WallSocketRecipe;
				break;
			}
		}

		SwitchPoleType();
	}
}

void AHolo_WireHologramBuildModes::GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGHologramBuildModeDescriptor>>& out_buildmodes) const
{
	//Super::GetSupportedBuildModes_Implementation(out_buildmodes);
	bool MustSnap = GetActiveAutomaticPoleHologram() != nullptr;
	bool SnapToWallHolo = false;
	bool SnappedToFoundation = false;
	if (MustSnap)
	{
		SnapToWallHolo = Cast<AFGPowerPoleWallHologram>(GetActiveAutomaticPoleHologram()) != nullptr;
		SnappedToFoundation = Cast<AFGBuildableFoundation>(GetActiveAutomaticPoleHologram()->GetSnappedBuilding()) != nullptr;
	}

	if (AFGRecipeManager* lRecipeManager = AFGRecipeManager::Get(GetWorld()))
	{
		out_buildmodes.Empty();

		if (!SnapToWallHolo)
		{
			for (auto bm : WireBuildModes)
			{
				if (!bm.IsWallSocket && lRecipeManager->IsRecipeAvailable(bm.Recipe))
				{
					out_buildmodes.AddUnique(bm.BuildMode);
				}
			}
		}
		else
		{
			for (auto bm : WireBuildModes)
			{
				if (bm.IsWallSocket && lRecipeManager->IsRecipeAvailable(bm.Recipe))
				{
					if (SnappedToFoundation && !bm.IsDoubleWallSocket)
					{
						out_buildmodes.AddUnique(bm.BuildMode);
					}
					else if (!SnappedToFoundation)
					{
						out_buildmodes.AddUnique(bm.BuildMode);
					}
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


		if ((SnapToWallHolo && RecipeManager->IsRecipeAvailable(mDefaultPowerPoleWallRecipe)) || 1 == 1)
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


		if ((!SnapToWallHolo && RecipeManager->IsRecipeAvailable(mDefaultPowerPoleRecipe)) || 1 == 1)
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

void AHolo_WireHologramBuildModes::RespawnChildPoleHolograms()
{
	if (CP_Subsystem->LastUsedPoleBuildMode.BuildMode)
	{
		this->mDefaultBuildMode = CP_Subsystem->LastUsedPoleBuildMode.BuildMode;
		mDefaultPowerPoleRecipe = CP_Subsystem->LastUsedPoleBuildMode.Recipe;

		auto spawnedHG = AFGHologram::SpawnChildHologramFromRecipe(this, mDefaultPowerPoleRecipe, ConstructionInstigator, FVector(), ConstructionInstigator);
		mPowerPole = Cast<AFGPowerPoleHologram>(spawnedHG);
		if (mPowerPole)
		{
			mPowerPole->SetDisabled(true);
		}
	}
}

void AHolo_WireHologramBuildModes::RespawnChildWallSocketHolograms()
{
	if (CP_Subsystem->LastUsedWallSockedBuildMode.BuildMode)
	{
		mDefaultPowerPoleWallRecipe = CP_Subsystem->LastUsedWallSockedBuildMode.Recipe;
		DefaultWallSocketRecipe = CP_Subsystem->LastUsedWallSockedBuildMode.Recipe;
		mPowerPoleWall = Cast<AFGPowerPoleWallHologram>(SpawnChildHologramFromRecipe(this, mDefaultPowerPoleWallRecipe, ConstructionInstigator, FVector(), ConstructionInstigator));
		if (mPowerPoleWall)
		{
			mPowerPoleWall->SetDisabled(true);
		}
	}
	else
	{
		SetSafeDefaultWallSocket();
	}
}

void AHolo_WireHologramBuildModes::SetSafeDefaultWallSocket()
{
	if (!RecipeManager->IsRecipeAvailable(DefaultWallSocketRecipe))
	{
		for (auto bmode : WireBuildModes)
		{
			if (bmode.IsWallSocket)
			{
				if (RecipeManager->IsRecipeAvailable(bmode.Recipe))
				{
					mDefaultPowerPoleWallRecipe = bmode.Recipe;
					mPowerPoleWall = Cast<AFGPowerPoleWallHologram>(SpawnChildHologramFromRecipe(this, mDefaultPowerPoleWallRecipe, ConstructionInstigator, FVector(), ConstructionInstigator));
					if (mPowerPoleWall)
					{
						mPowerPoleWall->SetDisabled(true);
					}
					break;
				}
			}
		}
	}
}
//#pragma optimize("", on)
