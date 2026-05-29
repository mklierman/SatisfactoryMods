

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGSaveInterface.h"
#include "FGFactoryClipboard.h"
#include "CTBP_Component.generated.h"

USTRUCT(BlueprintType, Category = "CopyToBP")
struct FClipboardData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UObject> ObjectClass;

	UPROPERTY(BlueprintReadWrite)
	class UFGFactoryClipboardSettings* ClipboardSettings;
};

UCLASS(Blueprintable)
class COPYTOBP_API UCTBP_Component : public UActorComponent, public IFGSaveInterface
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ShouldSave_Implementation() const override { return true; }

public:	
	// Sets default values for this component's properties
	UCTBP_Component();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetShouldPasteToBP(bool bNewValue);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetPlayerClipboardData(const TArray<FClipboardData>& data);

	UPROPERTY(BlueprintReadWrite, Replicated, SaveGame)
	bool ShouldPasteToBP = true;

	UPROPERTY(BlueprintReadWrite, Replicated)
	TArray<FClipboardData> PlayerClipboardData;
};
