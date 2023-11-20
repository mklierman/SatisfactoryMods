#include "DT_BPFL.h"

void UDT_BPFL::SetMeshInstanced(UFGColoredInstanceMeshProxy* mesh, bool setToInstanced)
{
	if (mesh)
	{
		mesh->SetInstanced(setToInstanced);
	}
};