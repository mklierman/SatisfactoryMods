#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "HypertubeDestinationTypes.h"
#include "HypertubeDestinationRCO.generated.h"

class AFGPipeHyperStart;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHypertubeDestinationOptionsReceived, const FString&, SourceDisplayName, const TArray<FHypertubeDestinationOption>&, Options);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHypertubeDestinationNameSet, bool, Success);

UCLASS(NotBlueprintable)
class HYPERTUBEDESTINATIONS_API UHypertubeDestinationRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

public:
	UHypertubeDestinationRCO();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Hypertube Destinations")
	void Server_SetDestinationName(AFGPipeHyperStart* Entrance, const FString& NewName);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Hypertube Destinations")
	void Server_SelectDestination(AFGPipeHyperStart* Source, AFGPipeHyperStart* Destination);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Hypertube Destinations")
	void Server_RequestDestinationOptions(AFGPipeHyperStart* Source);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Hypertube Destinations")
	void Server_SelectDestinationById(AFGPipeHyperStart* Source, FGuid DestinationId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Hypertube Destinations")
	void Server_CancelRoute();

	UPROPERTY(BlueprintAssignable, Category = "Hypertube Destinations")
	FHypertubeDestinationOptionsReceived OnDestinationOptionsReceived;

	UPROPERTY(BlueprintAssignable, Category = "Hypertube Destinations")
	FHypertubeDestinationNameSet OnDestinationNameSet;

private:
	UFUNCTION(Client, Reliable)
	void Client_ReceiveDestinationNameResult(bool Success);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveDestinationOptions(const FString& SourceDisplayName, const TArray<FHypertubeDestinationOption>& Options);

	UPROPERTY(Replicated)
	bool bForceNetField = false;
};
