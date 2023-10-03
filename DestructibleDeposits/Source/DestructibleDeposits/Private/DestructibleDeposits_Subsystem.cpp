#include "DestructibleDeposits_Subsystem.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceDeposit.h"

ADestructibleDeposits_Subsystem::ADestructibleDeposits_Subsystem()
{
//#if !WITH_EDITOR
//	RadialDamage.BindUFunction(this, "OnTakeRadialDamage");
//	AFGResourceDeposit* rdCDO = GetMutableDefault<AFGResourceDeposit>();
//	SUBSCRIBE_METHOD_VIRTUAL(AFGResourceDeposit::BeginPlay, rdCDO, [=](auto& scope, AFGResourceDeposit* self)
//		{
//			self->OnTakeRadialDamage.Add(RadialDamage);
//		});
//#endif
}

void ADestructibleDeposits_Subsystem::OnTakeRadialDamage(AActor* actor)
{
	actor->Destroy();
}
