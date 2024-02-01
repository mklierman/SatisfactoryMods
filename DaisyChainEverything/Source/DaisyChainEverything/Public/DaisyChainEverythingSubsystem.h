#pragma once
#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGOptionInterface.h"
#include "DaisyChainEverythingSubsystem.generated.h"


UCLASS()
class DAISYCHAINEVERYTHING_API ADaisyChainEverythingSubsystem : public AModSubsystem
{
	GENERATED_BODY()
	public:
		ADaisyChainEverythingSubsystem();
		virtual void Init() override;
		void OnSessionSettingUpdated(const FString StrID, FVariant value);
		FOnOptionUpdated OnOptionUpdatedDelegate;
};

