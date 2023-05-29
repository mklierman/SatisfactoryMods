#include "NoRefundsModule.h"
#include "Equipment/FGBuildGunDismantle.h"
#include "Patching/NativeHookManager.h"

//#pragma optimize("", off)
void FNoRefundsModule::StartupModule() {

	TArray< FInventoryStack > refund = TArray< FInventoryStack >();
	SUBSCRIBE_METHOD(UFGBuildGunStateDismantle::GetDismantleRefund, [=](auto& scope, const UFGBuildGunStateDismantle* self)
		{
			TArray< FInventoryStack > refund = scope(self);
			refund = AllowDismantleRefund(refund);
			scope.Override(refund);
		});
#if !WITH_EDITOR
#endif
}

TArray<FInventoryStack> FNoRefundsModule::AllowDismantleRefund(TArray<FInventoryStack> refund)
{
	TArray<FInventoryStack> newRefund;
	if (refund.Num() > 0)
	{
		for (auto stack : refund)
		{
			auto itemClass = stack.Item.GetItemClass()->GetName();
			if (itemClass.Equals("Desc_HUBParts_C") || itemClass.Equals("Desc_CrystalShard_C"))
			{
				newRefund.Add(stack);
			}
		}
	}
	if (newRefund.Num() > 0)
	{
		return newRefund;
	}
	refund.Empty();
	return refund;
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoRefundsModule, NoRefunds);