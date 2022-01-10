#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGLadderHologram.h"
#include "Hologram/FGFoundationHologram.h"
#include "Hologram/FGWallHologram.h"
#include "Hologram/FGWalkwayHologram.h"
#include "FGPillarHologram.h"
#include "Patching/NativeHookManager.h"
#include "FGResearchManager.h"
#include "FGGameState.h"
#include "FGResearchTree.h"
#include "FGResearchTreeNode.h"
#include "Subsystem/SubsystemActorManager.h"
#include "InfiniteZoopSubsystem.h"
#include "Equipment/FGBuildGunBuild.h"
#include "FGHologramBuildModeDescriptor.h"


DEFINE_LOG_CATEGORY(InfiniteZoop_Log);
void FInfiniteZoopModule::StartupModule() {

#if !WITH_EDITOR
	AFGHologram* hCDO = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::BeginPlay, hCDO, [](auto scope, AFGHologram* self)
		{
			if (self && self->HasAuthority())
			{
				UWorld* world = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
				AInfiniteZoopSubsystem* zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();

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
				//if (bhg)
				//{
				//	bhg->mDefaultBlockedZoopDirections = 0;
				//}
				//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: "));
				if (!zoopSubsystem->zoopCorners)
				{
					//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: False"));
					if (bhg && bhg->mMaxZoopAmount > 0)
					{
						if (zoopSubsystem->tempZoopAmount > 0)
						{
							bhg->mMaxZoopAmount = zoopSubsystem->tempZoopAmount - 1;
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
						if (zoopSubsystem->tempZoopAmount > 0)
						{
							bhg->mMaxZoopAmount = zoopSubsystem->tempZoopAmount - 1;
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
					if (zoopSubsystem->tempZoopAmount > 0)
					{
						ladderHG->mMaxSegmentCount = zoopSubsystem->tempZoopAmount - 1;
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

				if (zoopSubsystem->tempZoopAmount > 0)
				{
					self->mMaxZoop = zoopSubsystem->tempZoopAmount - 1;
				}
				else
				{
					self->mMaxZoop = zoopSubsystem->currentZoopAmount - 1;
				}
			}
		});

	//void BlockZoopDirectionsBasedOnSnapDirection(const FVector & worldSpaceSnapDirection);
	SUBSCRIBE_METHOD(AFGFactoryBuildingHologram::BlockZoopDirectionsBasedOnSnapDirection, [](auto scope, AFGFactoryBuildingHologram* self, const FVector& worldSpaceSnapDirection)
		{
			scope.Cancel();
		});


	//virtual void OnBuildModeChanged() override;
	AFGFactoryBuildingHologram* fbhg = GetMutableDefault<AFGFactoryBuildingHologram>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGFactoryBuildingHologram::OnBuildModeChanged, fbhg, [](auto scope, AFGFactoryBuildingHologram* self)
		{

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
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGFactoryBuildingHologram::OnBuildModeChanged, fbhg, []( AFGFactoryBuildingHologram* self)
		{

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
		});
#endif
}


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);