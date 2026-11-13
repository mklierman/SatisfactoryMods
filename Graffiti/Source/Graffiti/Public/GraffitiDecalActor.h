#pragma once

#include "CoreMinimal.h"
#include "FGIconLibrary.h"
#include "FGSaveInterface.h"
#include "Replication/FGStaticReplicatedActor.h"
#include "GraffitiDecalActor.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS(BlueprintType)
class GRAFFITI_API AGraffitiDecalActor : public AFGStaticReplicatedActor, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	AGraffitiDecalActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& OutDependentObjects) override;
	virtual bool NeedTransform_Implementation() override;
	virtual bool ShouldSave_Implementation() const override;

	void InitializeGraffiti(const FPersistentGlobalIconId& InIconId, const FLinearColor& InTint, float InSize, bool bInMirrorHorizontal, int32 InLayerOrder);

	UFUNCTION(BlueprintPure, Category = "Graffiti")
	const FPersistentGlobalIconId& GetIconId() const { return IconId; }

	UFUNCTION(BlueprintPure, Category = "Graffiti")
	FLinearColor GetTint() const { return Tint; }

	UFUNCTION(BlueprintPure, Category = "Graffiti")
	float GetGraffitiSize() const { return Size; }

	UFUNCTION(BlueprintPure, Category = "Graffiti")
	bool IsMirroredHorizontally() const { return bMirrorHorizontal; }

	UFUNCTION(BlueprintPure, Category = "Graffiti")
	int32 GetLayerOrder() const { return LayerOrder; }

protected:
	UFUNCTION()
	void OnRep_GraffitiData();

	void RefreshDecal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Graffiti")
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Graffiti")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_GraffitiData, BlueprintReadOnly, Category = "Graffiti")
	FPersistentGlobalIconId IconId;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_GraffitiData, BlueprintReadOnly, Category = "Graffiti")
	FLinearColor Tint = FLinearColor::White;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_GraffitiData, BlueprintReadOnly, Category = "Graffiti", meta = (ClampMin = "25.0", ClampMax = "1000.0"))
	float Size = 200.0f;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_GraffitiData, BlueprintReadOnly, Category = "Graffiti")
	bool bMirrorHorizontal = false;

	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_GraffitiData, BlueprintReadOnly, Category = "Graffiti")
	int32 LayerOrder = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
};
