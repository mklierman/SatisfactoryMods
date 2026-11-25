#include "InfiniteZoopTempZoopCommand.h"

#include "Command/CommandSender.h"
#include "InfiniteZoopSubsystem.h"
#include "InfiniteZoop_RCO.h"
#include "Subsystem/SubsystemActorManager.h"

AInfiniteZoopTempZoopCommand::AInfiniteZoopTempZoopCommand()
{
	bOnlyUsableByPlayer = true;
	CommandName = TEXT("TempZoop");
	Aliases.Add(TEXT("tzoop"));
	Usage = NSLOCTEXT("InfiniteZoop", "TempZoopUsage", "/TempZoop [Amount] - Sets the max Zoop amount for the session or until changed again.");
}

EExecutionStatus AInfiniteZoopTempZoopCommand::ExecuteCommand_Implementation(
	UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label)
{
	AFGPlayerController* Player = Sender->GetPlayer();
	UInfiniteZoop_RCO* RemoteCallObject = Player ? Player->GetRemoteCallObjectOfClass<UInfiniteZoop_RCO>() : nullptr;
	if (!RemoteCallObject)
	{
		Sender->SendChatMessage(TEXT("Temporary Zoop is unavailable."), FLinearColor::Red);
		return EExecutionStatus::UNCOMPLETED;
	}

	if (Arguments.IsEmpty())
	{
		const int32 Amount = RemoteCallObject->GetTemporaryZoopAmount();
		Sender->SendChatMessage(Amount > 0
			? FString::Printf(TEXT("Current Temp Zoop Amount: %d"), Amount)
			: FString(TEXT("Temporary Zoop Not Set")));
		return EExecutionStatus::COMPLETED;
	}

	if (Arguments.Num() != 1)
	{
		PrintCommandUsage(Sender);
		return EExecutionStatus::BAD_ARGUMENTS;
	}

	int64 Amount = 0;
	const FString& Argument = Arguments[0];
	if (!Argument.Equals(TEXT("off"), ESearchCase::IgnoreCase))
	{
		const TCHAR* Start = *Argument;
		TCHAR* End = nullptr;
		Amount = FCString::Strtoi64(Start, &End, 10);
		if (Start == End || *End != TEXT('\0') || Amount < 0)
		{
			PrintCommandUsage(Sender);
			return EExecutionStatus::BAD_ARGUMENTS;
		}
	}

	if (Amount == 0)
	{
		RemoteCallObject->SetTemporaryZoopAmount(0);
		Sender->SendChatMessage(TEXT("Temporary Zoop disabled"));
		return EExecutionStatus::COMPLETED;
	}

	USubsystemActorManager* SubsystemManager = GetWorld()->GetSubsystem<USubsystemActorManager>();
	const AInfiniteZoopSubsystem* ZoopSubsystem = SubsystemManager->GetSubsystemActor<AInfiniteZoopSubsystem>();
	if (!ZoopSubsystem)
	{
		Sender->SendChatMessage(TEXT("Temporary Zoop is unavailable."), FLinearColor::Red);
		return EExecutionStatus::UNCOMPLETED;
	}

	if (Amount > ZoopSubsystem->currentZoopAmount)
	{
		RemoteCallObject->SetTemporaryZoopAmount(0);
		Sender->SendChatMessage(TEXT("Unable to set Temporary Zoop higher than Max Zoop. Temporary Zoop disabled."), FLinearColor::Red);
		return EExecutionStatus::BAD_ARGUMENTS;
	}

	const int32 AcceptedAmount = static_cast<int32>(Amount);
	RemoteCallObject->SetTemporaryZoopAmount(AcceptedAmount);
	Sender->SendChatMessage(FString::Printf(TEXT("Temporary Zoop set to %d"), AcceptedAmount));
	return EExecutionStatus::COMPLETED;
}
