#include "InfiniteNudgeModule.h"
#include "InfiniteNudge_ConfigurationStruct.h"

//#pragma optimize("", off)
void FInfiniteNudgeModule::StartupModule() {

	AFGHologram* bh = GetMutableDefault<AFGHologram>();
	//ENudgeFailReason AFGHologram::NudgeHologram(const FVector& NudgeInput, const FHitResult& HitResult){ return ENudgeFailReason(); }
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::NudgeHologram, bh, [=](auto scope, const AFGHologram* self, const FVector& NudgeInput, const FHitResult& HitResult)
		{
			auto hg = const_cast<AFGHologram*>(self);
			auto contr = Cast<APlayerController>(hg->GetConstructionInstigator()->GetController());
			if (contr && contr->IsInputKeyDown(EKeys::LeftControl))
			{
				hg->mDefaultNudgeDistance = LeftCtrlNudgeAmount;
			}
			else if (contr && contr->IsInputKeyDown(EKeys::LeftAlt))
			{
				hg->mDefaultNudgeDistance = LeftAltNudgeAmount;
			}
			else
			{
				hg->mDefaultNudgeDistance = 100.0;
			}
		});

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHologram::BeginPlay, bh, [=](AFGHologram* self)
		{
			auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig();
			LeftCtrlNudgeAmount = (float)config.LeftCtrlNudgeAmount;
			LeftAltNudgeAmount = (float)config.LeftAltNudgeAmount;

			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
			self->mMaxNudgeDistance = 5000000.0;
		});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::CanNudgeHologram, bh, [=](auto& scope, const AFGHologram* self)
		{
			scope.Override(true);
		});

	//const FVector& GetNudgeOffset() const { return mHologramNudgeOffset; }
	SUBSCRIBE_METHOD(AFGHologram::LockHologramPosition, [=](auto scope, AFGHologram* self, bool lock)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
		});
		//const FVector& GetNudgeOffset() const { return mHologramNudgeOffset; }
	SUBSCRIBE_METHOD(AFGHologram::GetMaxNudgeDistance, [=](auto scope, const AFGHologram* self)
		{
			FVector ret = FVector(50000, 50000, 50000);
			scope.Override(ret);
			//auto hg = Cast<AFGHologram>(self);
			//hg->mHologramNudgeOffset.X = 0;
			//hg->mHologramNudgeOffset.Y = 0;
			//hg->mHologramNudgeOffset.Z = 0;
		});
	//	virtual ENudgeFailReason NudgeTowardsWorldDirection(const FVector & Direction);
	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::AddNudgeOffset, bh, [=](auto& scope, AFGHologram* self, const FVector& Direction)
		{
			auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig();
			FVector newVector = FVector(0.0, 0.0, 0.0);
			auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
			if (contr && contr->IsInputKeyDown(EKeys::LeftShift) && contr->IsInputKeyDown(EKeys::Up))
			{
				newVector.Z = 100;
				//FVector newV = NudgeTowardsWorldDirection(self, Direction);
				//FVector nV = FVector(0.0, 0.0, 100.0);
				if (contr->IsInputKeyDown(EKeys::LeftControl))
				{
					newVector.Z = LeftCtrlNudgeAmount;
				}
				else if (contr->IsInputKeyDown(EKeys::LeftAlt))
				{
					newVector.Z = LeftAltNudgeAmount;
				}
				ENudgeFailReason fr = scope(self, newVector);
				scope.Override(fr);
				//ENudgeFailReason fr = scope(self, nV);
				//scope.Override(ENudgeFailReason::NFR_Success);
			}
			else if (contr && contr->IsInputKeyDown(EKeys::LeftShift) && contr->IsInputKeyDown(EKeys::Down))
			{
				newVector.Z = -100;
				//FVector neV = FVector(0.0, 0.0, -100.0);
				if (contr->IsInputKeyDown(EKeys::LeftControl))
				{
					newVector.Z = LeftCtrlNudgeAmount * -1;
				}
				else if (contr->IsInputKeyDown(EKeys::LeftAlt))
				{
					newVector.Z = LeftAltNudgeAmount * -1;
				}
				ENudgeFailReason fr = scope(self, newVector);
				scope.Override(fr);
				//	ENudgeFailReason fr = scope(self, neV);
				//	scope.Override(ENudgeFailReason::NFR_Success);
			}
			//else
			//{
			//	newVector.X = Direction.X;
			//	newVector.Y = Direction.Y;
			//	if (contr && contr->IsInputKeyDown(EKeys::LeftControl))
			//	{
			//		newVector.X = Direction.X / 2;
			//		newVector.Y = Direction.Y / 2;
			//		ENudgeFailReason fr = scope(self, newVector);
			//		scope.Override(fr);
			//	}
			//	else if (contr && contr->IsInputKeyDown(EKeys::LeftAlt))
			//	{
			//		newVector.X = Direction.X / 10;
			//		newVector.Y = Direction.Y / 10;
			//		ENudgeFailReason fr = scope(self, newVector);
			//		scope.Override(fr);
			//	}
			//}
			//self->mDefaultNudgeDistance = savedNudgeDistance;
		});

	//virtual ENudgeFailReason NudgeHologram( const FVector& NudgeInput, const FHitResult& HitResult );
	// 
	///Hooking this causes weird bugs. Need to find another spot?
	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::SetNudgeOffset, bh, [=](auto& scope, AFGHologram* self, const FVector& NewNudgeOffset)
	//	{
	//		savedNudgeDistance = self->mDefaultNudgeDistance;
	//		auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	//		if (contr && contr->IsInputKeyDown(EKeys::LeftControl))
	//		{
	//			self->mDefaultNudgeDistance = 20.0;
	//		}
	//		else if (contr && contr->IsInputKeyDown(EKeys::LeftAlt))
	//		{
	//			self->mDefaultNudgeDistance = 50.0;
	//		}
	//		else
	//		{
	//			self->mDefaultNudgeDistance = 100.0;
	//		}
	//	});

	SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, bh, [=](auto& scope, AFGHologram* self, int32 delta)
		{
			if (self->IsHologramLocked())
			{
				RotateLockedHologram(self, delta);
			}
		});
#if !WITH_EDITOR
#endif
}

FVector FInfiniteNudgeModule::NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction)
{
	return Direction;
}

void FInfiniteNudgeModule::RotateLockedHologram(AFGHologram* self, int32 delta)
{
	auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
	//auto newRotation = self->GetActorRotation();
	auto newRotation = FRotator(0,0,0);
	int rotationAmount = 15 * delta;

	// Set Fine Rotation
	if (contr && contr->IsInputKeyDown(EKeys::LeftControl) || contr->IsInputKeyDown(EKeys::RightControl))
	{
		rotationAmount = 1 * delta;
	}

	if (contr->IsInputKeyDown(EKeys::LeftShift))
	{
		//Rotate Pitch
		newRotation.Pitch = newRotation.Pitch + rotationAmount;
	}
	else if (contr->IsInputKeyDown(EKeys::RightShift))
	{
		//Rotate Roll
		newRotation.Roll = newRotation.Roll + rotationAmount;
	}
	else
	{
		//Rotate Yaw
		newRotation.Yaw = newRotation.Yaw + rotationAmount;
	}
	self->AddActorLocalRotation(newRotation);
}
//#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FInfiniteNudgeModule, InfiniteNudge);