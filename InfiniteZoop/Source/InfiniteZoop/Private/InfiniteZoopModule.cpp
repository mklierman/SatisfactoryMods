#include "InfiniteZoopModule.h"
#include "Hologram/FGFactoryBuildingHologram.h"
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
			//if (this->zoopSubsystem)
			//{
				auto world = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();
				auto zoopSubsystem = SubsystemActorManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
			//}
			AFGFactoryBuildingHologram* bhg = Cast<AFGFactoryBuildingHologram>(self);
			if (bhg)
			{
				bhg->mMaxZoopAmount = zoopSubsystem->currentZoopAmount - 1;
			}
		});
#endif
}


IMPLEMENT_GAME_MODULE(FInfiniteZoopModule, InfiniteZoop);