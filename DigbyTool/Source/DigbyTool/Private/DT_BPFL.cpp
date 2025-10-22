#include "DT_BPFL.h"

static TAutoConsoleVariable<bool> CVarDigbyToolDebugMode(
	TEXT("DigbyTool.DebugMode"),
	false,
	TEXT("Verbose logging for Digby Tool"));

void UDT_BPFL::SetMeshInstanced(UFGColoredInstanceMeshProxy* mesh, bool setToInstanced)
{
	if (mesh)
	{
		mesh->SetInstanced(setToInstanced);
	}
}

void UDT_BPFL::SetMeshMaterial(UFGColoredInstanceMeshProxy* mesh, int32 index, UMaterialInterface* material)
{
	mesh->SetMaterial(index, material);
}

void UDT_BPFL::OpenURL(FString URL)
{
	FString cmd = "explorer";
	FString newUrl = "\"" + URL + "\"";
	const TCHAR* parms = *newUrl;
	FString sl = "C:\\";
	const TCHAR* startLoc = *sl;
	uint32* procID = nullptr;
	FPlatformProcess::CreateProc(*cmd, parms, true, false, false, nullptr, 1, nullptr, (void*)nullptr);
}

void UDT_BPFL::UpdateTargetList(AFGVehicleSubsystem* vehicleSubsystem, AFGDrivingTargetList* targetList)
{
	vehicleSubsystem->UpdateTargetList(targetList);
}

void UDT_BPFL::UpdateTargetLists(AFGVehicleSubsystem* vehicleSubsystem)
{
	vehicleSubsystem->UpdateTargetLists();
}

void UDT_BPFL::UpdateTargetPoints(AFGVehicleSubsystem* vehicleSubsystem)
{
	vehicleSubsystem->UpdateTargetPoints();
}
