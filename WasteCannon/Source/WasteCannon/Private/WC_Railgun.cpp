#include "WC_Railgun.h"
#include "TimerManager.h"
#include "Logging/StructuredLog.h"
#include "Async/Async.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

//DEFINE_LOG_CATEGORY(WasteCannon_Log);
#pragma optimize("", off)

AWC_Railgun::AWC_Railgun()
{
	this->StorageInventoryComponent = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("StorageInventory"));
	this->MissileInventoryComponent = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("MissileInventory"));
	this->MinTilt = 25;
	this->MaxTilt = 81;
	this->RotationSpeed = 10;
	this->animationState = ERailgunState::IDLE;
}

void AWC_Railgun::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetWorld())
	{
		if (StorageInventoryComponent)
		{
			StorageInventoryComponent->SetReplicationRelevancyOwner(this);
			StorageInventoryComponent->SetLocked(false);
			StorageInventoryComponent->Resize(24);
			//StorageInventoryComponent->AddArbitrarySlotSize(0, 20);
		}
		if (MissileInventoryComponent)
		{
			MissileInventoryComponent->SetReplicationRelevancyOwner(this);
			MissileInventoryComponent->SetLocked(false);
			MissileInventoryComponent->Resize(24);
			//StorageInventoryComponent->AddArbitrarySlotSize(0, 20);
		}

		InputConnections.Empty();
		FOR_EACH_FACTORY_INLINE_COMPONENTS(Connection)
		{
			if (Connection && Connection->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
			{
				InputConnections.Add(Connection);
				Connection->SetInventory(StorageInventoryComponent);
				Connection->SetInventoryAccessIndex(0);
			}
		}

		const float Duration = GetCurrentStateDuration();
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		const float ServerTime = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
		StateStartServerTime = ServerTime - (FMath::Clamp(StateTime, 0.f, 1.f) * Duration);
		ReplicatedAimTarget = FVector2D(towerMovement.target, barrelMovement.target);
	}
	else
	{
		LastPresentedShotSequence = ShotSequence;
		LastClientAimUpdateTime = GetWorld()->GetTimeSeconds();
		GetWorldTimerManager().SetTimer(ClientAimTimerHandle, this, &AWC_Railgun::ClientTickAim, 1.f / 60.f, true);
	}
}

void AWC_Railgun::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(ClientAimTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AWC_Railgun::ClientTickAim()
{
	if (HasAuthority() || !GetWorld()) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float DeltaSeconds = FMath::Max(0.f, CurrentTime - LastClientAimUpdateTime);
	LastClientAimUpdateTime = CurrentTime;

	if (animationState == ERailgunState::RESETTING || HasPower())
	{
		UpdateStateTimeFromServerClock();
	}

	towerMovement.current = FMath::FInterpConstantTo(towerMovement.current, towerMovement.target, DeltaSeconds, RotationSpeed);
	barrelMovement.current = FMath::FInterpConstantTo(barrelMovement.current, barrelMovement.target, DeltaSeconds, RotationSpeed);
}

void AWC_Railgun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWC_Railgun, animationState);
	DOREPLIFETIME(AWC_Railgun, isMoving);
	DOREPLIFETIME(AWC_Railgun, StateStartServerTime);
	DOREPLIFETIME(AWC_Railgun, bReplicatedHasAimTarget);
	DOREPLIFETIME(AWC_Railgun, ReplicatedAimStart);
	DOREPLIFETIME(AWC_Railgun, ReplicatedAimTarget);
	DOREPLIFETIME(AWC_Railgun, AimCommandSequence);
	DOREPLIFETIME(AWC_Railgun, ReplicatedShotTransform);
	DOREPLIFETIME(AWC_Railgun, ReplicatedShotTargetLocation);
	DOREPLIFETIME(AWC_Railgun, ShotSequence);
}

void AWC_Railgun::Factory_Tick(float dt)
{
	Super::Factory_Tick(dt);
	if (!HasAuthority())
	{
		return;
	}

	if (HasPower())
	{
		for (UFGFactoryConnectionComponent* Connection : InputConnections)
		{
			if (!Connection || !Connection->IsConnected())
			{
				continue;
			}

			if (!GetStorageInventory() || GetStorageInventory()->IsLocked())
			{
				continue;
			}

			TArray<FInventoryItem> PeekItems;
			while (Connection->Factory_PeekOutput(PeekItems))
			{
				if (PeekItems.IsEmpty() || !PeekItems[0].IsValid() ||
					!GetStorageInventory()->HasEnoughSpaceForItem(PeekItems[0]))
				{
					break;
				}

				float Offset;
				FInventoryItem Item;
				if (!Connection->Factory_GrabOutput(Item, Offset, PeekItems[0].GetItemClass()))
				{
					break;
				}

				if (!GetStorageInventory()->AddItem(Item))
				{
					break;
				}

				PeekItems.Reset();
			}
		}
	}

	Railgun_TickState(dt);
	if (HasPower()) Railgun_TickAim(dt);
}

void AWC_Railgun::Railgun_TickState(float dt)
{
	if (animationState == ERailgunState::RESETTING)
	{
		bStateTimerPausedForPower = false;
		UpdateStateTimeFromServerClock();
		if (StateTime >= 1.f)
		{
			ToIdle();
		}
		return;
	}

	if (!HasPower())
	{
		if (!bStateTimerPausedForPower)
		{
			UpdateStateTimeFromServerClock();
			PausedStateTime = StateTime;
			bStateTimerPausedForPower = true;
		}
		return;
	}

	if (bStateTimerPausedForPower)
	{
		StateTime = PausedStateTime;
		ResetStateTimer();
		const float Duration = GetCurrentStateDuration();
		StateStartServerTime -= FMath::Clamp(PausedStateTime, 0.f, 1.f) * Duration;
		StateTime = PausedStateTime;
		bStateTimerPausedForPower = false;
		ForceNetUpdate();
	}
	else
	{
		UpdateStateTimeFromServerClock();
	}

	switch (animationState)
	{
	case ERailgunState::IDLE:

		if (StateTime >= 1.f && StorageInventoryComponent->GetNumItems(nullptr) >= ShootWasteThreshold)
		{
			ToLoading();
		}
		break;

	case ERailgunState::LOADING:
		if (StateTime >= 1.f)
		{
			ToAiming();
		}
		break;

	default:
		break;
	}
}

void AWC_Railgun::Railgun_TickAim(float dt)
{
	if (animationState == ERailgunState::RESETTING) return;

	if (HasAuthority())
	{
		FVector2D DesiredDirection = IdleAimDirection;
		aimForShoot = false;

		if (animationState == ERailgunState::AIMING)
		{
			aimForShoot = TryGetWorldTargetAimDirection(DesiredDirection);
		}

		bReplicatedHasAimTarget = aimForShoot;
		AimToDirection(DesiredDirection);
	}

	this->towerMovement.current = FMath::FInterpConstantTo(this->towerMovement.current, this->towerMovement.target, dt, this->RotationSpeed);
	this->barrelMovement.current = FMath::FInterpConstantTo(this->barrelMovement.current, this->barrelMovement.target, dt, this->RotationSpeed);

	if (!HasAuthority()) return;
	if (!bAimCompletionArmed)
	{
		bAimCompletionArmed = true;
		return;
	}

	if (FMath::IsNearlyEqual(towerMovement.current, towerMovement.target, precisionThreshold) &&
		FMath::IsNearlyEqual(barrelMovement.current, barrelMovement.target, precisionThreshold))
	{
		if (isMoving)
		{
			SetAimDirectionDirect(FVector2D(towerMovement.target, barrelMovement.target));
			SetMoving(false);
			AsyncTask(ENamedThreads::GameThread, [this]()
				{
					this->MovementComplete();
				});
		}
	}
	else if (animationState == ERailgunState::AIMING && aimForShoot && !isMoving)
	{
		SetMoving(true);
	}
}

void AWC_Railgun::ToLoading()
{
	TArray<FInventoryStack> stacks;
	StorageInventoryComponent->GetInventoryStacks(stacks);
	MissileInventoryComponent->AddStacks(stacks);
	StorageInventoryComponent->Empty();
	aimForShoot = false;
	bReplicatedHasAimTarget = false;
	bAimCompletionArmed = false;
	animationState = ERailgunState::LOADING;
	ResetStateTimer();
	SetMoving(false);
	OnLoading();
	FlushNetDormancy();
	ForceNetUpdate();
}

void AWC_Railgun::ToAiming()
{
	animationState = ERailgunState::AIMING;
	ResetStateTimer();
	FVector2D DesiredDirection = IdleAimDirection;
	aimForShoot = TryGetWorldTargetAimDirection(DesiredDirection);
	bReplicatedHasAimTarget = aimForShoot;
	ReplicatedAimStart = FVector2D(towerMovement.current, barrelMovement.current);
	AimToDirection(DesiredDirection);
	++AimCommandSequence;
	bAimCompletionArmed = false;
	SetMoving(true);
	FlushNetDormancy();
	ForceNetUpdate();
}

void AWC_Railgun::MovementComplete()
{
	if (!HasAuthority() || animationState != ERailgunState::AIMING || !aimForShoot || !CanSeeSun())
	{
		return;
	}

	animationState = ERailgunState::RESETTING;
	ResetStateTimer();

	const FTransform ShotTransform = GetActorTransform();
	const FVector ShotLocation = GetActorLocation();
	OnShoot(ShotTransform);
	RemoveAllWasteItems();
	ReplicatedShotTransform = ShotTransform;
	ReplicatedShotTargetLocation = AimTargetWorldLocation;
	++ShotSequence;
	FlushNetDormancy();
	ForceNetUpdate();
	MulticastPresentShot(ShotSequence, ShotTransform, ShotLocation, ReplicatedShotTargetLocation);
}

void AWC_Railgun::ToIdle()
{
	aimForShoot = false;
	bReplicatedHasAimTarget = false;
	animationState = ERailgunState::IDLE;
	ResetStateTimer();
	ReplicatedAimStart = FVector2D(towerMovement.current, barrelMovement.current);
	AimToDirection(IdleAimDirection);
	++AimCommandSequence;
	bAimCompletionArmed = false;
	SetMoving(true);
	netSig_Finished();
	FlushNetDormancy();
	ForceNetUpdate();
}

void AWC_Railgun::MulticastPresentShot_Implementation(uint32 Sequence, const FTransform& ShotTransform, const FVector& Location, const FVector& TargetLocation)
{
	PresentShot(Sequence, ShotTransform, Location, TargetLocation);
}

bool AWC_Railgun::ShouldSave_Implementation() const
{
	return true;
}

void AWC_Railgun::UpdateAimDirection(const FVector2D& direction)
{
	FVector2D DesiredDirection = direction;
	DesiredDirection.X = FMath::UnwindDegrees(DesiredDirection.X);
	DesiredDirection.Y = FMath::Clamp(DesiredDirection.Y, MinTilt, MaxTilt);

	towerMovement.target = DesiredDirection.X;
	barrelMovement.target = DesiredDirection.Y;
	if (HasAuthority())
	{
		ReplicatedAimTarget = DesiredDirection;
	}

	towerMovement.start = towerMovement.current;
	barrelMovement.start = barrelMovement.current;

	if (this->towerMovement.start + 180 < this->towerMovement.target) this->towerMovement.start += 360;
	if (this->towerMovement.start - 180 > this->towerMovement.target) this->towerMovement.start -= 360;
	this->towerMovement.current = this->towerMovement.start;
}

void AWC_Railgun::SetAimDirectionDirect(const FVector2D & direction)
{
	FVector2D DesiredDirection = direction;
	DesiredDirection.X = FMath::UnwindDegrees(DesiredDirection.X);
	DesiredDirection.Y = FMath::Clamp(DesiredDirection.Y, MinTilt, MaxTilt);

	this->towerMovement.start = direction.X;
	this->barrelMovement.start = direction.Y;
	this->towerMovement.target = direction.X;
	this->barrelMovement.target = direction.Y;
	this->towerMovement.current = direction.X;
	this->barrelMovement.current = direction.Y;
}

FVector2D AWC_Railgun::GetAimDirection()
{
	return FVector2D(this->towerMovement.current, this->barrelMovement.current);
}

FRotator AWC_Railgun::GetAimRotation()
{
	return FRotator(this->towerMovement.current, this->barrelMovement.current, 0);
}

void AWC_Railgun::AimToDirection(const FVector2D & direction)
{
	UpdateAimDirection(direction);
}

bool AWC_Railgun::TryGetWorldTargetAimDirection(FVector2D& OutDirection)
{
	const FVector Origin = GetActorLocation();
	FVector ToTarget = AimTargetWorldLocation - Origin;
	const float DistSq = ToTarget.SizeSquared();
	if (DistSq < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// Unit direction to target in actor space (+X forward, +Y right, +Z up).
	const FVector LocalDir = GetActorTransform().InverseTransformVectorNoScale(ToTarget.GetSafeNormal());

	const float TowerDeg = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X)));
	float BarrelDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Z,
		FMath::Sqrt(FMath::Square(LocalDir.X) + FMath::Square(LocalDir.Y))));

	if (bClampWorldTargetBarrelToTiltLimits) 
	{
		//UE_LOGFMT(WasteCannon_Log, Display, "Try aiming the sun. Tilt: {0}", BarrelDeg);
		const bool bWithinTilt = (BarrelDeg >= MinTilt && BarrelDeg <= MaxTilt);
		if (!bWithinTilt)
		{
			return false;
		}
	}

	OutDirection = FVector2D(TowerDeg, BarrelDeg);
	return true;
}

void AWC_Railgun::RemoveAllWasteItems()
{
	if (!MissileInventoryComponent)
	{
		return;
	}
	MissileInventoryComponent->Empty();
}

void AWC_Railgun::OnRep_AnimationState(ERailgunState PreviousState)
{
	UpdateStateTimeFromServerClock();

	if (animationState == ERailgunState::LOADING)
	{
		OnLoading();
	}
	else if (animationState == ERailgunState::IDLE && PreviousState != ERailgunState::IDLE)
	{
		netSig_Finished();
	}
}

void AWC_Railgun::OnRep_IsMoving()
{
	if (isMoving)
	{
		OnStartMoving();
	}
	else
	{
		OnStopMoving();
	}
}

void AWC_Railgun::OnRep_AimCommandSequence()
{
	towerMovement.current = ReplicatedAimStart.X;
	barrelMovement.current = ReplicatedAimStart.Y;
	UpdateAimDirection(ReplicatedAimTarget);
}

void AWC_Railgun::OnRep_HasAimTarget()
{
	aimForShoot = bReplicatedHasAimTarget;
}

void AWC_Railgun::OnRep_ShotSequence()
{
	PresentShot(ShotSequence, ReplicatedShotTransform, ReplicatedShotTransform.GetLocation(), ReplicatedShotTargetLocation);
}

void AWC_Railgun::PresentShot(uint32 Sequence, const FTransform& ShotTransform, const FVector& Location, const FVector& TargetLocation)
{
	if (Sequence == 0 || Sequence == LastPresentedShotSequence)
	{
		return;
	}

	LastPresentedShotSequence = Sequence;
	if (!HasActorBegunPlay())
	{
		return;
	}

	if (!HasAuthority())
	{
		AimTargetWorldLocation = TargetLocation;
		OnShoot(ShotTransform);
	}

	netSig_Shooted(Location);
}

void AWC_Railgun::SetMoving(bool bNewMoving)
{
	if (isMoving == bNewMoving)
	{
		return;
	}

	isMoving = bNewMoving;
	if (isMoving)
	{
		OnStartMoving();
	}
	else
	{
		OnStopMoving();
	}
}

void AWC_Railgun::ResetStateTimer()
{
	StateTime = 0.f;
	if (!GetWorld())
	{
		StateStartServerTime = 0.f;
		return;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	StateStartServerTime = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

void AWC_Railgun::UpdateStateTimeFromServerClock()
{
	if (!GetWorld())
	{
		return;
	}

	const float Duration = GetCurrentStateDuration();
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		StateTime = 0.f;
		return;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const float ServerTime = GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	StateTime = FMath::Clamp((ServerTime - StateStartServerTime) / Duration, 0.f, 1.f);
}

float AWC_Railgun::GetCurrentStateDuration() const
{
	switch (animationState)
	{
	case ERailgunState::IDLE:
		return IdleSeconds;
	case ERailgunState::LOADING:
		return LoadingSeconds;
	case ERailgunState::RESETTING:
		return ResettingSeconds;
	default:
		return 0.f;
	}
}

#pragma optimize("", on)
