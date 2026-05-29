#pragma once

#include "Hologram/FGHologram.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ScrollMode.h"
#include "InfiniteGizmo.generated.h"


USTRUCT(BlueprintType)
struct FGizmoData {
	GENERATED_BODY()

	//BP properties
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* staticMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator meshRotation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float meshScale;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLinearColor color;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EScrollMode scrollMode;

	//Runtime only
	UPROPERTY()
	UStaticMeshComponent* component;
	UPROPERTY()
	UMaterialInstanceDynamic* material;

	float progress;
};


UCLASS(Blueprintable)
class UInfiniteGizmo : public USceneComponent
{
	GENERATED_BODY()
public:

	UInfiniteGizmo(const FObjectInitializer& ObjectInitializer);
	virtual void TickComponent(float delta, ELevelTick tickType, FActorComponentTickFunction* callback) override;

	static bool ToggleVisibility();
	static bool IsGizmoVisible();
	static void UpdatePivot(FVector position);
	static void Create(AFGHologram* hologram);	
	static void Destroy();

protected:
	bool bIsVisible;
	TWeakObjectPtr<const AFGHologram> mHologram;

	/* Setup from the BP */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FGizmoData> mGizmos;

	void Init(const AFGHologram* holo);
	//Toggle Visibility
	bool Toggle();

};