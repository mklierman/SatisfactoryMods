#include "ShowSplinePathModule.h"

//#pragma optimize("", off)
void FShowSplinePathModule::StartupModule() {
#if !WITH_EDITOR
	//void RemoveTargetPoint(class AFGTargetPoint* targetToRemove, bool updateList = true);
	SUBSCRIBE_METHOD_AFTER(AFGVehicleSubsystem::UpdateTargetList, [this](AFGVehicleSubsystem* self, AFGDrivingTargetList* targetList)
		{
			if (self->HasAuthority())
			{
				UWorld* WorldObject = targetList->GetWorld();

				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
				this->ShowPathSpline(targetList, subsystem);
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGDrivingTargetList::CreatePath, [this](AFGDrivingTargetList* self)
		{
			if (self->HasAuthority())
			{
				UWorld* WorldObject = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
				this->ShowPathSpline(self, subsystem);
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGDrivingTargetList::OnRep_IsPathVisible, [this](AFGDrivingTargetList* self)
		{
			if (self->HasAuthority())
			{
				UWorld* WorldObject = self->GetWorld();
				USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
				ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
				subsystem->HandlePathSplines(self, self->mIsPathVisible);
			}
		});

	SUBSCRIBE_METHOD_AFTER(AFGVehicleSubsystem::AddTargetPoint, [this](AFGVehicleSubsystem* self, class AFGTargetPoint* target)
		{
			if (self->HasAuthority())
			{
				if (!target)
				{
					return;
				}
				AFGDrivingTargetList* targetList = target->GetOwningList();
				if (targetList)
				{
					UWorld* WorldObject = targetList->GetWorld();
					USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
					ASSP_Subsystem* subsystem = SubsystemActorManager->GetSubsystemActor<ASSP_Subsystem>();
					//this->ShowPathSpline(targetList, subsystem);
					subsystem->HandlePathSplines(targetList, false);
					subsystem->HandlePathSplines(targetList, true);
				}
			}
		});

	SUBSCRIBE_METHOD(AFGVehicleSubsystem::RemoveTargetPoint, [this](auto& scope, AFGVehicleSubsystem* self, class AFGTargetPoint* targetToRemove, bool updateList)
		{
			if (self->HasAuthority())
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
			}
		});
#endif
}

void FShowSplinePathModule::ShowPathSpline(AFGDrivingTargetList* targetList, ASSP_Subsystem* subsystem)
{
	if (subsystem->TargetListMeshPools.Contains(targetList))
	{
		subsystem->HandlePathSplines(targetList, false);
		subsystem->HandlePathSplines(targetList, true);
	}
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FShowSplinePathModule, ShowSplinePath);