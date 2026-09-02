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
	bool Success = false;
	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		Success = Subsystem->SetDestinationName(GetOwnerPlayerCharacter(), Entrance, NewName);
	}
	Client_ReceiveDestinationNameResult(Success);
}

bool UHypertubeDestinationRCO::Server_SetDestinationName_Validate(AFGPipeHyperStart* Entrance, const FString& NewName)
{
	const FString TrimmedName = NewName.TrimStartAndEnd();
	return IsValid(GetOwnerPlayerCharacter()) && IsValid(Entrance) && !TrimmedName.IsEmpty() && TrimmedName.Len() <= HypertubeRcoMaxDestinationNameLength;
}

void UHypertubeDestinationRCO::Client_ReceiveDestinationNameResult_Implementation(bool Success)
{
	OnDestinationNameSet.Broadcast(Success);
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

void UHypertubeDestinationRCO::Server_RequestDestinationOptions_Implementation(AFGPipeHyperStart* Source)
{
	AFGCharacterPlayer* Player = GetOwnerPlayerCharacter();

	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		FString SourceDisplayName;
		FHypertubeDestinationRecord SourceRecord;
		if (Subsystem->GetDestinationForEntrance(Source, SourceRecord))
		{
			SourceDisplayName = SourceRecord.DisplayName;
		}
		Client_ReceiveDestinationOptions(SourceDisplayName, Subsystem->GetReachableDestinationOptions(Player, Source));
	}
}

bool UHypertubeDestinationRCO::Server_RequestDestinationOptions_Validate(AFGPipeHyperStart* Source)
{
	return IsValid(GetOwnerPlayerCharacter()) && IsValid(Source);
}

void UHypertubeDestinationRCO::Server_SelectDestinationById_Implementation(AFGPipeHyperStart* Source, FGuid DestinationId)
{
	AFGCharacterPlayer* Player = GetOwnerPlayerCharacter();

	if (AHypertubeDestinationSubsystem* Subsystem = AHypertubeDestinationSubsystem::Get(this))
	{
		Subsystem->SelectDestinationById(Player, Source, DestinationId);
	}
}

bool UHypertubeDestinationRCO::Server_SelectDestinationById_Validate(AFGPipeHyperStart* Source, FGuid DestinationId)
{
	return IsValid(GetOwnerPlayerCharacter()) && IsValid(Source) && DestinationId.IsValid();
}

void UHypertubeDestinationRCO::Client_ReceiveDestinationOptions_Implementation(const FString& SourceDisplayName, const TArray<FHypertubeDestinationOption>& Options)
{
	OnDestinationOptionsReceived.Broadcast(SourceDisplayName, Options);
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
