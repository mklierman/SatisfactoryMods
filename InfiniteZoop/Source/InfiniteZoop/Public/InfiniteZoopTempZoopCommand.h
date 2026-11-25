#pragma once

#include "Command/ChatCommandInstance.h"
#include "InfiniteZoopTempZoopCommand.generated.h"

UCLASS()
class AInfiniteZoopTempZoopCommand : public AChatCommandInstance
{
	GENERATED_BODY()

public:
	AInfiniteZoopTempZoopCommand();

	EExecutionStatus ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label) override;
};
