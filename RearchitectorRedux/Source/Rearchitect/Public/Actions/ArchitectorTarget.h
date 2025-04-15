#pragma once
#include "AbstractInstanceManager.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableConveyorBase.h"

#include "ArchitectorTarget.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorToolTarget
{
	GENERATED_BODY()

public:

	UPROPERTY() AActor* Target = nullptr;
	UPROPERTY() UPrimitiveComponent* HitComponent = nullptr;
	UPROPERTY() bool IsAbstract = false;

	FArchitectorToolTarget(){}
	FArchitectorToolTarget(const FHitResult& HitResult)
	{
		HitResultActor = HitResult.GetActor();
		HitComponent = HitResult.GetComponent();
		HitItem = HitResult.Item;
		
		if(auto AbstractInstance = Cast<AAbstractInstanceManager>(HitResultActor))
		{
			FInstanceHandle InstanceHandle;
			AbstractInstance->ResolveHit(HitResult, InstanceHandle);
			Target = InstanceHandle.GetOwner<AActor>();
			IsAbstract = true;
		}
		else Target = Cast<AFGBuildable>(HitResult.GetActor());
	}

	FInstanceHandle GenerateInstanceHandle() const
	{
		FHitResult Hit = FHitResult(HitResultActor, HitComponent, FVector::ZeroVector, FVector::ZeroVector);
		Hit.Item = HitItem;

		FInstanceHandle Out;
		auto AbstractInstance = Cast<AAbstractInstanceManager>(HitResultActor);
		AbstractInstance->ResolveHit(Hit, Out);

		return Out;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FArchitectorToolTarget& Thing)
	{
		uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FArchitectorToolTarget));
		return Hash;
	}

	bool Equals(const FArchitectorToolTarget& Other) const { return *this == Other; }
	bool operator==(const FArchitectorToolTarget& Other) const { return Target == Other.Target; }
	bool operator==(AActor* Actor) const { return Target == Actor; }
	static bool IsValidTarget(const FHitResult& HitResult)
	{
		auto Actor = HitResult.GetActor();
		if(!Actor) return false;
		
		bool IsValidClass = (Actor->IsA<AFGBuildable>() && !Actor->HasAnyFlags(RF_WasLoaded)) && !HitResult.GetActor()->IsA<AAbstractInstanceManager>();
		if (IsValidClass)
		{
			auto Belt = Cast<AFGBuildableConveyorBase>(Actor);
			if (Belt)
			{
				IsValidClass = false;
			}
		}
		return IsValidClass;
	}

private:

	UPROPERTY() AActor* HitResultActor = nullptr;
	UPROPERTY() int32 HitItem = 0;
	
};
