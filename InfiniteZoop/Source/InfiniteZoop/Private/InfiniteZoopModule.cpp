#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGLadderHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "Hologram/FGWallHologram.h"
#include "Hologram/FGWalkwayHologram.h"
#include "Hologram/FGPillarHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGResearchManager.h"
#include "FGPlayerController.h"
#include "Hologram/FGPillarHologram.h"
#include "FGGameState.h"
#include "FGResearchTree.h"
#include "FGResearchTreeNode.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Subsystem/SubsystemActorManager.h"
#include "InfiniteZoopSubsystem.h"
#include "InfiniteZoop_ClientSubsystem.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Hologram/FGHologramBuildModeDescriptor.h"
#include "InfiniteZoop_ConfigurationStruct.h"
#include "InfiniteZoop_RCO.h"
#include "Hologram/FGRampHologram.h"
#include <Logging/StructuredLog.h>


DEFINE_LOG_CATEGORY(InfiniteZoop_Log);
#pragma optimize("", off)
int GetClosestZoopAngle(double angleToCheck)
{
	if (angleToCheck >= double(0.0) && angleToCheck < double(90.0))
	{
		double firstNum = 0.0;
		double secondNum = 90.0;
		if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
		{
			return firstNum;
		}
		else
		{
			return secondNum;
		}
	}
	else if (angleToCheck >= double(90.0) && angleToCheck < double(180.0))
	{
		double firstNum = 90.0;
		double secondNum = 180.0;
		if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
		{
			return firstNum;
		}
		else
		{
			return secondNum;
		}
	}
	else if (angleToCheck >= double(180.0) && angleToCheck < double(270.0))
	{
		double firstNum = 180.0;
		double secondNum = 270.0;
		if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
		{
			return firstNum;
		}
		else
		{
			return secondNum;
		}
	}
	else if (angleToCheck >= double(270.0) && angleToCheck <= double(360.0))
	{
		double firstNum = 270.0;
		double secondNum = 360.0;
		if (abs(angleToCheck - firstNum) < abs(angleToCheck - secondNum))
		{
			return firstNum;
		}
		else
		{
			return 0;
		}
	}
	return -1;
}


void FInfiniteZoopModule::ScrollHologram(AFGHologram* self, int32 delta)
{
	auto pc = Cast<AFGPlayerController>(self->GetWorld()->GetFirstPlayerController());
	AFGFactoryBuildingHologram* Fhg = Cast<AFGFactoryBuildingHologram>(self);
	if (Fhg)
	{
		AFGFoundationHologram* foundation = Cast<AFGFoundationHologram>(self);
		AFGWallHologram* wall = Cast<AFGWallHologram>(self);
		auto currentZoop = Fhg->mDesiredZoop;
		if (currentZoop.IsZero())
		{
			if (wall && HologramsToZoop.Num() > 0)
			{
				HologramsToZoop.Remove(self);
			}
			if (foundation && FoundationsBeingZooped.Num() > 0 && FoundationsBeingZooped.Contains(foundation))
			{
				FoundationsBeingZooped[foundation]->inScrollMode = false;
			}
			return;
		}

		if (Fhg->IsCurrentBuildMode(Fhg->mBuildModeZoop))
		{
			auto invRotation = roundf(self->GetActorRotation().GetInverse().Yaw);
			if (foundation)
			{
				auto camInvRotator2 = pc->PlayerCameraManager->GetCameraRotation().Add(0, invRotation, 0).Yaw;
				auto newAngle = fmod(camInvRotator2, 360);
				if (newAngle < 0) newAngle += 360;
				auto lookedAtZoopAxis = GetClosestZoopAngle(newAngle);
				bool isXZoop = (lookedAtZoopAxis == 0 || lookedAtZoopAxis == 180);

				auto zStruct = FoundationsBeingZooped[foundation];
				currentZoop.X = zStruct->X;
				currentZoop.Y = zStruct->Y;
				currentZoop.Z = zStruct->Z;

				if (isXZoop)
				{
					if (currentZoop.X == 0) //Handle starting from 0
					{
						if (lookedAtZoopAxis == 0 && delta > 0)
						{
							currentZoop.X = 1;
						}
						else if (lookedAtZoopAxis == 180 && delta > 0)
						{
							currentZoop.X = -1;
						}
					}
					else
					{
						if (currentZoop.X > 0)
						{
							currentZoop.X = currentZoop.X + (1 * delta);
						}
						else
						{
							currentZoop.X = (abs(currentZoop.X) + (1 * delta)) * -1;
						}
					}
				}
				else
				{
					if (currentZoop.Y == 0) //Handle starting from 0
					{
						if (lookedAtZoopAxis == 90 && delta > 0)
						{
							currentZoop.Y = 1;
						}
						else if (lookedAtZoopAxis == 270 && delta > 0)
						{
							currentZoop.Y = -1;
						}
					}
					else
					{
						if (currentZoop.Y > 0)
						{
							currentZoop.Y = currentZoop.Y + (1 * delta);
						}
						else
						{
							currentZoop.Y = (abs(currentZoop.Y) + (1 * delta)) * -1;
						}
					}
				}

				zStruct->X = currentZoop.X;
				zStruct->Y = currentZoop.Y;
				zStruct->Z = currentZoop.Z;
			}
			else if (wall)
			{
				auto quat = self->GetActorQuat();
				auto camInvRotator = pc->PlayerCameraManager->GetCameraRotation().Add(0, invRotation, 0);
				auto axisY = CalcPivotAxis(EAxis::Y, camInvRotator.Vector(), quat);
				if (abs(axisY.X) < abs(axisY.Y))
				{

					if (currentZoop.Z == 0) //Handle starting from 0
					{
						if (axisY.Z >= 0 && delta > 0)
						{
							currentZoop.Z = 1;
						}
						else if (axisY.Z < 0 && delta > 0)
						{
							currentZoop.Z = -1;
						}
					}
					else
					{
						if (currentZoop.Z > 0)
						{
							currentZoop.Z = currentZoop.Z + (1 * delta);
						}
						else
						{
							currentZoop.Z = (abs(currentZoop.Z) + (1 * delta)) * -1;
						}
					}
				}
				if (abs(axisY.Y) < abs(axisY.X))
				{

					if (currentZoop.Y == 0) //Handle starting from 0
					{
						if (axisY.Y >= 0 && delta > 0)
						{
							currentZoop.Y = 1;
						}
						else if (axisY.Y < 0 && delta > 0)
						{
							currentZoop.Y = -1;
						}
					}
					else
					{
						if (currentZoop.Y > 0)
						{
							currentZoop.Y = currentZoop.Y + (1 * delta);
						}
						else
						{
							currentZoop.Y = (abs(currentZoop.Y) + (1 * delta)) * -1;
						}
					}
				}
			}
		}
		else
		{
			if (foundation && foundation->IsCurrentBuildMode(foundation->mBuildModeVerticalZoop))
			{
				if (currentZoop.Z > 0)
				{
					currentZoop.Z = currentZoop.Z + (1 * delta);
				}
				else
				{
					currentZoop.Z = (abs(currentZoop.Z) + (1 * delta)) * -1;
				}
			}
		}
		if (wall)
		{
			HologramsToZoop.Add(self, currentZoop);
		}
		else if (foundation && FoundationsBeingZooped.Contains(foundation) && Fhg->IsCurrentBuildMode(Fhg->mBuildModeZoop))
		{
			FoundationsBeingZooped[foundation]->inScrollMode = true;
		}
		Fhg->SetZoopAmount(currentZoop);
	}
}

bool FInfiniteZoopModule::SetZoopAmount(AFGFactoryBuildingHologram* self, const FIntVector& Zoop)
{
	auto baseHG = Cast<AFGHologram>(self);
	AFGFoundationHologram* foundation = Cast<AFGFoundationHologram>(self);
	AFGWallHologram* wall = Cast<AFGWallHologram>(self);
	if (baseHG && wall)
	{
		auto newZoop = HologramsToZoop.Find(baseHG);
		if (newZoop == nullptr || newZoop->IsZero())
		{
			return false;
		}
		if (newZoop && Zoop != *newZoop)
		{
			return true; //scope.Cancel();
		}
	}
	else if (foundation)
	{
		if (FoundationsBeingZooped.Num() > 0 && FoundationsBeingZooped.Contains(foundation))
		{
			auto zStruct = FoundationsBeingZooped[foundation];
			FIntVector newZoop = FIntVector(zStruct->X, zStruct->Y, zStruct->Z);
			if (newZoop.IsZero() || !zStruct->inScrollMode)
			{
				return false;
			}
			if (zStruct->inScrollMode && (Zoop.X != newZoop.X || Zoop.Y != newZoop.Y || Zoop.Z != newZoop.Z))
			{
				return true; //scope.Cancel();
			}
		}
	}
	return false;
}

void FInfiniteZoopModule::OnZoopUpdated(UFGBuildGunStateBuild* self, float currentZoop, float maxZoop, const FVector& zoopLocation)
{
	auto hg = self->GetHologram();
	auto fbhg = Cast< AFGFactoryBuildingHologram>(hg);
	if (fbhg)
	{
		auto mult = fbhg->GetBaseCostMultiplier();
		auto dz = fbhg->mDesiredZoop;
	}
}

bool FInfiniteZoopModule::OnRep_DesiredZoop(AFGFactoryBuildingHologram* self)
{
	auto fhg = Cast< AFGFoundationHologram>(self);
	if (fhg)
	{
		if (IsZoopMode(fhg))
		{
			//Something here is what stopped vertical zoop from following the mouse
			auto material = self->mPlacementMaterialState;
			auto zStruct = GetStruct(fhg);
			if (!zStruct->secondPassComplete)
			{
			zStruct->hgMaterialState1 = material;
			}
			else
			{
				zStruct->hgMaterialState2 = material;
			}

			self->SetMaterialState(self->GetHologramMaterialState());

			if (zStruct->hgMaterialState1 == EHologramMaterialState::HMS_ERROR || zStruct->hgMaterialState2 == EHologramMaterialState::HMS_ERROR)
			{
				self->SetMaterialState(EHologramMaterialState::HMS_ERROR);
			}
			else if (zStruct->hgMaterialState1 == EHologramMaterialState::HMS_WARNING || zStruct->hgMaterialState2 == EHologramMaterialState::HMS_WARNING)
			{
				self->SetMaterialState(EHologramMaterialState::HMS_WARNING);
			}
			else
			{
				self->SetMaterialState(EHologramMaterialState::HMS_OK);
			}
			if (!zStruct->secondPassComplete && !zStruct->inScrollMode)
			{
				return false;
			}
			else
			{
				if (zStruct->secondPassComplete || zStruct->inScrollMode)
				{
					//UE_LOGFMT(InfiniteZoop_Log, Display, "Max Zoop Amount: {0}", self->mMaxZoopAmount);
					auto maxZ = self->mMaxZoopAmount;
					auto minZ = (self->mMaxZoopAmount * -1);
					self->mDesiredZoop.X = FMath::Clamp(zStruct->X, minZ, maxZ);
					self->mDesiredZoop.Y = FMath::Clamp(zStruct->Y, minZ, maxZ);
					self->mDesiredZoop.Z = FMath::Clamp(zStruct->Z, minZ, maxZ);

					zStruct->firstPassComplete = false;
					zStruct->secondPassComplete = false;
					FoundationsBeingZooped[fhg] = zStruct;

					SetSubsystemZoopAmounts(self->mDesiredZoop.X, self->mDesiredZoop.Y, self->mDesiredZoop.Z, true, self->GetWorld(), self);
				}
				self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_None;
				self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_None;
			}
		}
		else
		{
			self->SetMaterialState(self->GetHologramMaterialState());

			SetSubsystemZoopAmounts(-1, -1, -1, true, self->GetWorld(), self);
		}
	}
	else if (auto wallHolo = Cast<AFGWallHologram>(self))
	{
		SetSubsystemZoopAmounts(self->mDesiredZoop.X, self->mDesiredZoop.Y, self->mDesiredZoop.Z, false, self->GetWorld(), self);
	}
	return true;
}

void FInfiniteZoopModule::CreateDefaultFoundationZoop(AFGFoundationHologram* self, const FHitResult& hitResult)
{
	if (IsZoopMode(self) && FoundationsBeingZooped.Num() > 0 && FoundationsBeingZooped.Contains(self) &&  !FoundationsBeingZooped[self]->inScrollMode)
	{
		auto zStruct = GetStruct(self);

		//UE_LOG(InfiniteZoop_Log, Display, TEXT("CreateDefaultFoundationZoop: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
		auto x = abs(self->mDesiredZoop.X);
		auto y = abs(self->mDesiredZoop.Y);
		if (x > y)
		{
			zStruct->X = self->mDesiredZoop.X;
			self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_Forward + (uint8)EHologramZoopDirections::HZD_Backward;
			self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_Forward + (uint8)EHologramZoopDirections::HZD_Backward;
		}
		else if (y > x)
		{
			zStruct->Y = self->mDesiredZoop.Y;
			self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_Left + (uint8)EHologramZoopDirections::HZD_Right;
			self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_Left + (uint8)EHologramZoopDirections::HZD_Right;
		}
		else if (y == 0 && x == 0)
		{
			if (zStruct->firstPassComplete)
			{
				if (self->mBlockedZoopDirectionMask == (uint8)EHologramZoopDirections::HZD_Left + (uint8)EHologramZoopDirections::HZD_Right)
				{
					zStruct->X = self->mDesiredZoop.X;
				}
				else if (self->mBlockedZoopDirectionMask == (uint8)EHologramZoopDirections::HZD_Forward + (uint8)EHologramZoopDirections::HZD_Backward)
				{
					zStruct->Y = self->mDesiredZoop.Y;
				}
				else
				{
					zStruct->X = self->mDesiredZoop.X;
					zStruct->Y = self->mDesiredZoop.Y;
				}
			}
			self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_None;
			self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_None;
		}
		else
		{
			self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_None;
			self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_None;
		}
		zStruct->Z = self->mDesiredZoop.Z;
		if (!zStruct->firstPassComplete)
		{
			zStruct->firstPassComplete = true;
			//zStruct->hgMaterialState1 = self->GetHologramMaterialState();
		}
		else
		{
			zStruct->secondPassComplete = true;
			//zStruct->hgMaterialState2 = self->GetHologramMaterialState();
		}

		FoundationsBeingZooped[self] = zStruct;
		self->OnRep_DesiredZoop();
	}
}

int32 FInfiniteZoopModule::GetBaseCostMultiplier(const AFGFactoryBuildingHologram* self)
{

	auto nonConst = const_cast<AFGFactoryBuildingHologram*>(self);
	auto fhg = Cast< AFGFoundationHologram>(nonConst);
	if (fhg)
	{
		auto zStruct = GetStruct(fhg);
		auto x = abs(zStruct->X);
		auto y = abs(zStruct->Y);
		auto z = abs(zStruct->Z);
		x = x > 0 ? x + 1 : 1;
		y = y > 0 ? y + 1 : 1;
		z = z > 0 ? z + 1 : 1;
		auto result = (x * y * z);
		return result;
	}

	//UE_LOG(InfiniteZoop_Log, Display, TEXT("GetBaseCostMultiplier: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
	auto x = abs(self->mDesiredZoop.X);
	auto y = abs(self->mDesiredZoop.Y);
	auto z = abs(self->mDesiredZoop.Z);
	x = x > 0 ? x + 1 : 1;
	y = y > 0 ? y + 1 : 1;
	z = z > 0 ? z + 1 : 1;
	auto result = (x * y * z);
	return result;
}

void FInfiniteZoopModule::ConstructZoop(AFGFoundationHologram* self, TArray<AActor*>& out_children)
{
	//UWorld* world = self->GetWorld();
	//UE_LOGFMT(InfiniteZoop_Log, Display, "FInfiniteZoopModule::ConstructZoop. zoopSubsystem->currentZoopAmount: {0}", zoopSubsystem->currentZoopAmount);
	if (IsZoopMode(self))
	{
		auto zStruct = GetStruct(self);
		if (self)
		{
			//UE_LOGFMT(InfiniteZoop_Log, Display, "Max Zoop Amount: {0}", self->mMaxZoopAmount);
			auto maxZ = self->mMaxZoopAmount;
			auto minZ = (self->mMaxZoopAmount * -1);
			self->mDesiredZoop.X = FMath::Clamp(zStruct->X, minZ, maxZ);
			self->mDesiredZoop.Y = FMath::Clamp(zStruct->Y, minZ, maxZ);
			self->mDesiredZoop.Z = FMath::Clamp(zStruct->Z, minZ, maxZ);
			//UE_LOG(InfiniteZoop_Log, Display, TEXT("ConstructZoop: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
			self->OnRep_DesiredZoop();
			//UE_LOG(InfiniteZoop_Log, Display, TEXT("ConstructZoop 2: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);

			zStruct->firstPassComplete = false;
			zStruct->secondPassComplete = false;
			//zStruct->hgMaterialState1 = EHologramMaterialState::HMS_OK;
			//zStruct->hgMaterialState2 = EHologramMaterialState::HMS_OK;
			//zStruct->X = 0;
			//zStruct->Y = 0;
			//zStruct->Z = 0;
			FoundationsBeingZooped[self] = zStruct;


		}
	}
}

FZoopStruct* FInfiniteZoopModule::GetStruct(AFGFoundationHologram* self)
{
	FZoopStruct* zStruct = new FZoopStruct();
	if (FoundationsBeingZooped.Contains(self))
	{
		zStruct = FoundationsBeingZooped[self];
	}
	else
	{
		FoundationsBeingZooped.Add(self, zStruct);
	}
	return zStruct;
}

bool FInfiniteZoopModule::IsZoopMode(AFGFoundationHologram* self)
{
	auto vertMode = self->mBuildModeVerticalZoop;
	if (self->IsCurrentBuildMode(vertMode))
	{
		return false;
	}
	return true;
}

bool FInfiniteZoopModule::BGSecondaryFire(UFGBuildGunStateBuild* self)
{
	
	if (auto hologram = Cast<AFGFactoryBuildingHologram>(self->GetHologram()))
	{
		if (auto foundation = Cast<AFGFoundationHologram>(hologram))
		{
			if (Cast<AFGRampHologram>(hologram) || FoundationsBeingZooped.Num() <= 0)
			{
				return true;
			}
			if (FoundationsBeingZooped.Contains(foundation))
			{
				auto zStruct = FoundationsBeingZooped[foundation];
				if (zStruct->inScrollMode)
				{
					zStruct->inScrollMode = false;
					return false;
				}
			}
		}
		else if (auto wall = Cast< AFGWallHologram>(hologram))
		{
			if (HologramsToZoop.Contains(hologram))
			{
				HologramsToZoop.Remove(hologram);
				return false;
			}
		}

		SetSubsystemZoopAmounts(-1, -1, -1, true, self->GetWorld(), hologram);
	}
	return true;
}

bool FInfiniteZoopModule::ValidatePlacementAndCost(AFGHologram* self, class UFGInventoryComponent* inventory)
{
	if (auto foundation = Cast<AFGFoundationHologram>(self))
	{
		if (FoundationsBeingZooped.Contains(foundation))
		{
			auto zStruct = FoundationsBeingZooped[foundation];
			if (zStruct->secondPassComplete)
			{
				return true;
			}
			return false;
		}
	}
	return true;
}

void FInfiniteZoopModule::CheckBuildEffects(const AFGFactoryBuildingHologram* fbHolo, class AFGBuildable* inBuildable)
{
	auto x = FMath::Clamp(FMath::Abs(fbHolo->mDesiredZoop.X), 1, 99999);
	auto y = FMath::Clamp(FMath::Abs(fbHolo->mDesiredZoop.Y), 1, 99999);
	auto z = FMath::Clamp(FMath::Abs(fbHolo->mDesiredZoop.Z), 1, 99999);
	auto total = x * y * z;
	if (total > 10)
	{
		auto config = FInfiniteZoop_ConfigurationStruct::GetActiveConfig(fbHolo->GetWorld());
		if (config.BuildEffects.SkipEffects && total > config.BuildEffects.SkipAfterAmount)
		{
			inBuildable->mSkipBuildEffect = true;
		}
	}
}

void FInfiniteZoopModule::HGBeginPlay(AFGHologram* self)
{
	//TArray< TSubclassOf< UFGBuildGunModeDescriptor > > out_buildmodes;
	//self->GetSupportedBuildModes(out_buildmodes);
	//out_buildmodes.Num();
	if (self)
	{
		TArray< TSubclassOf< UFGBuildGunModeDescriptor > > out_buildmodes;
		self->GetSupportedBuildModes(out_buildmodes);
		if (out_buildmodes.Num() == 0)
		{
			return;
		}
		UWorld* world = self->GetWorld();
		USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
		AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
		AInfiniteZoop_ClientSubsystem* clientSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoop_ClientSubsystem>();

		//UE_LOGFMT(InfiniteZoop_Log, Display, "AFGHologram::BeginPlay. zoopSubsystem->currentZoopAmount: {0}", zoopSubsystem->currentZoopAmount);


		AFGWallHologram* Whg = Cast<AFGWallHologram>(self);
		if (Whg)
		{
			Whg->mOnlyAllowLineZoop = false;
		}

		AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);

		if (bhg)
		{
			//UE_LOGFMT(InfiniteZoop_Log, Display, "self->HasAuthority(): {0}", self->HasAuthority());
			if (!self->GetInstigatorController())
			{
				AFGPlayerController* PlayerController = Cast<AFGPlayerController>(self->GetWorld()->GetFirstPlayerController());
				UInfiniteZoop_RCO* RCO = PlayerController->GetRemoteCallObjectOfClass<UInfiniteZoop_RCO>();
				if (RCO)
				{
					//UE_LOGFMT(InfiniteZoop_Log, Display, "RCO Setting max zoop to {0}", zoopSubsystem->currentZoopAmount - 1);
					RCO->SetHologramMaxZoop(bhg, zoopSubsystem->currentZoopAmount - 1);
				}
			}
		}
		if (clientSubsystem)
		{
			//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: False"));
			if (bhg && bhg->mMaxZoopAmount > 0)
			{

				bhg->SetMaterialState(self->GetHologramMaterialState());
				if (clientSubsystem->tempZoopAmount > 0)
				{
					bhg->mMaxZoopAmount = clientSubsystem->tempZoopAmount - 1;
				}
				else
				{
					bhg->mMaxZoopAmount = zoopSubsystem->currentZoopAmount - 1;
				}
				//UE_LOG(InfiniteZoop_Log, Display, TEXT("Increased Zoop Amount"));
				return;
			}
		}
		AFGLadderHologram* ladderHG = Cast<AFGLadderHologram>(self);
		if (ladderHG && clientSubsystem)
		{
			if (clientSubsystem->tempZoopAmount > 0)
			{
				ladderHG->mMaxSegmentCount = clientSubsystem->tempZoopAmount - 1;
			}
			else if (zoopSubsystem)
			{
				ladderHG->mMaxSegmentCount = zoopSubsystem->currentZoopAmount - 1;
			}
		}
	}
}

void FInfiniteZoopModule::StartupModule() 
{
#if !WITH_EDITOR
	lockObj = new FCriticalSection();

	AFGBuildableHologram* bhg = GetMutableDefault<AFGBuildableHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableHologram::ConfigureActor, bhg, [=](auto& scope, const AFGBuildableHologram* self, class AFGBuildable* inBuildable)
		{
			if (self->CanBeZooped())
			{
				auto fbHolo = Cast<AFGFactoryBuildingHologram>(self);
				if (fbHolo)
				{
					CheckBuildEffects(fbHolo, inBuildable);
				}
			}
		});

	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::OnRep_DesiredZoop, [=](auto& scope, AFGFactoryBuildingHologram* self)
		{
			if (auto ramphg = Cast<AFGRampHologram>(self))
			{
				return;
			}
	if (!this->OnRep_DesiredZoop(self))
	{
		scope.Cancel();
	}
		});

	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::SetZoopAmount, [=](auto& scope, AFGFactoryBuildingHologram* self, const FIntVector& Zoop)
		{
			if (auto ramphg = Cast<AFGRampHologram>(self))
			{
				return;
			}
			if (this->SetZoopAmount(self, Zoop))
			{
				scope.Cancel();
			}
		});
	SUBSCRIBE_METHOD(UFGBuildGunStateBuild::OnZoopUpdated, [=](auto& scope, UFGBuildGunStateBuild* self, float currentZoop, float maxZoop, const FVector& zoopLocation)
		{
			// Fix for incorrect # showing when 2D zooping
			auto hg = self->GetHologram();
	auto fbhg = Cast< AFGFactoryBuildingHologram>(hg);
	if (fbhg)
	{
		if (auto ramphg = Cast<AFGRampHologram>(fbhg))
		{
			return;
		}
		auto mult = fbhg->GetBaseCostMultiplier();
		if (LastMultiplier.Num() > 0)
		{
			if (LastMultiplier.Contains(fbhg))
			{
				if (LastMultiplier[fbhg] == mult)
				{
					scope.Cancel();
					return;
				}
				LastMultiplier[fbhg] = mult;
			}
			else
			{
				LastMultiplier.Add(fbhg, mult);
			}
		}
		else
		{
			LastMultiplier.Add(fbhg, mult);
		}
		mult = mult > 0 ? mult - 1 : 0;
		scope(self, (float)mult, maxZoop, zoopLocation);
	}
		});

	SUBSCRIBE_METHOD(UFGBuildGunState::SecondaryFire, [=](auto& scope, UFGBuildGunState* self)
		{
			if (auto bgsb = Cast< UFGBuildGunStateBuild>(self))
			{
				if (bgsb && !BGSecondaryFire(bgsb))
				{
					scope.Cancel();
				}
			}
		});
	UFGBuildGunStateBuild* bgbCDO = GetMutableDefault<UFGBuildGunStateBuild>();
	//SUBSCRIBE_METHOD_VIRTUAL(UFGBuildGunStateBuild::SecondaryFire_Implementation, bgbCDO, [=](auto& scope, UFGBuildGunStateBuild* self)
	//	{
	//		if (!BGSecondaryFire(self))
	//		{
	//			scope.Cancel();
	//		}
	//	});

	AFGFactoryBuildingHologram* fbhgCDO = GetMutableDefault<AFGFactoryBuildingHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGFactoryBuildingHologram::GetBaseCostMultiplier, fbhgCDO, [=](auto& scope, const AFGFactoryBuildingHologram* self)
		{
			if (auto ramphg = Cast<AFGRampHologram>(self))
			{
				return;
			}
	if (auto fhg = Cast< AFGFoundationHologram>(self))
	{
		if (self->IsCurrentBuildMode(fhg->mBuildModeVerticalZoop))
		{
			return;
		}
	}
	auto result = this->GetBaseCostMultiplier(self);
	scope.Override(result);
		});

	AFGFoundationHologram* fhCDO = GetMutableDefault<AFGFoundationHologram>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::ConstructZoop, fhCDO, [=](auto& scope, AFGFoundationHologram* self, TArray<AActor*>& out_children)
		{
			ConstructZoop(self, out_children);
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGFoundationHologram::CreateDefaultFoundationZoop, fhCDO, [=](AFGFoundationHologram* self, const FHitResult& hitResult)
		{
			CreateDefaultFoundationZoop(self, hitResult);
		});

	//void ValidatePlacementAndCost(class UFGInventoryComponent* inventory);
	SUBSCRIBE_METHOD(AFGHologram::ValidatePlacementAndCost, [=](auto scope, AFGHologram* self, class UFGInventoryComponent* inventory)
		{
			if (!ValidatePlacementAndCost(self, inventory))
			{
				scope.Cancel();
			}
		});

	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, hCDO, [=](auto scope, AFGHologram* self, int32 delta)
		{
			if (auto ramphg = Cast<AFGRampHologram>(self))
			{
				return;
			}
			if (self->IsHologramLocked())
			{
				return;
			}

			this->ScrollHologram(self, delta);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Destroyed, hCDO, [=](auto& scope, AFGHologram* self)
		{
			if (self->CanBeZooped())
			{
				if (Cast<AFGWallHologram>(self))
				{
					HologramsToZoop.Remove(self);
				}
				else if (auto fhg = Cast<AFGFoundationHologram>(self))
				{
					FoundationsBeingZooped.Remove(fhg);
				}

				if (auto fbhg = Cast<AFGFactoryBuildingHologram>(self))
				{
					if (LastMultiplier.Num() > 0)
					{
						if (LastMultiplier.Contains(fbhg))
						{
							LastMultiplier.Remove(fbhg);
						}
					}
				}

				SetSubsystemZoopAmounts(-1, -1, -1, true, self->GetWorld(), self);
			}
		});

	//TODO Figure out how to do this differently
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::OnBuildModeChanged, hCDO, [=](auto& scope, AFGHologram* self, TSubclassOf<UFGHologramBuildModeDescriptor> buildMode)
	//	{
	//		if (self->CanBeZooped())
	//		{
	//			if (Cast<AFGWallHologram>(self))
	//			{
	//				HologramsToZoop.Remove(self);
	//			}
	//			else if (auto fhg = Cast<AFGFoundationHologram>(self))
	//			{
	//				FoundationsBeingZooped.Remove(fhg);

	//				fhg->mDesiredZoop.X = 0;
	//				fhg->mDesiredZoop.Y = 0;
	//				fhg->mDesiredZoop.Z = 0;
	//				fhg->SetZoopAmount(FIntVector(0, 0, 0));
	//				fhg->UpdateZoop();
	//			}

	//		}
	//		SetSubsystemZoopAmounts(-1, -1, -1, true, self->GetWorld(), self);
	//	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [=](auto scope, AFGHologram* self)
		{
			HGBeginPlay(self);
			
		});

	AFGPillarHologram* pCDO = GetMutableDefault<AFGPillarHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPillarHologram::BeginPlay, pCDO, [](auto scope, AFGPillarHologram* self)
		{
			if (self && self->HasAuthority())
			{
				UWorld* world = self->GetWorld();
				if (world)
				{
					USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
					if (SubsystemActorManager)
					{
						AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
						if (zoopSubsystem)
						{
							AInfiniteZoop_ClientSubsystem* clientSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoop_ClientSubsystem>();
							if (clientSubsystem)
							{
								if (clientSubsystem->tempZoopAmount > 0)
								{
									self->mMaxZoop = clientSubsystem->tempZoopAmount - 1;
								}
								else
								{
									self->mMaxZoop = zoopSubsystem->currentZoopAmount - 1;
								}
							}
						}
					}
				}
			}
		});
#endif
}

FVector FInfiniteZoopModule::CalcPivotAxis(const EAxis::Type DesiredAxis, const FVector& ViewVector, const FQuat& ActorQuat)
{
	auto ProcessAxes = [&](const FVector& VAxis, const FVector& Axis1, const FVector& Axis2) -> FVector
	{
		int Inverted = FMath::Sign(VAxis.Z);
		if (DesiredAxis == EAxis::Z) {
			return Inverted * VAxis;
		}
		float Check1 = FVector(Axis1.X, Axis1.Y, 0.f).GetSafeNormal() | ViewVector;
		float Check2 = FVector(Axis2.X, Axis2.Y, 0.f).GetSafeNormal() | ViewVector;
		if (FMath::Abs(Check1) >= FMath::Abs(Check2)) {
			return FMath::Sign(Check1) * ((DesiredAxis == EAxis::X) ? Axis1 : (Inverted * Axis2));
		}
		return FMath::Sign(Check2) * ((DesiredAxis == EAxis::X) ? Axis2 : (Inverted * -Axis1));
	};

	FVector XAxis = ActorQuat.GetAxisX();
	FVector YAxis = ActorQuat.GetAxisY();
	FVector ZAxis = ActorQuat.GetAxisZ();
	if (FMath::Abs(ZAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(ZAxis, XAxis, YAxis);
	}
	else if (FMath::Abs(YAxis | FVector::UpVector) >= UE_HALF_SQRT_2) {
		return ProcessAxes(YAxis, ZAxis, XAxis);
	}
	return ProcessAxes(XAxis, YAxis, ZAxis);
}

void FInfiniteZoopModule::SetSubsystemZoopAmounts(int x, int y, int z, bool isFoundation, UWorld* world, AFGHologram* hologram)
{
	USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
	AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
	bool isZoopMode = false;
	bool isVerticalMode = false;
	if (isFoundation)
	{
		if (auto fhg = Cast<AFGFoundationHologram>(hologram))
		{
			auto vertMode = fhg->mBuildModeVerticalZoop;
			auto zoopMode = fhg->mBuildModeZoop;
			if (fhg->IsCurrentBuildMode(vertMode) || fhg->IsCurrentBuildMode(zoopMode))
			{
				isZoopMode = true;
			}
			isVerticalMode = fhg->IsCurrentBuildMode(vertMode);
		}
	}
	else
	{
		if (auto whg = Cast<AFGWallHologram>(hologram))
		{
			auto zoopMode = whg->mBuildModeZoop;
			if (whg->IsCurrentBuildMode(zoopMode))
			{
				isZoopMode = true;
			}
		}
	}
	if (!isZoopMode)
	{
		zoopSubsystem->SetPublicZoopAmount(-1, -1, -1, isFoundation, isVerticalMode, hologram->GetConstructionInstigator(), lockObj);
		return;
	}
	int newX = FMath::Abs(x);
	int newY = FMath::Abs(y);
	int newZ = FMath::Abs(z);
	if (newX >= 0)
	{
		newX++;
	}
	if (newY >= 0)
	{
		newY++;
	}
	if (newZ >= 0)
	{
		newZ++;
	}
	if (isFoundation && newX == 1 && newY == 1)
	{
		newX = -1;
		newY = -1;
	}
	
	zoopSubsystem->SetPublicZoopAmount(newX, newY, newZ, isFoundation, isVerticalMode, hologram->GetConstructionInstigator(), lockObj);
}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);