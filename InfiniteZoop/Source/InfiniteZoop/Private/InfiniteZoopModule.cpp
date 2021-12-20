#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
#include "Hologram/FGLadderHologram.h"
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

				AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);
				//UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: "));
				if (!zoopSubsystem->zoopCorners)
				{
					UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: False"));
					if (bhg && bhg->mMaxZoopAmount > 0)
					{
						bhg->mMaxZoopAmount = zoopSubsystem->currentZoopAmount - 1;
						UE_LOG(InfiniteZoop_Log, Display, TEXT("Increased Zoop Amount"));
						return;
					}
				}
				else
				{
					UE_LOG(InfiniteZoop_Log, Display, TEXT("Zoop Corners: True"));
					if (bhg)
					{
						bhg->mMaxZoopAmount = zoopSubsystem->currentZoopAmount - 1;
						UE_LOG(InfiniteZoop_Log, Display, TEXT("Increased Zoop Amount"));
						return;
					}
				}

				AFGLadderHologram* ladderHG = Cast<AFGLadderHologram>(self);
				if (ladderHG)
				{
					ladderHG->mMaxSegmentCount = zoopSubsystem->currentZoopAmount - 1;
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

				self->mMaxZoop = zoopSubsystem->currentZoopAmount - 1;
			}
		});

#endif
}


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);