#include "InfiniteNudgeModule.h"

#pragma optimize("", off)
void FInfiniteNudgeModule::StartupModule() {
#if !WITH_EDITOR
	AFGHologram* bh = GetMutableDefault<AFGHologram>();
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGHologram::BeginPlay, bh, [=](AFGHologram* self)
		{
			self->mCanNudgeHologram = true;
			self->mCanLockHologram = true;
			self->mMaxNudgeDistance = 50000.0;
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
			FVector newVector = FVector(0.0, 0.0, 0.0);
			auto contr = Cast<APlayerController>(self->GetConstructionInstigator()->GetController());
			if (contr && contr->IsInputKeyDown(EKeys::LeftShift) && contr->IsInputKeyDown(EKeys::Up))
			{
				newVector.Z = 100;
				//FVector newV = NudgeTowardsWorldDirection(self, Direction);
				//FVector nV = FVector(0.0, 0.0, 100.0);
				if (contr->IsInputKeyDown(EKeys::LeftControl))
				{
					newVector.Z = newVector.Z / 2;
				}
				else if (contr->IsInputKeyDown(EKeys::LeftAlt))
				{
					newVector.Z = newVector.Z / 10;
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
					newVector.Z = newVector.Z / 2;
				}
				else if (contr->IsInputKeyDown(EKeys::LeftAlt))
				{
					newVector.Z = newVector.Z / 10;
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

	//SUBSCRIBE_METHOD_VIRTUAL(AFGHologram::Scroll, bh, [=](auto& scope, AFGHologram* self, int32 delta)
	//	{
	//		if (self->IsHologramLocked())
	//		{
	//			self->ScrollRotate(delta, 1);
	//		}
	//	});
#endif
}

FVector FInfiniteNudgeModule::NudgeTowardsWorldDirection(AFGHologram* self, const FVector& Direction)
{
	return Direction;
}
#pragma optimize("", on)


IMPLEMENT_GAME_MODULE(FInfiniteNudgeModule, InfiniteNudge);