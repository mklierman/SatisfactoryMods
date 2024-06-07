#include "DT_BPFL.h"

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
;