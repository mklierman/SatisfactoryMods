#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "HypertubeDestinationRCO.generated.h"

class AFGPipeHyperStart;

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
	void Server_CancelRoute();

private:
	UPROPERTY(Replicated)
	bool bForceNetField = false;
};
