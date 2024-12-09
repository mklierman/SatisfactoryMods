#include "NoRefundsModule.h"
#include "Equipment/FGBuildGunDismantle.h"
#include "Patching/NativeHookManager.h"
#include "Equipment/FGChainsaw.h"
#include "FGFoliageLibrary.h"
#include "FGCharacterPlayer.h"
#include "NR_ConfigStruct.h"

//#pragma optimize("", off)
void FNoRefundsModule::StartupModule() {
	TArray< FInventoryStack > refund = TArray< FInventoryStack >();
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGBuildGunStateDismantle::GetPeekDismantleRefund, [=](auto& scope, const UFGBuildGunStateDismantle* self)
		{
			TArray< FInventoryStack > refund = scope(self);
			refund = AllowDismantleRefund(refund);
			scope.Override(refund);
		});

	SUBSCRIBE_METHOD(UFGBuildGunStateDismantle::Internal_DismantleActor, [=](auto& scope, UFGBuildGunStateDismantle* self, class AActor* actorToDismantle, TArray< AActor* >& out_couldNotDismantle, TArray<FInventoryStack>& out_dismantleRefunds, bool bNoBuildCostEnabled)
		{
			scope(self, actorToDismantle, out_couldNotDismantle, out_dismantleRefunds, bNoBuildCostEnabled);
			TArray<FInventoryStack> newRefund;
			newRefund = AllowDismantleRefund(out_dismantleRefunds);
			out_dismantleRefunds = newRefund;
			scope.Cancel();
		});

	SUBSCRIBE_METHOD_AFTER(UFGFoliageLibrary::CheckInventorySpaceAndGetStacks, [=](auto returnValue, AFGCharacterPlayer* character, UHierarchicalInstancedStaticMeshComponent* meshComponent, TArrayView< uint32 > randomSeeds, TArray<struct FInventoryStack>& out_inventoryStacks)
		{
			auto config = FNR_ConfigStruct::GetActiveConfig(character->GetWorld());
			bool shouldPreventFoliage = config.Foliage;
			if (shouldPreventFoliage)
			{
				out_inventoryStacks.Empty();
			}
		});
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
			if (itemClass.Equals("Desc_HUBParts_C") || itemClass.Equals("Desc_CrystalShard_C") || itemClass.Equals("Desc_WAT1_C") || itemClass.Equals("Desc_WAT2_C"))
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