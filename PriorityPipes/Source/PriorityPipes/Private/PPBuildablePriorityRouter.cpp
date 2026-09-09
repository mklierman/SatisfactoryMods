
#include "PPBuildablePriorityRouter.h"
#include "FGPipeNetwork.h"
#include "FGPipeSubsystem.h"
#include "Hologram/FGFactoryHologram.h"
#include "Net/UnrealNetwork.h"

static TSubclassOf<UFGItemDescriptor> GetPortFluid(UFGPipeConnectionFactory* port)
{
	if (!IsValid(port))
	{
		return nullptr;
	}

	if (auto fluid = port->GetFluidDescriptor())
	{
		return fluid;
	}

	if (auto* other = port->GetPipeConnection())
	{
		if (auto fluid = other->GetFluidDescriptor())
		{
			return fluid;
		}
	}

	if (auto* pipes = AFGPipeSubsystem::Get(port->GetWorld()))
	{
		if (auto* net = pipes->FindPipeNetwork(port->GetPipeNetworkID()))
		{
			return net->GetFluidDescriptor();
		}
	}

	return nullptr;
}

static FFluidBox* GetConnectedBox(UFGPipeConnectionFactory* port)
{
	if (!IsValid(port))
	{
		return nullptr;
	}

	auto* other = port->GetPipeConnection();
	if (!other || !other->HasFluidIntegrant())
	{
		return nullptr;
	}

	return other->GetFluidIntegrant()->GetFluidBox();
}

APPBuildablePriorityRouter::APPBuildablePriorityRouter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	mFactoryTickFunction.bCanEverTick = true;
	mHasInventoryPotential = false;
	mAddToSignificanceManager = false;
	mHasPipeInput = false;
	mHasPipeOutput = false;
	mHologramClass = AFGFactoryHologram::StaticClass();

	Mesh = CreateDefaultSubobject<UFGColoredInstanceMeshProxy>(TEXT("Mesh"));
	if (RootComponent)
	{
		Mesh->SetupAttachment(RootComponent);
	}
	else
	{
		SetRootComponent(Mesh);
	}

	FFGClearanceData clearance;
	clearance.ClearanceBox = FBox(FVector(-400.f, -400.f, -600.f), FVector(400.f, 400.f, 600.f));
	clearance.RelativeTransform = FTransform(FQuat::Identity, FVector(0.f, 0.f, 600.f), FVector::OneVector);
	mClearanceData.Add(clearance);
}

TArray<UFGPipeConnectionFactory*> APPBuildablePriorityRouter::GetPorts()
{
	return mPorts;
}

void APPBuildablePriorityRouter::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
	StripStrayPipePorts();

	auto* pipes = AFGPipeSubsystem::Get(GetWorld());
	mOutputs.Reset();
	mPortWasConnected.Reset();
	for (auto* port : mPorts)
	{
		if (!IsValid(port))
		{
			continue;
		}

		port->SetInventoryAccessIndex(0);
		port->SetFluidIntegrant(port);
		if (pipes)
		{
			pipes->RegisterFluidIntegrant(port);
		}

		if (port->GetPipeConnectionType() == EPipeConnectionType::PCT_PRODUCER)
		{
			mOutputs.Add(port);
		}

		mPortWasConnected.Add(port->IsConnected() ? 1 : 0);
	}
}

void APPBuildablePriorityRouter::UpdatePortNetworks()
{
	auto* pipes = AFGPipeSubsystem::Get(GetWorld());
	if (mPortWasConnected.Num() != mPorts.Num())
	{
		mPortWasConnected.Init(0, mPorts.Num());
	}

	for (int32 i = 0; i < mPorts.Num(); ++i)
	{
		auto* port = mPorts[i];
		if (!IsValid(port))
		{
			mPortWasConnected[i] = 0;
			continue;
		}

		port->SetFluidIntegrant(port);
		auto connected = port->IsConnected() ? 1 : 0;
		if (!connected)
		{
			if (auto* box = port->GetFluidBox())
			{
				box->Content = 0.f;
				box->FlowThrough = 0.f;
				box->FlowFill = 0.f;
				box->FlowDrain = 0.f;
			}
		}

		if (pipes && connected != mPortWasConnected[i])
		{
			pipes->RegisterFluidIntegrant(port);
		}

		mPortWasConnected[i] = connected;
	}
}

void APPBuildablePriorityRouter::Tick(float dt)
{
	Super::Tick(dt);
	if (HasAuthority())
	{
		UpdatePortNetworks();
	}
}

void APPBuildablePriorityRouter::Factory_Tick(float dt)
{
	if (HasAuthority() && dt > 0.f && HasPower())
	{
		ClearLeftoverIfNeeded();
		mMovedThisTick = 0;
		RouteFluids(dt);
		mIsTransferring = mMovedThisTick || mBuffer.HasItems();

		for (auto* port : mOutputs)
		{
			if (port && port->GetFluidBox())
			{
				port->GetFluidBox()->AddedPressure = 10.f;
			}
		}
	}
	else if (HasAuthority())
	{
		mMovedThisTick = 0;
		mIsTransferring = 0;
		for (auto* port : mOutputs)
		{
			if (port && port->GetFluidBox())
			{
				port->GetFluidBox()->AddedPressure = 0.f;
			}
		}
	}

	Super::Factory_Tick(dt);
}

bool APPBuildablePriorityRouter::CanProduce_Implementation() const
{
	return HasPower() && mIsTransferring != 0;
}

EProductionStatus APPBuildablePriorityRouter::GetProductionIndicatorStatus() const
{
	if (!HasPower())
	{
		return EProductionStatus::IS_ERROR;
	}

	if (mIsTransferring || IsProducing())
	{
		return EProductionStatus::IS_PRODUCING;
	}

	return EProductionStatus::IS_STANDBY;
}

void APPBuildablePriorityRouter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APPBuildablePriorityRouter, mIsTransferring);
}

void APPBuildablePriorityRouter::RouteFluids(float)
{
}

UFGPipeConnectionFactory* APPBuildablePriorityRouter::CreatePipePort(FName name, FVector loc, FRotator rot, bool bOutput)
{
	auto* port = CreateDefaultSubobject<UFGPipeConnectionFactory>(name);
	if (RootComponent)
	{
		port->SetupAttachment(RootComponent);
	}
	else
	{
		port->SetupAttachment(Mesh);
	}

	FTransform meshRel(FRotator(0.f, 540.000122f, 0.f));
	FTransform portRel(rot, loc);
	auto onRoot = portRel * meshRel;
	port->SetRelativeLocation(onRoot.GetLocation());
	port->SetRelativeRotation(onRoot.Rotator());
	if (bOutput)
	{
		port->SetPipeConnectionType(EPipeConnectionType::PCT_PRODUCER);
	}
	else
	{
		port->SetPipeConnectionType(EPipeConnectionType::PCT_CONSUMER);
	}
	port->mFluidBoxVolume = FFluidBox::MINIMUM_VOLUME;
	port->mApplyAdditionalPressure = bOutput;
	port->SetInventoryAccessIndex(0);

	mPorts.Add(port);
	if (bOutput)
	{
		mOutputs.Add(port);
	}

	return port;
}

void APPBuildablePriorityRouter::StripStrayPipePorts()
{
	TInlineComponentArray<UFGPipeConnectionFactory*> allPorts;
	GetComponents(allPorts);

	auto* pipes = AFGPipeSubsystem::Get(GetWorld());
	for (auto* port : allPorts)
	{
		if (!IsValid(port) || mPorts.Contains(port))
		{
			continue;
		}

		port->ClearConnection();
		port->SetPipeConnectionType(EPipeConnectionType::PCT_SNAP_ONLY);
		port->mApplyAdditionalPressure = false;
		if (pipes)
		{
			pipes->UnregisterFluidIntegrant(port);
		}

		port->DestroyComponent();
	}
}

int32 APPBuildablePriorityRouter::GetFlowLiters(UFGPipeConnectionFactory* port, float dt)
{
	float flowLimit = FFluidBox::DEFAULT_FLOW_LIMIT;

	if (auto* pipe = GetConnectedBox(port))
	{
		flowLimit = pipe->FlowLimit;
	}
	else if (port && port->GetFluidBox())
	{
		flowLimit = port->GetFluidBox()->FlowLimit;
	}

	if (flowLimit <= 0.f)
	{
		return TNumericLimits<int32>::Max();
	}

	return FMath::Max(1, FMath::RoundToInt(flowLimit * dt * 1000.f));
}

// Unused dest capacity
int32 APPBuildablePriorityRouter::GetPullRoom(float dt)
{
	int32 destSum = 0;
	bool anyDest = false;

	for (auto* port : mOutputs)
	{
		if (!port || !port->IsConnected())
		{
			continue;
		}

		anyDest = true;
		destSum += GetFlowLiters(port, dt);
	}

	if (!anyDest)
	{
		return 0;
	}

	int32 have = 0;
	if (mBuffer.HasItems())
	{
		have = mBuffer.NumItems;
	}
	return FMath::Max(0, destSum + 4 - have);
}

bool APPBuildablePriorityRouter::CanOutputFluid(TSubclassOf<UFGItemDescriptor> fluid)
{
	if (!fluid)
	{
		return false;
	}

	for (auto* port : mOutputs)
	{
		if (!port || !port->IsConnected())
		{
			continue;
		}

		auto destFluid = GetPortFluid(port);
		if (!destFluid || destFluid == fluid)
		{
			return true;
		}
	}

	return false;
}

void APPBuildablePriorityRouter::ClearLeftoverIfNeeded()
{
	bool anyConnected = false;
	for (auto* port : mPorts)
	{
		if (IsValid(port) && port->IsConnected())
		{
			anyConnected = true;
			break;
		}
	}

	if (!anyConnected)
	{
		for (auto* port : mPorts)
		{
			if (port && port->GetFluidBox())
			{
				port->GetFluidBox()->Content = 0.f;
			}
		}
		mBuffer = FInventoryStack();
		return;
	}

	if (!mBuffer.HasItems())
	{
		return;
	}

	if (CanOutputFluid(mBuffer.Item.GetItemClass()))
	{
		return;
	}

	bool anyDest = false;
	for (auto* port : mOutputs)
	{
		if (port && port->IsConnected())
		{
			anyDest = true;
			break;
		}
	}
	if (!anyDest)
	{
		return;
	}

	mBuffer = FInventoryStack();
	for (auto* port : mPorts)
	{
		if (!port || mOutputs.Contains(port))
		{
			continue;
		}

		if (auto* box = port->GetFluidBox())
		{
			box->Content = 0.f;
		}
	}
}

int32 APPBuildablePriorityRouter::PullPipe(UFGPipeConnectionFactory* port, int32 maxLiters, float dt)
{
	if (!IsValid(port) || !port->IsConnected() || maxLiters <= 0)
	{
		return 0;
	}

	auto fluid = GetPortFluid(port);
	if (!fluid)
	{
		return 0;
	}

	if (!CanOutputFluid(fluid))
	{
		return 0;
	}

	if (mBuffer.HasItems() && mBuffer.Item.GetItemClass() != fluid)
	{
		return 0;
	}

	auto* box = port->GetFluidBox();
	auto take = 0;
	if (box)
	{
		take = FMath::Min(maxLiters, box->GetContentInLiters());
		if (take > 0)
		{
			box->RemoveContentInLiters(take);
			if (box->Content < 0.f)
			{
				box->Content = 0.f;
			}
		}
	}

	if (take < maxLiters)
	{
		FInventoryStack stack;
		port->Factory_PullPipeInput(dt, stack, fluid, maxLiters - take);
		if (stack.HasItems())
		{
			take += stack.NumItems;
		}
	}

	if (take <= 0)
	{
		return 0;
	}

	if (!mBuffer.HasItems())
	{
		mBuffer = FInventoryStack(take, fluid);
	}
	else
	{
		mBuffer.NumItems += take;
	}

	mMovedThisTick = 1;
	return take;
}

int32 APPBuildablePriorityRouter::PushPipe(UFGPipeConnectionFactory* port, float dt)
{
	if (!IsValid(port) || !port->IsConnected() || !mBuffer.HasItems())
	{
		return 0;
	}

	auto liters = port->Factory_PushPipeOutput(dt, mBuffer);
	mBuffer.NumItems -= liters;

	auto tick = GetFlowLiters(port, dt);
	if (mBuffer.HasItems() && liters < tick)
	{
		if (auto* box = port->GetFluidBox())
		{
			auto extra = FMath::Min3(mBuffer.NumItems, tick - liters, FMath::Max(0, box->GetAvailableSpaceInLiters()));
			if (extra > 0)
			{
				box->AddContentInLiters(extra);
				mBuffer.NumItems -= extra;
				liters += extra;
			}
		}
	}

	if (mBuffer.NumItems <= 0)
	{
		mBuffer = FInventoryStack();
	}

	if (liters > 0)
	{
		mMovedThisTick = 1;
	}

	return liters;
}
