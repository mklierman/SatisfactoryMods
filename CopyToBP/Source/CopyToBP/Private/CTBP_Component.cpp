


#include "CTBP_Component.h"
#include <Net/UnrealNetwork.h>
#include "Logging/StructuredLog.h"

void UCTBP_Component::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCTBP_Component, ShouldPasteToBP);
	DOREPLIFETIME(UCTBP_Component, PlayerClipboardData);
}

// Sets default values for this component's properties
UCTBP_Component::UCTBP_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UCTBP_Component::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCTBP_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCTBP_Component::ServerSetPlayerClipboardData_Implementation(const TArray<FClipboardData>& data)
{
	PlayerClipboardData = data;
}

void UCTBP_Component::ServerSetShouldPasteToBP_Implementation(bool bNewValue)
{
	ShouldPasteToBP = bNewValue;
	//UE_LOGFMT(CTBP_Log, Display, "Set ShouldPaste = {0}", bNewValue);
}

