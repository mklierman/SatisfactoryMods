#include "SOBBuildableStorage.h"

void ASOBBuildableStorage::Dismantle_Implementation()
{
	if (inputConnection && outputConnection)
	{
		if (inputConnection->GetConnection() && outputConnection->GetConnection())
		{
			auto inputLoc = inputConnection->GetConnection()->GetConnectorLocation();
			auto outputLoc = outputConnection->GetConnection()->GetConnectorLocation();
			if ((inputLoc - outputLoc).IsNearlyZero(0.01))
			{
				auto belt1Conn = inputConnection->GetConnection();
				auto belt2Conn = outputConnection->GetConnection();
				if (belt1Conn && belt2Conn)
				{
					auto belt1 = Cast< AFGBuildableConveyorBelt>(belt1Conn->GetOuterBuildable());
					auto belt2 = Cast< AFGBuildableConveyorBelt>(belt2Conn->GetOuterBuildable());
					if (belt1 && belt2)
					{
						if (belt1->GetSpeed() == belt2->GetSpeed())
						{
							auto belt1CustomizationData = belt1->Execute_GetCustomizationData(belt1);
							TArray< AFGBuildableConveyorBelt*> belts;
							belts.Add(belt1);
							belts.Add(belt2);
							belt1Conn->ClearConnection();
							belt2Conn->ClearConnection();
							belt1Conn->SetConnection(belt2Conn);
							auto newBelt = AFGBuildableConveyorBelt::Merge(belts);
							if (newBelt && belt1CustomizationData.IsInitialized())
							{
								newBelt->Execute_SetCustomizationData(newBelt, belt1CustomizationData);
							}
						}
						else
						{
							belt1Conn->ClearConnection();
							belt2Conn->ClearConnection();
							belt1Conn->SetConnection(belt2Conn);
						}
					}
				}
			}
		}
	}
	Super::Dismantle_Implementation();
}