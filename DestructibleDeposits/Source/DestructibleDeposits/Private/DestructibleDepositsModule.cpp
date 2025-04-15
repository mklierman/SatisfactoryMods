#include "DestructibleDepositsModule.h"
#include "Patching/NativeHookManager.h"
#include "Resources/FGResourceDeposit.h"

void FDestructibleDepositsModule::StartupModule() {
	//TScriptDelegate <FWeakObjectPtr> RadialDamage;
	//RadialDamage.BindUFunction(this, "OnTakeRadialDamage");
	////virtual void BeginPlay() override;
	//AFGResourceDeposit* rdCDO = GetMutableDefault<AFGResourceDeposit>();
	//SUBSCRIBE_METHOD_VIRTUAL(AFGResourceDeposit::BeginPlay, rdCDO, [this](auto& scope, AFGResourceDeposit* self)
	//	{
	//	});

}


IMPLEMENT_GAME_MODULE(FDestructibleDepositsModule, DestructibleDeposits);