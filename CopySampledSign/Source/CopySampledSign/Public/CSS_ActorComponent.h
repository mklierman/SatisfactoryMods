#pragma once

#include "FGSignTypes.h"
#include "CSS_ActorComponent.generated.h"



USTRUCT(BlueprintType)
struct FReplicatedPrefabSignData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TSoftClassPtr<UFGSignPrefabWidget> PrefabLayout = {};

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TArray<FString> TextElementDataKeys = {};
	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TArray<FString> TextElementDataValues = {};

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TArray<FString> IconElementDataKeys = {};
	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TArray<int32> IconElementDataValues = {};

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	FLinearColor ForegroundColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	FLinearColor BackgroundColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	FLinearColor AuxiliaryColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	float Emissive = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	float Glossiness = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Prefab Sign Data")
	TSubclassOf<UFGSignTypeDescriptor> SignTypeDesc;

	// For clients to know if they should call the server RPC to set the data when they receive new data (if from replication don't trigger the RPC as it came from an RPC)
	bool IsFromReplication = false;
};

UCLASS(Blueprintable)
class UCSS_ActorComponent : public UActorComponent
{
	GENERATED_BODY()
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
	FReplicatedPrefabSignData SignData;
};
