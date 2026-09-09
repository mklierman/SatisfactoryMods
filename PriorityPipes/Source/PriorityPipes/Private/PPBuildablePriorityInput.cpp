
#include "PPBuildablePriorityInput.h"

APPBuildablePriorityInput::APPBuildablePriorityInput()
{
	PriorityIn = CreatePipePort(TEXT("PriorityIn"), FVector(0.f, 300.f, 180.f), FRotator(0.f, 90.f, 0.f), false);
	Out = CreatePipePort(TEXT("Out"), FVector(0.f, -300.f, 180.f), FRotator(0.f, -90.f, 0.f), true);
	NormalIn = CreatePipePort(TEXT("NormalIn"), FVector(0.f, 350.f, 375.f), FRotator(0.f, 90.f, 0.f), false);
}

// Drain priority first
void APPBuildablePriorityInput::RouteFluids(float dt)
{
	if (PriorityIn && PriorityIn->GetFluidBox())
	{
		PriorityIn->GetFluidBox()->AddedPressure = 0.f;
	}
	if (NormalIn && NormalIn->GetFluidBox())
	{
		NormalIn->GetFluidBox()->AddedPressure = 0.f;
	}

	PushPipe(Out, dt);

	int32 want = GetPullRoom(dt);
	if (Out && Out->IsConnected())
	{
		want = FMath::Min(want, GetFlowLiters(Out, dt));
	}
	else if (PriorityIn && PriorityIn->IsConnected())
	{
		want = FMath::Min(want, GetFlowLiters(PriorityIn, dt));
	}
	else if (NormalIn && NormalIn->IsConnected())
	{
		want = FMath::Min(want, GetFlowLiters(NormalIn, dt));
	}

	int32 got = 0;
	if (PriorityIn && PriorityIn->IsConnected())
	{
		got = PullPipe(PriorityIn, FMath::Min(want, GetFlowLiters(PriorityIn, dt)), dt);
	}

	if (got < want)
	{
		PullPipe(NormalIn, want - got, dt);
	}

	PushPipe(Out, dt);
}
