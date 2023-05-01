#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"

#include "InfiniteZoop_ClientSubsystem.generated.h"

UCLASS()
class INFINITEZOOP_API AInfiniteZoop_ClientSubsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	AInfiniteZoop_ClientSubsystem();

	UFUNCTION(BlueprintPure)
		static AInfiniteZoop_ClientSubsystem* Get(UObject* worldContext);

	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		bool zoopCorners = false;

	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		int tempZoopAmount = 0;


	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		int xAmount = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		int yAmount = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		int zAmount = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		bool isFoundation = true;
	UPROPERTY(BlueprintReadWrite, Category = "Zoop Subsystem")
		bool needsUpdate = true;

	void SetPublicZoopAmount(int x, int y, int z, bool foundation, bool verticalZoop);
};
