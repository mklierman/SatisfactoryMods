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

				/*if (abs(currentZoop.Y) > abs(currentZoop.X))
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
				else if (abs(currentZoop.Y) < abs(currentZoop.X))
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
				else if (currentZoop.Z != 0)
				{
					if (currentZoop.Z > 0)
					{
						currentZoop.Z = currentZoop.Z + (1 * delta);
					}
					else
					{
						currentZoop.Z = (abs(currentZoop.Z) + (1 * delta)) * -1;
					}
				}*/
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

				/*if (abs(currentZoop.Y) > abs(currentZoop.X))
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
				else if (abs(currentZoop.Y) < abs(currentZoop.X))
				{
					if (currentZoop.Y > 0)
					{
						currentZoop.Y = currentZoop.Y + (1 * delta);
					}
					else
					{
						currentZoop.Y = (abs(currentZoop.Y) + (1 * delta)) * -1;
					}
				}*/
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

void FInfiniteZoopModule::StartupModule() {

#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::SetZoopAmount, [=](auto& scope, AFGFactoryBuildingHologram* self, const FIntVector& Zoop)
		{
			auto baseHG = Cast<AFGHologram>(self);
			if (baseHG)
			{
				auto newZoop = HologramsToZoop.Find(baseHG);
				if (newZoop == nullptr || newZoop->IsZero())
				{
					return;
				}
				if (newZoop && Zoop != *newZoop)
				{
					scope.Cancel();
				}
			}
		});

	AFGFoundationHologram* fhCDO = GetMutableDefault<AFGFoundationHologram>();
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

				AFGFoundationHologram* Fhg = Cast<AFGFoundationHologram>(self);
				AFGWallHologram* Whg = Cast<AFGWallHologram>(self);
				AFGWalkwayHologram* wwhg = Cast< AFGWalkwayHologram>(self);
				if (Fhg)
				{
					Fhg->mOnlyAllowLineZoop = false;
				}
				else if (Whg)
				{
					Whg->mOnlyAllowLineZoop = false;
				}
				else if (wwhg)
				{
					wwhg->mOnlyAllowLineZoop = false;
				}

				AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);

				if (!clientSubsystem->zoopCorners)
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
				else
				{
					//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: True"));
					if (bhg)
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