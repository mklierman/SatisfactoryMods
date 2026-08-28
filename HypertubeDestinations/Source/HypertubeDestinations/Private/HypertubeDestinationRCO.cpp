#include "HypertubeDestinationRCO.h"

#include "Buildables/FGPipeHyperStart.h"
#include "FGCharacterPlayer.h"
#include "HypertubeDestinationSubsystem.h"
#include "Net/UnrealNetwork.h"

namespace
{
constexpr int32 HypertubeRcoMaxDestinationNameLength = 64;
}

UHypertubeDestinationRCO::UHypertubeDestinationRCO() = default;

void UHypertubeDestinationRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHypertubeDestinationRCO, bForceNetField);
}

void UHypertubeDestinationRCO::Server_SetDestinationName_Implementation(AFGPipeHyperStart* Entrance, const FString& NewName)
{
	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		Subsystem->SetDestinationName(Entrance, NewName);
	}
}

bool UHypertubeDestinationRCO::Server_SetDestinationName_Validate(AFGPipeHyperStart* Entrance, const FString& NewName)
{
	const FString TrimmedName = NewName.TrimStartAndEnd();
	return IsValid(GetOwnerPlayerCharacter()) && IsValid(Entrance) && !TrimmedName.IsEmpty() && TrimmedName.Len() <= HypertubeRcoMaxDestinationNameLength;
}

void UHypertubeDestinationRCO::Server_SelectDestination_Implementation(AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination)
{
	AFGCharacterPlayer* Player = GetOwnerPlayerCharacter();

	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		Subsystem->SelectDestination(Player, Source, Destination);
	}
}

bool UHypertubeDestinationRCO::Server_SelectDestination_Validate(AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination)
{
	return IsValid(GetOwnerPlayerCharacter()) && IsValid(Source) && IsValid(Destination) && Source != Destination;
}

void UHypertubeDestinationRCO::Server_CancelRoute_Implementation()
{
	AFGCharacterPlayer* Player = GetOwnerPlayerCharacter();

	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		Subsystem->ClearActiveRoute(Player);
	}
}

bool UHypertubeDestinationRCO::Server_CancelRoute_Validate()
{
	return IsValid(GetOwnerPlayerCharacter());
}
