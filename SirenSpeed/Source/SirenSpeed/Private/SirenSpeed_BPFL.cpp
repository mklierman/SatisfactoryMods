


#include "SirenSpeed_BPFL.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void USirenSpeed_BPFL::SetEngineSpeed(UFGWheeledVehicleMovementComponent* movementComp, float newRPM, float newTorque)
{
	movementComp->EngineSetup.MaxRPM = newRPM;
	movementComp->EngineSetup.MaxTorque = newTorque;

	movementComp->RecreatePhysicsState();
}
