#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "InfiniteZoopSubsystem.generated.h"


UCLASS()
class AInfiniteZoopSubsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	AInfiniteZoopSubsystem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
	int currentZoopAmount = 12;

	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
		int xAmount = -1;
	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
		int yAmount = -1;
	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
		int zAmount = -1;
	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
		bool isFoundation = true;
	UPROPERTY(BlueprintReadWrite, replicated, Category = "Zoop Subsystem")
		bool needsUpdate = true;

	void SetPublicZoopAmount(int x, int y, int z, bool foundation, bool verticalZoop);
	//UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
	//bool zoopCorners = false;

	//UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
	//int tempZoopAmount = 0;
};

