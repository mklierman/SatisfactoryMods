#include "CloudToButtModule.h"
#include "Patching/NativeHookManager.h"
#include "Blueprint/UserWidget.h"
#include "CTB_Subsystem.h"
#include "CTB_BPFL.h"
#include "Subsystem/ModSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"

DEFINE_LOG_CATEGORY(CTB_Log);

void FCloudToButtModule::StartupModule() {
#if !WITHEDITOR
	SUBSCRIBE_METHOD_AFTER(UUserWidget::Construct, [](UUserWidget* self)
		{
			//UE_LOG(CTB_Log, Display, TEXT("UUserWidget::Construct"));
			UWorld* WorldObject = self->GetWorld();
			if (WorldObject)
			{
				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				if (SubsystemActorManager)
				{
					ACTB_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ACTB_Subsystem>();
					if (subsystem)
					{
						subsystem->OnWidgetConstruct.Broadcast(self);
					}
				}
			}
		});
#endif

}


IMPLEMENT_GAME_MODULE(FCloudToButtModule, CloudToButt);