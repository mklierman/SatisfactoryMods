// ILikeBanas

#include "CDO_BPLib.h"
#include "AI/FGAISystem.h"
#include "FGRailroadVehicleMovementComponent.h"


float UCDO_BPLib::GetGravitationalForce(UFGRailroadVehicleMovementComponent* actor)
{
	return actor->GetGravitationalForce();
}

float UCDO_BPLib::GetResistiveForce(UFGRailroadVehicleMovementComponent* actor)
{
	return actor->GetResistiveForce();
}

float UCDO_BPLib::GetGradientForce(UFGRailroadVehicleMovementComponent* actor)
{
	return actor->GetGradientForce();
}
