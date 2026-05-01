#pragma once
#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGInventoryComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "FGItemDescriptor.h"
#include "WC_Railgun.generated.h"

USTRUCT(BlueprintType)
struct FRailgunMovement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float start;

	UPROPERTY(BlueprintReadOnly)
	float target;

	UPROPERTY(BlueprintReadOnly)
	float current;
};

UENUM(BlueprintType)
enum class ERailgunState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	AIMING UMETA(DisplayName = "Aiming"),
	SHOOTING UMETA(DisplayName = "Shooting"),
	RESETTING UMETA(DisplayName = "Resetting"),
	LOADING UMETA(DisplayName = "Loading"),
	GUARDING UMETA(DisplayName = "Guarding")
};

UCLASS()
class AWC_Railgun : public AFGBuildableFactory
{
	GENERATED_BODY()
	AWC_Railgun();

public:
	virtual void BeginPlay() override;
	virtual void Factory_Tick(float dt) override;

	virtual bool ShouldSave_Implementation() const override;

	UFUNCTION()
	void UpdateAimDirection(const FVector& position, const FVector2D& direction);

	UFUNCTION()
	void SetAimDirectionDirect(const FVector2D& direction);

	void MovementComplete();

	TArray<UFGFactoryConnectionComponent*> InputConnections;

	UFUNCTION(BlueprintCallable)
	FVector2D GetAimDirection();

	UFUNCTION(BlueprintCallable)
	FRotator GetAimRotation();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ResetTower();

	UFUNCTION(BlueprintCallable)
	void AnimationComplete();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void Shoot(const FTransform& position);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AimToDirection(const FVector2D& direction, ERailgunState newState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnStartMoving();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnStopMoving();

	UFUNCTION(BlueprintImplementableEvent)
	void OnAimChanged();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ReadyForShoot();

	// Fires at the exact moment a shot is committed in C++ Shoot().
	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void OnShoot(const FTransform& ShotTransform);

	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void OnShotTriggered(const FTransform& ShotTransform);

	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void netSig_Shooted(const FVector& Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void netSig_Finished();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class UFGInventoryComponent* GetStorageInventory() const
	{
		return StorageInventoryComponent;
	}

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class UFGInventoryComponent* GetFuelInventory() const
	{
		return FuelInventoryComponent;
	}

	UPROPERTY()
	UFGInventoryComponent* StorageInventoryComponent;

	UPROPERTY()
	UFGInventoryComponent* FuelInventoryComponent;



	UPROPERTY(BlueprintReadOnly, SaveGame)
	ERailgunState animationState = ERailgunState::IDLE;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool Loaded = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MinTilt = 45;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxTilt = 78;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RotationSpeed = 10;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVector TargetPosition;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVector2D AimDirection;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool isMoving;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FRailgunMovement towerMovement;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FRailgunMovement barrelMovement;

	UFUNCTION(BlueprintCallable, Category = "Waste Cannon")
	int32 GetWasteItemCount() const;

	UFUNCTION(BlueprintCallable, Category = "Waste Cannon")
	bool IsWasteInventoryFull() const;

	UFUNCTION(BlueprintCallable, Category = "Waste Cannon")
	bool TryAutoShoot();

	UFUNCTION(BlueprintPure, Category = "Waste Cannon|Ammo")
	bool CanShootWithCurrentAmmo() const;

protected:
	UFUNCTION()
	void EvaluateAutoShoot();
	UFGInventoryComponent* ResolveInventoryForConnection(UFGFactoryConnectionComponent* Connection) const;
	UFUNCTION()
	void FinishShotCycleFallback();

	bool ConsumeOneFuelItem();
	void RemoveAllWasteItems();

	// If true, ammo/fuel must match one of AllowedAmmoDescriptors.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon|Ammo")
	bool bUseAmmoWhitelist = true;

	// Exact item descriptor classes accepted as ammo/fuel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon|Ammo")
	TArray<TSubclassOf<UFGItemDescriptor>> AllowedAmmoDescriptors;

	// If waste count reaches this value, the railgun auto-fires.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon")
	int32 AutoShootWasteThreshold = 200;

	// Poll interval for auto-shoot checks on the server.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon")
	float AutoShootCheckIntervalSeconds = 0.25f;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Waste Cannon")
	int32 LastShotWasteCount = 0;

	FTimerHandle AutoShootTimerHandle;
	FTimerHandle ShotCycleFallbackTimerHandle;
};

