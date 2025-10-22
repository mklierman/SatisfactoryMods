#include "ShowSplinePathModule.h"
#include "SSP_RemoteCallObject.h"
#include "FGPlayerController.h"

//#pragma optimize("", off)
void FShowSplinePathModule::StartupModule()
{
#if !WITH_EDITOR
	SUBSCRIBE_METHOD_AFTER(AFGVehicleSubsystem::UpdateTargetList, [this](AFGVehicleSubsystem* self, AFGDrivingTargetList* targetList)
		{
				UWorld* WorldObject = targetList->GetWorld();

				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
				this->ShowPathSpline(targetList, subsystem);
		});

	SUBSCRIBE_METHOD_AFTER(AFGDrivingTargetList::CreatePath, [this](AFGDrivingTargetList* self)
		{
				UWorld* WorldObject = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
				subsystem->HandlePathSplines(self, self->mIsPathVisible);
		});

	SUBSCRIBE_METHOD_AFTER(AFGDrivingTargetList::OnRep_IsPathVisible, [this](AFGDrivingTargetList* self)
		{
			UWorld* WorldObject = self->GetWorld();
			USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
			ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
			subsystem->HandlePathSplines(self, self->mIsPathVisible);
		});

	SUBSCRIBE_METHOD_AFTER(AFGVehicleSubsystem::AddTargetPoint, [this](AFGVehicleSubsystem* self, class AFGTargetPoint* target)
		{
			if (!target)
			{
				return;
			}
			AFGDrivingTargetList* targetList = target->GetOwningList();
			if (targetList)
			{
				UWorld* WorldObject = targetList->GetWorld();
				AFGVehicleSubsystem* vehicleSubsystem = AFGVehicleSubsystem::Get(WorldObject);
				vehicleSubsystem->UpdateTargetList(targetList);
			}
		});

	SUBSCRIBE_METHOD(AFGVehicleSubsystem::RemoveTargetPoint, [this](auto& scope, AFGVehicleSubsystem* self, class AFGTargetPoint* targetToRemove, bool updateList)
		{
				if (!targetToRemove)
				{
					scope.Cancel();
					return;
				}
				AFGDrivingTargetList* targetList = targetToRemove->GetOwningList();
				if (targetList)
				{
					UWorld* WorldObject = targetList->GetWorld();
					USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
					ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
					scope(self, targetToRemove, updateList);
					this->ShowPathSpline(targetList, subsystem);
				}
		});
#endif
}

void FShowSplinePathModule::ShowPathSpline(AFGDrivingTargetList* targetList, ASSP_Subsystem* subsystem)
{
	if (subsystem)
	{
		subsystem->HandlePathSplines(targetList, false);
		subsystem->HandlePathSplines(targetList, true);
	}
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FShowSplinePathModule, ShowSplinePath);
