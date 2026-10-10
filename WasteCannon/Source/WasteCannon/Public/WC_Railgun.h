#pragma once
#include "CoreMinimal.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGInventoryComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "Resources/FGItemDescriptor.h"
#include "WC_Railgun.generated.h"

USTRUCT(BlueprintType)
struct FRailgunMovement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float start = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float target = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float current = 0.f;
};

UENUM(BlueprintType)
enum class ERailgunState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	AIMING UMETA(DisplayName = "Aiming"),
	RESETTING UMETA(DisplayName = "Resetting"),
	LOADING UMETA(DisplayName = "Loading")
};

UCLASS()
class AWC_Railgun : public AFGBuildableFactory
{
	GENERATED_BODY()
	AWC_Railgun();

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Factory_Tick(float dt) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Railgun_TickState(float dt);
	virtual void Railgun_TickAim(float dt);

	virtual bool ShouldSave_Implementation() const override;

	UFUNCTION()
	void ToIdle();
	UFUNCTION()
	void ToLoading();
	UFUNCTION()
	void ToAiming();

	UFUNCTION()
	void UpdateAimDirection(const FVector2D& direction);

	UFUNCTION()
	void SetAimDirectionDirect(const FVector2D& direction);

	UFUNCTION()
	void MovementComplete();

	TArray<UFGFactoryConnectionComponent*> InputConnections;

	UFUNCTION(BlueprintCallable)
	FVector2D GetAimDirection();

	UFUNCTION(BlueprintCallable)
	FRotator GetAimRotation();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AimToDirection(const FVector2D& direction);

	// Sets AimDirection from AimTargetWorldLocation (tower = X deg, barrel = Y deg in actor-local space, +X forward).
	UFUNCTION(BlueprintCallable, Category = "Waste Cannon|Aim")
	bool TryGetWorldTargetAimDirection(FVector2D& OutDirection);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	bool CanSeeSun();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnStartMoving();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnStopMoving();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnLoading();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ReadyForShoot();

	// Fires at the exact moment a shot is committed in C++ Shoot().
	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void OnShoot(const FTransform& ShotTransform);

	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void netSig_Shooted(const FVector& Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "Waste Cannon")
	void netSig_Finished();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPresentShot(uint32 Sequence, const FTransform& ShotTransform, const FVector& Location, const FVector& TargetLocation);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE class UFGInventoryComponent* GetStorageInventory() const
	{
		return StorageInventoryComponent;
	}

	UPROPERTY()
	UFGInventoryComponent* StorageInventoryComponent;

	UPROPERTY()
	UFGInventoryComponent* MissileInventoryComponent;

	UPROPERTY(BlueprintReadOnly, SaveGame, ReplicatedUsing = OnRep_AnimationState)
	ERailgunState animationState = ERailgunState::IDLE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MinTilt = 45;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxTilt = 78;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RotationSpeed = 10;

	// World position to aim at; used when bUseAimTargetWorldLocation is true (see ApplyAimFromWorldTarget).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Aim")
	FVector AimTargetWorldLocation = FVector::ZeroVector;

	// When true, barrel elevation from world aim is clamped to [MinTilt, MaxTilt].
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Aim")
	bool bClampWorldTargetBarrelToTiltLimits = true;

	UPROPERTY(BlueprintReadOnly)
	bool aimForShoot = false;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FRailgunMovement towerMovement;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FRailgunMovement barrelMovement;

	UPROPERTY(Replicated)
	FVector2D ReplicatedAimTarget = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Timing")
	float IdleSeconds = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Timing")
	float LoadingSeconds = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Timing")
	float ResettingSeconds = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Thresholds")
	int32 ShootWasteThreshold = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Aim")
	FVector2D IdleAimDirection = FVector2D(0.f, 45.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waste Cannon|Aim")
	float precisionThreshold = .001f;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Waste Cannon")
	float StateTime = 0.f;

protected:
	UFUNCTION()
	void OnRep_AnimationState(ERailgunState PreviousState);

	UFUNCTION()
	void OnRep_IsMoving();

	UFUNCTION()
	void OnRep_AimCommandSequence();

	UFUNCTION()
	void OnRep_HasAimTarget();

	UFUNCTION()
	void OnRep_ShotSequence();

	UFUNCTION()
	void RemoveAllWasteItems();

	void SetMoving(bool bNewMoving);
	void ResetStateTimer();
	void UpdateStateTimeFromServerClock();
	void ClientTickAim();
	float GetCurrentStateDuration() const;
	void PresentShot(uint32 Sequence, const FTransform& ShotTransform, const FVector& Location, const FVector& TargetLocation);

	UPROPERTY(Replicated)
	float StateStartServerTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_HasAimTarget)
	bool bReplicatedHasAimTarget = false;

	UPROPERTY(Replicated)
	FVector2D ReplicatedAimStart = FVector2D::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_AimCommandSequence)
	uint32 AimCommandSequence = 0;

	UPROPERTY(Replicated)
	FTransform ReplicatedShotTransform;

	UPROPERTY(Replicated)
	FVector ReplicatedShotTargetLocation = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_ShotSequence)
	uint32 ShotSequence = 0;

	uint32 LastPresentedShotSequence = 0;

	bool bStateTimerPausedForPower = false;
	bool bAimCompletionArmed = false;
	float PausedStateTime = 0.f;
	float LastClientAimUpdateTime = 0.f;
	FTimerHandle ClientAimTimerHandle;

	UPROPERTY(BlueprintReadOnly, SaveGame, ReplicatedUsing = OnRep_IsMoving)
	bool isMoving = true;
};
