#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGLadderHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "Hologram/FGWallHologram.h"
#include "Hologram/FGWalkwayHologram.h"
#include "FGPillarHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGResearchManager.h"
#include "FGPlayerController.h"
#include "FGPillarHologram.h"
#include "FGGameState.h"
#include "FGResearchTree.h"
#include "FGResearchTreeNode.h"
#include "Equipment/FGBuildGunBuild.h"
#include "Subsystem/SubsystemActorManager.h"
#include "InfiniteZoopSubsystem.h"
#include "InfiniteZoop_ClientSubsystem.h"
#include "Equipment/FGBuildGunBuild.h"
#include "FGHologramBuildModeDescriptor.h"


DEFINE_LOG_CATEGORY(InfiniteZoop_Log);
#pragma optimize("", off)
void FInfiniteZoopModule::ScrollHologram(AFGHologram* self, int32 delta)
{
	auto pc = Cast<AFGPlayerController>(self->GetWorld()->GetFirstPlayerController());
	bool isShiftDown = pc->IsInputKeyDown(EKeys::LeftShift);
	AFGFactoryBuildingHologram* Fhg = Cast<AFGFactoryBuildingHologram>(self);
	if (Fhg)
	{
		auto currentZoop = Fhg->mDesiredZoop;
		if (currentZoop.IsZero())
		{
			HologramsToZoop.Remove(self);
			return;
		}

		if (Fhg->GetCurrentBuildMode() == Fhg->mBuildModeZoop)
		{
			AFGFoundationHologram* foundation = Cast<AFGFoundationHologram>(self);
			auto quat = self->GetActorQuat();
			auto loc = self->GetActorLocation().ToOrientationQuat();
			auto axisX = CalcPivotAxis(EAxis::X, pc->PlayerCameraManager->GetCameraRotation().Vector(), self->GetActorQuat());
			auto axisY = CalcPivotAxis(EAxis::Y, pc->PlayerCameraManager->GetCameraRotation().Vector(), self->GetActorQuat());
			if (foundation)
			{
				if (abs(axisX.X) > abs(axisX.Y))
				{
					if (currentZoop.X == 0) //Handle starting from 0
					{
						if (axisX.X >= 0 && delta > 0)
						{
							currentZoop.X = 1;
						}
						else if (axisX.X < 0 && delta > 0)
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
				if (abs(axisX.Y) > abs(axisX.X))
				{
					if (currentZoop.Y == 0) //Handle starting from 0
					{
						if (axisX.Y >= 0 && delta > 0)
						{
							currentZoop.Y = 1;
						}
						else if (axisX.Y < 0 && delta > 0)
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
			else if (auto wall = Cast<AFGWallHologram>(self))
			{
				if (abs(axisX.X) < abs(axisX.Y))
				{

					if (currentZoop.Z == 0) //Handle starting from 0
					{
						if (axisX.Z >= 0 && delta > 0)
						{
							currentZoop.Z = 1;
						}
						else if (axisX.Z < 0 && delta > 0)
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
				if (abs(axisX.Y) < abs(axisX.X))
				{

					if (currentZoop.Y == 0) //Handle starting from 0
					{
						if (axisX.Y >= 0 && delta > 0)
						{
							currentZoop.Y = 1;
						}
						else if (axisX.Y < 0 && delta > 0)
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
			AFGFoundationHologram* foundation = Cast<AFGFoundationHologram>(self);
			if (foundation && foundation->GetCurrentBuildMode() == foundation->mBuildModeVerticalZoop)
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

		HologramsToZoop.Add(self, currentZoop);
		Fhg->SetZoopAmount(currentZoop);
	}
}

bool FInfiniteZoopModule::SetZoopAmount(AFGFactoryBuildingHologram* self, const FIntVector& Zoop)
{
	auto baseHG = Cast<AFGHologram>(self);
	if (baseHG)
	{
		auto newZoop = HologramsToZoop.Find(baseHG);
		if (newZoop == nullptr || newZoop->IsZero())
		{
			return false;
		}
		if (newZoop && Zoop != *newZoop)
		{
			return true;
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
			int index = GetStructIndex(fhg);
			FZoopStruct* zStruct = index >= 0 ? FoundationsBeingZooped[index] : new FZoopStruct();

			if (zStruct->FoundationHologram == nullptr || !zStruct->secondPassComplete)
			{
				return false;
			}
			else
			{
				if (index >= 0)
				{
					if (zStruct->secondPassComplete)
					{
						self->mDesiredZoop.X = zStruct->X;
						self->mDesiredZoop.Y = zStruct->Y;
						self->mDesiredZoop.Z = zStruct->Z;
						//UE_LOG(InfiniteZoop_Log, Display, TEXT("OnRep_DesiredZoop: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
						//self->OnRep_DesiredZoop();

						zStruct->firstPassComplete = false;
						zStruct->secondPassComplete = false;
						FoundationsBeingZooped[index] = zStruct;
					}
				}
				self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_None;
				self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_None;
			}
		}
	}
	return true;
}

void FInfiniteZoopModule::CreateDefaultFoundationZoop(AFGFoundationHologram* self, const FHitResult& hitResult)
{
	if (IsZoopMode(self))
	{
		int index = GetStructIndex(self);
		FZoopStruct* zStruct = index >= 0 ? FoundationsBeingZooped[index] : new FZoopStruct();

		if (zStruct->FoundationHologram == nullptr)
		{
			zStruct->FoundationHologram = self;
		}

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
		}
		else
		{
			zStruct->secondPassComplete = true;
		}

		if (index >= 0)
		{
			FoundationsBeingZooped[index] = zStruct;
		}
		else
		{
			FoundationsBeingZooped.Add(zStruct);
		}

		self->OnRep_DesiredZoop();
	}
}

bool FInfiniteZoopModule::UpdateZoop(AFGFoundationHologram* self)
{
	auto fhg = Cast< AFGFoundationHologram>(self);
	if (fhg)
	{
		if (IsZoopMode(self))
		{
			int index = GetStructIndex(self);
			FZoopStruct* zStruct = index >= 0 ? FoundationsBeingZooped[index] : new FZoopStruct();

			if (zStruct->FoundationHologram == nullptr || !zStruct->secondPassComplete)
			{
				return false;
			}
			else
			{
				if (index >= 0)
				{
					if (zStruct->secondPassComplete)
					{
						self->mDesiredZoop.X = zStruct->X;
						self->mDesiredZoop.Y = zStruct->Y;
						self->mDesiredZoop.Z = zStruct->Z;
						//UE_LOG(InfiniteZoop_Log, Display, TEXT("UpdateZoop: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
						self->OnRep_DesiredZoop();

						zStruct->firstPassComplete = false;
						zStruct->secondPassComplete = false;
						//zStruct->X = 0;
						//zStruct->Y = 0;
						//zStruct->Z = 0;
						FoundationsBeingZooped[index] = zStruct;
					}
				}
				self->mBlockedZoopDirectionMask = (uint8)EHologramZoopDirections::HZD_None;
				self->mDefaultBlockedZoopDirections = (uint8)EHologramZoopDirections::HZD_None;
			}
		}
	}
	return true;
}

int32 FInfiniteZoopModule::GetBaseCostMultiplier(const AFGFactoryBuildingHologram* self)
{

	auto nonConst = const_cast<AFGFactoryBuildingHologram*>(self);
	auto fhg = Cast< AFGFoundationHologram>(nonConst);
	if (fhg)
	{
		int index = GetStructIndex(fhg);
		FZoopStruct* zStruct = index >= 0 ? FoundationsBeingZooped[index] : new FZoopStruct();
		if (zStruct->FoundationHologram)
		{
			auto x = abs(zStruct->X);
			auto y = abs(zStruct->Y);
			auto z = abs(zStruct->Z);
			x = x > 0 ? x + 1 : 1;
			y = y > 0 ? y + 1 : 1;
			z = z > 0 ? z + 1 : 1;
			auto result = (x * y * z);
			return result;
		}
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
	if (IsZoopMode(self))
	{
		int index = GetStructIndex(self);
		FZoopStruct* zStruct = index >= 0 ? FoundationsBeingZooped[index] : new FZoopStruct();
		if (zStruct->FoundationHologram)
		{
			self->mDesiredZoop.X = zStruct->X;
			self->mDesiredZoop.Y = zStruct->Y;
			self->mDesiredZoop.Z = zStruct->Z;
			//UE_LOG(InfiniteZoop_Log, Display, TEXT("ConstructZoop: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);
			self->OnRep_DesiredZoop();
			//UE_LOG(InfiniteZoop_Log, Display, TEXT("ConstructZoop 2: X:%d, Y:%d"), self->mDesiredZoop.X, self->mDesiredZoop.Y);

			zStruct->firstPassComplete = false;
			zStruct->secondPassComplete = false;
			//zStruct->X = 0;
			//zStruct->Y = 0;
			//zStruct->Z = 0;
			FoundationsBeingZooped[index] = zStruct;
		}
	}
}

int32 FInfiniteZoopModule::GetStructIndex(AFGFoundationHologram* self)
{
	int index = -1;
	for (int i = 0; i < FoundationsBeingZooped.Num(); i++)
	{
		FZoopStruct* zs = FoundationsBeingZooped[i];
		if (zs->FoundationHologram && zs->FoundationHologram == self)
		{
			index = i;
			break;
		}
	}
	return index;
}

bool FInfiniteZoopModule::IsZoopMode(AFGFoundationHologram* self)
{
	auto vertMode = self->mBuildModeVerticalZoop;
	TSubclassOf<UFGHologramBuildModeDescriptor> currentBuildMode;
	auto owner = self->GetOwner();
	if (owner)
	{
		auto bgun = Cast<AFGBuildGun>(owner);
		if (bgun)
		{
			auto buildState = bgun->GetBuildGunStateFor(EBuildGunState::BGS_BUILD);
			if (buildState)
			{
				auto bgBuild = Cast<UFGBuildGunStateBuild>(buildState);
				if (bgBuild)
				{
					currentBuildMode = bgBuild->mCurrentHologramBuildMode;
				}
			}
		}
	}
	if (currentBuildMode == vertMode)
	{
		return false;
	}
	return true;
}

void FInfiniteZoopModule::StartupModule() {


	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::OnRep_DesiredZoop, [=](auto& scope, AFGFactoryBuildingHologram* self)
		{
			if (!this->OnRep_DesiredZoop(self))
			{
				scope.Cancel();
			}
		});

	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::SetZoopAmount, [=](auto& scope, AFGFactoryBuildingHologram* self, const FIntVector& Zoop)
		{
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
				auto mult = fbhg->GetBaseCostMultiplier();
				mult = mult > 0 ? mult - 1 : 0;
				scope(self, (float)mult, maxZoop, zoopLocation);
			}
		});

	AFGFactoryBuildingHologram* fbhgCDO = GetMutableDefault<AFGFactoryBuildingHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGFactoryBuildingHologram::GetBaseCostMultiplier, fbhgCDO, [=](auto& scope, const AFGFactoryBuildingHologram* self)
		{
			auto result = this->GetBaseCostMultiplier(self);
			scope.Override(result);
		});

	AFGFoundationHologram* fhCDO = GetMutableDefault<AFGFoundationHologram>();

	//SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::UpdateZoop, fhCDO, [=](auto& scope, AFGFoundationHologram* self)
	//	{
	//		if (!this->UpdateZoop(self))
	//		{
	//			scope.Cancel();
	//		}
	//	});

	SUBSCRIBE_METHOD_VIRTUAL(AFGFoundationHologram::ConstructZoop, fhCDO, [=](auto& scope, AFGFoundationHologram* self, TArray<AActor*>& out_children)
		{
			this->ConstructZoop(self, out_children);
	});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGFoundationHologram::CreateDefaultFoundationZoop, fhCDO, [=](AFGFoundationHologram* self, const FHitResult& hitResult)
		{
			this->CreateDefaultFoundationZoop(self, hitResult);
		});

	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, hCDO, [=](auto scope, AFGHologram* self, int32 delta)
		{
			this->ScrollHologram(self, delta);
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Destroyed, hCDO, [=](auto& scope, AFGHologram* self)
		{
			if (self->CanBeZooped())
			{
				HologramsToZoop.Remove(self);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::OnBuildModeChanged, hCDO, [=](auto& scope, AFGHologram* self)
		{
			if (self->CanBeZooped())
			{
				HologramsToZoop.Remove(self);
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [](auto scope, AFGHologram* self)
		{
			if (self)
			{
				UWorld* world = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
				AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
				AInfiniteZoop_ClientSubsystem* clientSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoop_ClientSubsystem>();

				AFGWallHologram* Whg = Cast<AFGWallHologram>(self);
				if (Whg)
				{
					Whg->mOnlyAllowLineZoop = false;
				}

				AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);

				if (clientSubsystem)
				{
					//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: False"));
					if (bhg && bhg->mMaxZoopAmount > 0)
					{
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
				//else
				//{
				//	//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: True"));
				//	if (bhg)
				//	{
				//		if (clientSubsystem->tempZoopAmount > 0)
				//		{
				//			bhg->mMaxZoopAmount = clientSubsystem->tempZoopAmount - 1;
				//		}
				//		else
				//		{
				//			bhg->mMaxZoopAmount = zoopSubsystem->currentZoopAmount - 1;
				//		}
				//		//UE_LOG(InfiniteZoop_Log, Display, TEXT("Increased Zoop Amount"));
				//		return;
				//	}
				//}

				AFGLadderHologram* ladderHG = Cast<AFGLadderHologram>(self);
				if (ladderHG)
				{
					if (clientSubsystem->tempZoopAmount > 0)
					{
						ladderHG->mMaxSegmentCount = clientSubsystem->tempZoopAmount - 1;
					}
					else
					{
						ladderHG->mMaxSegmentCount = zoopSubsystem->currentZoopAmount - 1;
					}
				}
			}
		});

#if !WITH_EDITOR
	AFGPillarHologram* pCDO = GetMutableDefault<AFGPillarHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGPillarHologram::BeginPlay, pCDO, [](auto scope, AFGPillarHologram* self)
		{
			if (self && self->HasAuthority())
			{
				UWorld* world = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
				AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
				AInfiniteZoop_ClientSubsystem* clientSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoop_ClientSubsystem>();

				if (clientSubsystem->tempZoopAmount > 0)
				{
					self->mMaxZoop = clientSubsystem->tempZoopAmount - 1;
				}
				else
				{
					self->mMaxZoop = zoopSubsystem->currentZoopAmount - 1;
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
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);