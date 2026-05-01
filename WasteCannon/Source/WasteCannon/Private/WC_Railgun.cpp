#include "WC_Railgun.h"
#include "TimerManager.h"
#include <Logging/StructuredLog.h>
#include <Async.h>

DEFINE_LOG_CATEGORY(WasteCannon_Log);
#pragma optimize("", off)

AWC_Railgun::AWC_Railgun()
{
	this->FuelInventoryComponent = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("FuelInventory"));
	this->StorageInventoryComponent = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("StorageInventory"));

	this->Loaded = false;
	this->MinTilt = 45;
	this->MaxTilt = 78;
	this->RotationSpeed = 10;
	this->animationState = ERailgunState::IDLE;
	this->AutoShootWasteThreshold = 200;
	this->AutoShootCheckIntervalSeconds = 0.25f;
	this->LastShotWasteCount = 0;
	this->bUseAmmoWhitelist = true;
}

void AWC_Railgun::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetWorld())
	{
		if (FuelInventoryComponent)
		{
			FuelInventoryComponent->SetReplicationRelevancyOwner(this);
			FuelInventoryComponent->SetLocked(false);
			FuelInventoryComponent->Resize(1);
			FuelInventoryComponent->AddArbitrarySlotSize(0, 20);
		}
		if (StorageInventoryComponent)
		{
			StorageInventoryComponent->SetReplicationRelevancyOwner(this);
			StorageInventoryComponent->SetLocked(false);
			StorageInventoryComponent->Resize(1);
			StorageInventoryComponent->AddArbitrarySlotSize(0, 20);
		}

		InputConnections.Empty();
		FOR_EACH_FACTORY_INLINE_COMPONENTS(Connection)
		{
			if (Connection && Connection->GetDirection() == EFactoryConnectionDirection::FCD_INPUT)
			{
				InputConnections.Add(Connection);
			}
		}

		if (InputConnections.Num() > 0 && InputConnections[0] && FuelInventoryComponent)
		{
			InputConnections[0]->SetInventory(FuelInventoryComponent);
			InputConnections[0]->SetInventoryAccessIndex(0);
		}
		if (InputConnections.Num() > 1 && InputConnections[1] && StorageInventoryComponent)
		{
			InputConnections[1]->SetInventory(StorageInventoryComponent);
			InputConnections[1]->SetInventoryAccessIndex(0);
		}

		GetWorld()->GetTimerManager().SetTimer(
			AutoShootTimerHandle,
			this,
			&AWC_Railgun::EvaluateAutoShoot,
			FMath::Max(0.05f, AutoShootCheckIntervalSeconds),
			true
		);
	}
}

void AWC_Railgun::Factory_Tick(float dt)
{
	Super::Factory_Tick(dt);

	if (HasAuthority())
	{
		for (UFGFactoryConnectionComponent* Connection : InputConnections)
		{
			if (!Connection || !Connection->IsConnected())
			{
				continue;
			}

			UFGInventoryComponent* TargetInventory = ResolveInventoryForConnection(Connection);
			if (!TargetInventory || TargetInventory->IsLocked())
			{
				continue;
			}

			TArray<FInventoryItem> PeekItems;
			if (!Connection->Factory_PeekOutput(PeekItems, nullptr))
			{
				continue;
			}

			for (const FInventoryItem& PeekItem : PeekItems)
			{
				if (!PeekItem.IsValid())
				{
					continue;
				}
				if (!TargetInventory->HasEnoughSpaceForItem(PeekItem))
				{
					continue;
				}

				FInventoryItem GrabbedItem;
				float OffsetBeyond = 100.0f;
				if (!Connection->Factory_GrabOutput(GrabbedItem, OffsetBeyond, nullptr))
				{
					continue;
				}

				TargetInventory->AddItem(GrabbedItem);
				break;
			}
		}
	}

	if (this->isMoving)
	{
		this->towerMovement.current = FMath::FInterpConstantTo(this->towerMovement.current, this->towerMovement.target, dt, this->RotationSpeed);
		this->barrelMovement.current = FMath::FInterpConstantTo(this->barrelMovement.current, this->barrelMovement.target, dt, this->RotationSpeed);

		if (FMath::Abs(this->towerMovement.current - this->towerMovement.target) < 0.01f &&
			FMath::Abs(this->barrelMovement.current - this->barrelMovement.target) < 0.01f)
		{
			this->isMoving = false;
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->MovementComplete();
			});
		}
	}
}

bool AWC_Railgun::ShouldSave_Implementation() const
{
	return true;
}

void AWC_Railgun::UpdateAimDirection(const FVector& position, const FVector2D& direction)
{
	//UE_LOGFMT(WasteCannon_Log, Display, "UpdateAimDirection");
	this->TargetPosition = position;
	this->AimDirection = direction;
	this->towerMovement.start = this->towerMovement.current;
	this->barrelMovement.start = this->barrelMovement.current;
	this->towerMovement.target = direction.X;
	this->barrelMovement.target = direction.Y;

	if (this->towerMovement.start + 180 < this->towerMovement.target) this->towerMovement.start += 360;
	if (this->towerMovement.start - 180 > this->towerMovement.target) this->towerMovement.start -= 360;
	this->towerMovement.current = this->towerMovement.start;

	this->isMoving = true;
	OnAimChanged();
	OnStartMoving();
}

void AWC_Railgun::SetAimDirectionDirect(const FVector2D & direction)
{
	this->towerMovement.start = direction.X;
	this->barrelMovement.start = direction.Y;
	this->towerMovement.target = direction.X;
	this->barrelMovement.target = direction.Y;
	this->towerMovement.current = direction.X;
	this->barrelMovement.current = direction.Y;
}

void AWC_Railgun::MovementComplete()
{
	//UE_LOGFMT(WasteCannon_Log, Display, "MovementComplete");
	OnStopMoving();
	if (HasAuthority())
	{
		SetAimDirectionDirect(FVector2D(this->towerMovement.target, this->barrelMovement.target));
		if (this->animationState == ERailgunState::AIMING && Loaded)
		{
			ReadyForShoot();
			if (this->animationState == ERailgunState::AIMING)
			{
				Shoot(GetActorTransform());
			}
		}
		else if (this->animationState == ERailgunState::RESETTING)
		{
			// Reset completes the shot cycle; next cycle will begin from loading.
			this->animationState = ERailgunState::IDLE;
			netSig_Finished();
			//UE_LOGFMT(WasteCannon_Log, Display, "transition to idle");
		}
	}
}

FVector2D AWC_Railgun::GetAimDirection()
{
	return FVector2D(this->towerMovement.target, this->barrelMovement.target);
}

FRotator AWC_Railgun::GetAimRotation()
{
	return FRotator(this->towerMovement.target, this->barrelMovement.target, 0);
}

void AWC_Railgun::ResetTower()
{
	AimToDirection(FVector2D(this->towerMovement.target, 0), ERailgunState::RESETTING);
}

void AWC_Railgun::AnimationComplete()
{
	if (this->animationState == ERailgunState::SHOOTING)
	{
		ResetTower();
	}
	else if (this->animationState == ERailgunState::LOADING)
	{
		AimToDirection(AimDirection, ERailgunState::AIMING);
	}
}

void AWC_Railgun::Shoot(const FTransform& position)
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetWasteItemCount() <= 0)
	{
		return;
	}
	if (!CanShootWithCurrentAmmo())
	{
		return;
	}
	if (!ConsumeOneFuelItem())
	{
		return;
	}

	LastShotWasteCount = GetWasteItemCount();
	RemoveAllWasteItems();

	this->animationState = ERailgunState::SHOOTING;

	//UE_LOGFMT(WasteCannon_Log, Display, "SHOOTING. State: {0}", UEnum::GetValueAsString(animationState));
	this->Loaded = false;
	OnShoot(position);
	OnShotTriggered(position);
	netSig_Shooted(position.GetLocation());
}

void AWC_Railgun::AimToDirection(const FVector2D & direction, ERailgunState newState)
{
	//UE_LOGFMT(WasteCannon_Log, Display, "AimToDirection. State: {0}", UEnum::GetValueAsString(animationState));
	this->animationState = newState;
	UpdateAimDirection(FVector::ZeroVector, direction);
}

int32 AWC_Railgun::GetWasteItemCount() const
{
	if (!StorageInventoryComponent)
	{
		return 0;
	}
	return StorageInventoryComponent->GetNumItems(nullptr);
}

bool AWC_Railgun::IsWasteInventoryFull() const
{
	if (!StorageInventoryComponent)
	{
		return false;
	}
	return StorageInventoryComponent->FindEmptyIndex() < 0;
}

bool AWC_Railgun::ConsumeOneFuelItem()
{
	if (!FuelInventoryComponent)
	{
		return false;
	}

	bool bHasValidWhitelistEntry = false;
	for (const TSubclassOf<UFGItemDescriptor>& AllowedDescriptor : AllowedAmmoDescriptors)
	{
		if (!AllowedDescriptor)
		{
			continue;
		}
		bHasValidWhitelistEntry = true;

		if (FuelInventoryComponent->HasItems(AllowedDescriptor, 1))
		{
			FuelInventoryComponent->Remove(AllowedDescriptor, 1);
			return true;
		}
	}

	const int32 FuelIndex = FuelInventoryComponent->GetFirstIndexWithItem();
	if (FuelIndex < 0)
	{
		return false;
	}

	FInventoryStack FuelStack;
	if (!FuelInventoryComponent->GetStackFromIndex(FuelIndex, FuelStack) || !FuelStack.HasItems())
	{
		return false;
	}

	FuelInventoryComponent->Remove(FuelStack.Item.GetItemClass(), 1);
	return true;
}

bool AWC_Railgun::CanShootWithCurrentAmmo() const
{
	if (!FuelInventoryComponent)
	{
		return false;
	}

	bool bHasValidWhitelistEntry = false;
	for (const TSubclassOf<UFGItemDescriptor>& AllowedDescriptor : AllowedAmmoDescriptors)
	{
		if (!AllowedDescriptor)
		{
			continue;
		}
		bHasValidWhitelistEntry = true;

		if (FuelInventoryComponent->GetNumItems(AllowedDescriptor) > 0)
		{
			return true;
		}
	}

	return FuelInventoryComponent->GetFirstIndexWithItem() >= 0;
}

void AWC_Railgun::RemoveAllWasteItems()
{
	if (!StorageInventoryComponent)
	{
		return;
	}
	StorageInventoryComponent->Empty();
}

void AWC_Railgun::EvaluateAutoShoot()
{
	//UE_LOGFMT(WasteCannon_Log, Display, "EvaluateAutoShoot. State: {0}", UEnum::GetValueAsString(animationState));
	TryAutoShoot();
}

void AWC_Railgun::FinishShotCycleFallback()
{
	if (!HasAuthority())
	{
		return;
	}

	if (this->animationState == ERailgunState::SHOOTING)
	{
		ResetTower();
	}
}

UFGInventoryComponent* AWC_Railgun::ResolveInventoryForConnection(UFGFactoryConnectionComponent* Connection) const
{
	if (!Connection)
	{
		return nullptr;
	}

	const FString ConnectionName = Connection->GetName();
	if (ConnectionName.Contains(TEXT("Fuel"), ESearchCase::IgnoreCase))
	{
		return FuelInventoryComponent;
	}
	if (ConnectionName.Contains(TEXT("Storage"), ESearchCase::IgnoreCase) ||
		ConnectionName.Contains(TEXT("Waste"), ESearchCase::IgnoreCase))
	{
		return StorageInventoryComponent;
	}

	const int32 FoundIndex = InputConnections.Find(Connection);
	if (FoundIndex == 0)
	{
		return FuelInventoryComponent;
	}
	if (FoundIndex == 1)
	{
		return StorageInventoryComponent;
	}

	return StorageInventoryComponent;
}

bool AWC_Railgun::TryAutoShoot()
{
	//UE_LOGFMT(WasteCannon_Log, Display, "TryAutoShoot. State: {0}", UEnum::GetValueAsString(animationState));
	if (!HasAuthority())
	{
		return false;
	}

	if (animationState != ERailgunState::IDLE && animationState != ERailgunState::GUARDING)
	{
		return false;
	}

	const int32 WasteCount = GetWasteItemCount();
	const bool bWasteThresholdReached = AutoShootWasteThreshold > 0 && WasteCount >= AutoShootWasteThreshold;
	const bool bWasteInventoryFull = IsWasteInventoryFull();
	if (!bWasteThresholdReached && !bWasteInventoryFull)
	{
		return false;
	}

	if (WasteCount <= 0)
	{
		return false;
	}

	if (!CanShootWithCurrentAmmo())
	{
		return false;
	}

	// Ensure barrel is back to load angle before entering LOADING.
	if (FMath::Abs(this->barrelMovement.current) > 0.01f)
	{
		ResetTower();
		return true;
	}

	Loaded = true;

	this->animationState = ERailgunState::LOADING;
	ReadyForShoot();

	return true;
}

#pragma optimize("", on)