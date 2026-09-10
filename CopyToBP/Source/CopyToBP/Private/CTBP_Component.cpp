

#include "CTBP_Component.h"
#include "FGPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Logging/StructuredLog.h"

void UCTBP_Component::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCTBP_Component, ShouldPasteToBP);
	DOREPLIFETIME_CONDITION(UCTBP_Component, PlayerClipboardData, COND_OwnerOnly);
}

UCTBP_Component::UCTBP_Component()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

UCTBP_Component* UCTBP_Component::FindOrAdd(AFGPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return nullptr;
	}

	if (UCTBP_Component* Existing = PlayerState->FindComponentByClass<UCTBP_Component>())
	{
		return Existing;
	}

	if (!PlayerState->HasAuthority())
	{
		return nullptr;
	}

	UCTBP_Component* Component = NewObject<UCTBP_Component>(PlayerState, TEXT("CTBP_Component"));
	Component->SetIsReplicated(true);
	PlayerState->AddInstanceComponent(Component);
	Component->RegisterComponent();
	return Component;
}

void UCTBP_Component::BeginPlay()
{
	Super::BeginPlay();
}

void UCTBP_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCTBP_Component::ServerSetPlayerClipboardData_Implementation(const TArray<FClipboardData>& data)
{
	PlayerClipboardData = data;
}

void UCTBP_Component::ServerSetShouldPasteToBP_Implementation(bool bNewValue)
{
	ShouldPasteToBP = bNewValue;
}
