#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FGIconLibrary.h"

#include "UnrestrictedIconsSubsystem.generated.h"

UCLASS()
class UNRESTRICTEDICONS_API AUnrestrictedIconsSubsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
public:

	virtual bool ShouldSave_Implementation() const override;

		void AddEnumType();

	UPROPERTY(BlueprintReadWrite)
		int EnumIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Icon Data Settings")
		TArray< FIconData > mModIconData;
};
