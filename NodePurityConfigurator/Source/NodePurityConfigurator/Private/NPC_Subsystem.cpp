


#include "NPC_Subsystem.h"



void ANPC_Subsystem::BeginPlay()
{
	IronPurities.Add(EResourcePurity::RP_Inpure, 39);
	IronPurities.Add(EResourcePurity::RP_Normal, 42);
	IronPurities.Add(EResourcePurity::RP_Pure, 46);
	
	Super::BeginPlay();
}
