#include "GAFB_BPFL.h"
#include "FGFactoryConnectionComponent.h"
#include "FGConveyorChainActor.h"
#include "FGConveyorItem.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "GAFB_RCO.h"

void UGAFB_BPFL::RemoveAllItemsFromChain(AFGConveyorChainActor* chain)
{
	if (!chain)
	{
		return;
	}

	while (chain->mLeadItemIndex != INDEX_NONE)
	{
		chain->RemoveItemAt_Internal(chain->mLeadItemIndex);
	}
}

void UGAFB_BPFL::ClearBeltChain(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller, bool giveItems)
{
	if (!belt)
	{
		return;
	}

	AFGConveyorChainActor* chain = belt->GetConveyorChainActor();
	if (!chain)
	{
		return;
	}

	if (giveItems)
	{
		if (!controller)
		{
			return;
		}

		for (const FConveyorBeltItem& beltItem : chain->mConveyorChainItems)
		{
			controller->Server_GiveItemSingle_Implementation(beltItem.Item.GetItemClass(), 1);
		}
	}

	RemoveAllItemsFromChain(chain);
}

void UGAFB_BPFL::GatherLinkedBeltNetworkRecursive(AFGBuildableConveyorBelt* belt, TSet<AFGBuildableConveyorBelt*>& visitedBelts, TSet<AFGBuildableConveyorAttachment*>& visitedAttachments, FGAFBLinkedBeltNetwork& outNetwork)
{
	if (!belt || visitedBelts.Contains(belt))
	{
		return;
	}

	AFGConveyorChainActor* chain = belt->GetConveyorChainActor();
	if (!chain)
	{
		return;
	}

	visitedBelts.Add(belt);
	outNetwork.Belts.Add(belt);

	auto VisitChainConnection = [&](UFGFactoryConnectionComponent* beltConnection)
		{
			if (!beltConnection)
			{
				return;
			}

			UFGFactoryConnectionComponent* connectedConnection = beltConnection->GetConnection();
			if (!connectedConnection)
			{
				return;
			}

			AActor* owner = connectedConnection->GetOwner();
			if (!owner || owner == belt)
			{
				return;
			}

			AFGBuildableConveyorAttachment* attachment = Cast<AFGBuildableConveyorAttachment>(owner);
			if (!attachment || visitedAttachments.Contains(attachment))
			{
				return;
			}

			visitedAttachments.Add(attachment);
			outNetwork.Attachments.Add(attachment);

			TInlineComponentArray<UFGFactoryConnectionComponent*> attachmentConnections;
			attachment->GetComponents(attachmentConnections);

			for (UFGFactoryConnectionComponent* attachmentConnection : attachmentConnections)
			{
				if (!attachmentConnection || attachmentConnection == connectedConnection)
				{
					continue;
				}

				UFGFactoryConnectionComponent* otherConnection = attachmentConnection->GetConnection();
				if (!otherConnection)
				{
					continue;
				}

				AFGBuildableConveyorBelt* connectedBelt =
					Cast<AFGBuildableConveyorBelt>(otherConnection->GetOwner());

				if (connectedBelt)
				{
					GatherLinkedBeltNetworkRecursive(connectedBelt, visitedBelts, visitedAttachments, outNetwork);
				}
			}
		};

	VisitChainConnection(chain->mConnection0);
	VisitChainConnection(chain->mConnection1);
}

FGAFBLinkedBeltNetwork UGAFB_BPFL::GatherLinkedBeltNetwork(AFGBuildableConveyorBelt* belt)
{
	FGAFBLinkedBeltNetwork network;
	TSet<AFGBuildableConveyorBelt*> visitedBelts;
	TSet<AFGBuildableConveyorAttachment*> visitedAttachments;

	GatherLinkedBeltNetworkRecursive(belt, visitedBelts, visitedAttachments, network);
	return network;
}

void UGAFB_BPFL::ClearBelt(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	ClearBeltChain(belt, controller, true);
}

void UGAFB_BPFL::ClearBeltVisual(AFGBuildableConveyorBelt* belt)
{
	ClearBeltChain(belt, nullptr, false);
}

void UGAFB_BPFL::ClearLinkedBelts(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	TArray<AFGBuildableConveyorBelt*> ignoredBelts;
	ClearLinkedBeltsAndCollect(belt, controller, ignoredBelts);
}

void UGAFB_BPFL::ClearLinkedBeltsAndCollect(AFGBuildableConveyorBelt* belt,	AFGPlayerController* controller, TArray<AFGBuildableConveyorBelt*>& outBelts)
{
	if (!controller)
	{
		return;
	}

	FGAFBLinkedBeltNetwork network = GatherLinkedBeltNetwork(belt);
	outBelts = network.Belts;

	for (AFGBuildableConveyorAttachment* attachment : network.Attachments)
	{
		if (attachment && attachment->GetBufferInventory())
		{
			attachment->GetBufferInventory()->Empty();
		}
	}

	for (AFGBuildableConveyorBelt* linkedBelt : network.Belts)
	{
		ClearBelt(linkedBelt, controller);
	}
}

void UGAFB_BPFL::ClearLinkedBeltsRemote(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	if (!belt || !controller)
	{
		return;
	}

	UGAFB_RCO* rco = controller->GetRemoteCallObjectOfClass<UGAFB_RCO>();
	if (!rco)
	{
		return;
	}

	rco->Server_ClearLinkedBelts(belt, controller);
}

void UGAFB_BPFL::ClearSingleBeltRemote(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller)
{
	if (!belt || !controller)
	{
		return;
	}

	UGAFB_RCO* rco = controller->GetRemoteCallObjectOfClass<UGAFB_RCO>();
	if (!rco)
	{
		return;
	}

	rco->Server_ClearSingleBelt(belt, controller);
}
