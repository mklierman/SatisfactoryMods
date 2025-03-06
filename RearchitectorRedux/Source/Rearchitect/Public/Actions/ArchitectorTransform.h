#pragma once

#include "CoreMinimal.h"
#include "ArchitectorTarget.h"
#include "UObject/Object.h"
#include "ArchitectorTransform.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FPositionModifier
{
	GENERATED_BODY()

public:

	/*
	 * Whether this action is used in this transform data.
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool IsUsed = false;

	/*
	 * Should the object be SET to the specified position instead of MOVED by the specified amount?
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool UseAsSetAction = false;

	UPROPERTY(BlueprintReadWrite, SaveGame) FVector Value = FVector::ZeroVector;

	FPositionModifier(){}
	FPositionModifier(const FVector& Value, bool UseAsSet = false) : IsUsed(true), UseAsSetAction(UseAsSet), Value(Value){}
	
};

USTRUCT(Blueprintable, BlueprintType)
struct FRotationModifier
{
	GENERATED_BODY()

public:

	/*
	 * Whether this action is used in this transform data.
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool IsUsed = false;

	/*
	 * Should the object be SET to the specified rotation instead of ROTATED by the specified amount?
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool UseAsSetAction = false;

	UPROPERTY(BlueprintReadWrite, SaveGame) FQuat Value = FQuat::Identity;

	FRotationModifier(){}
	FRotationModifier(const FQuat& Value, bool UseAsSet = false) : IsUsed(true), UseAsSetAction(UseAsSet), Value(Value){}
};

USTRUCT(Blueprintable, BlueprintType)
struct FScaleModifier
{
	GENERATED_BODY()

public:

	/*
	 * Whether this action is used in this transform data.
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool IsUsed = false;

	/*
	 * Should the object be SET to the specified scale instead of SCALED by the specified amount?
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame) bool UseAsSetAction = false;

	UPROPERTY(BlueprintReadWrite, SaveGame) FVector Value = FVector::ZeroVector;

	FScaleModifier(){}
	FScaleModifier(const FVector& Value, bool UseAsSet = false) : IsUsed(true), UseAsSetAction(UseAsSet), Value(Value){}
	
};

USTRUCT(Blueprintable, BlueprintType)
struct FActorTransformModifyData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame) FPositionModifier Move = FPositionModifier();
	UPROPERTY(BlueprintReadWrite, SaveGame) FRotationModifier Rotate = FRotationModifier();
	UPROPERTY(BlueprintReadWrite, SaveGame) FScaleModifier Scale = FScaleModifier();
};

USTRUCT(Blueprintable, BlueprintType)
struct FTargetModifyData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, SaveGame) FArchitectorToolTarget Target = FArchitectorToolTarget();
	UPROPERTY(BlueprintReadWrite, SaveGame) FActorTransformModifyData TransformData = FActorTransformModifyData();
	
	FTargetModifyData(){}
	FTargetModifyData(const FArchitectorToolTarget& Target, const FActorTransformModifyData& TransformData) : Target(Target), TransformData(TransformData){}
};

USTRUCT(Blueprintable, BlueprintType)
struct FTargetTransformData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FArchitectorToolTarget Target;

	UPROPERTY(BlueprintReadWrite)
	FTransform ActorTransform = FTransform::Identity;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FTransform TestTransform;

	UPROPERTY(SaveGame, BlueprintReadWrite)
	FTransform AbstractTransform;
};
