
#include "PPBuildablePriorityOutput.h"

APPBuildablePriorityOutput::APPBuildablePriorityOutput()
{
	In = CreatePipePort(TEXT("In"), FVector(0.f, 300.f, 180.f), FRotator(0.f, 90.f, 0.f), false);
	PriorityOut = CreatePipePort(TEXT("PriorityOut"), FVector(0.f, -300.f, 180.f), FRotator(0.f, -90.f, 0.f), true);
	OverflowOut = CreatePipePort(TEXT("OverflowOut"), FVector(0.f, -380.f, 375.f), FRotator(0.f, -90.f, 0.f), true);
}

// Fill priority first
void APPBuildablePriorityOutput::RouteFluids(float dt)
{
	if (In && In->GetFluidBox())
	{
		In->GetFluidBox()->AddedPressure = 0.f;
	}

	PushPipe(PriorityOut, dt);

	int32 want = GetPullRoom(dt);
	if (In && In->IsConnected())
	{
		want = FMath::Min(want, GetFlowLiters(In, dt));
	}

	PullPipe(In, want, dt);

	PushPipe(PriorityOut, dt);
	PushPipe(OverflowOut, dt);
}
