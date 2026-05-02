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

	// Sets AimDirection from AimTargetWorldLocation (tower = X deg, barrel = Y deg in actor-local space, +X forward).
	UFUNCTION(BlueprintCallable, Category = "Waste Cannon|Aim")
	void ApplyAimFromWorldTarget();

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

	// When false: no shooting, no LOADING, no aim movement. TryAutoShoot still calls ReadyForShoot first so BP can set this true before barrel/LOADING. Post-shot RESETTING still runs.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon")
	bool ShouldShoot = true;

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

	// World position to aim at; used when bUseAimTargetWorldLocation is true (see ApplyAimFromWorldTarget).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon|Aim")
	FVector AimTargetWorldLocation = FVector::ZeroVector;

	// If true, LOADING -> AIMING uses ApplyAimFromWorldTarget() so AimDirection is derived from AimTargetWorldLocation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon|Aim")
	bool bUseAimTargetWorldLocation = false;

	// When true, barrel elevation from world aim is clamped to [MinTilt, MaxTilt].
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Waste Cannon|Aim")
	bool bClampWorldTargetBarrelToTiltLimits = true;

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
	/** When fuel uses the ammo whitelist, only these descriptors may enter via the fuel input (see Factory_Tick). */
	bool IsItemClassAllowedOnFuelInput(TSubclassOf<UFGItemDescriptor> ItemClass) const;

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

