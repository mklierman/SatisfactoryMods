

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "Resources/FGResourceNode.h"
#include <FGGameState.h>
#include "ParadiseIsland_Subsystem.generated.h"

/**
 * 
 */
UCLASS()
class PARADISEISLAND_API AParadiseIsland_Subsystem : public AModSubsystem, public IFGSaveInterface
{
	GENERATED_BODY()
	
public:
	AParadiseIsland_Subsystem();

	virtual bool ShouldSave_Implementation() const override;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TMap< AFGResourceNode*, TEnumAsByte<EResourcePurity>> NodePuritySettings;
	
	
};
