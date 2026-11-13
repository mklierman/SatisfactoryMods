#pragma once

#include "CoreMinimal.h"
#include "FGIconLibrary.h"
#include "FGRemoteCallObject.h"
#include "GraffitiRemoteCallObject.generated.h"

UCLASS()
class GRAFFITI_API UGraffitiRemoteCallObject : public UFGRemoteCallObject
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Graffiti|Networking", meta = (WorldContext = "WorldContext"))
	static UGraffitiRemoteCallObject* Get(UObject* WorldContext);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Graffiti|Networking")
	void ServerSpray(
		const FPersistentGlobalIconId& IconId,
		FLinearColor Tint,
		float Size,
		float RollDegrees,
		float MaximumDistance = 1500.0f,
		bool bMirrorHorizontal = false,
		bool bSnapToGrid = false,
		float GridSize = 100.0f);

	bool ServerSpray_Validate(
		const FPersistentGlobalIconId& IconId,
		FLinearColor Tint,
		float Size,
		float RollDegrees,
		float MaximumDistance,
		bool bMirrorHorizontal,
		bool bSnapToGrid,
		float GridSize);

	void ServerSpray_Implementation(
		const FPersistentGlobalIconId& IconId,
		FLinearColor Tint,
		float Size,
		float RollDegrees,
		float MaximumDistance,
		bool bMirrorHorizontal,
		bool bSnapToGrid,
		float GridSize);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Graffiti|Networking")
	void ServerErase(float MaximumDistance = 1500.0f);
	bool ServerErase_Validate(float MaximumDistance);
	void ServerErase_Implementation(float MaximumDistance);

private:
	bool TraceFromOwner(float MaximumDistance, FHitResult& OutHit) const;

	UPROPERTY(Replicated)
	bool DummyReplicatedField = true;
};
