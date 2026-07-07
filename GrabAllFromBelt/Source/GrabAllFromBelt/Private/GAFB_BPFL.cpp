#include "GAFB_BPFL.h"
#include "FGFactoryConnectionComponent.h"
#include "FGConveyorChainActor.h"
#include "FGConveyorItem.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableConveyorAttachment.h"
#include "GAFB_RCO.h"

AFGConveyorChainActor* UGAFB_BPFL::GetBeltChain(AFGBuildableConveyorBelt* belt)
{
	if (!belt)
	{
		return nullptr;
	}

	return belt->GetConveyorChainActor();
}

void UGAFB_BPFL::GiveChainItemsToPlayer(AFGConveyorChainActor* chain, AFGPlayerController* controller)
{
	for (const FConveyorBeltItem& beltItem : chain->mConveyorChainItems)
	{
		controller->Server_GiveItemSingle_Implementation(beltItem.Item.GetItemClass(), 1);
	}
}

FGAFBAttachmentLink UGAFB_BPFL::GetAttachmentLinkedToBeltConnection(UFGFactoryConnectionComponent* beltConnection, AFGBuildableConveyorBelt* sourceBelt)
{
	FGAFBAttachmentLink link;
	if (!beltConnection)
	{
		return link;
	}

	UFGFactoryConnectionComponent* connectedConnection = beltConnection->GetConnection();
	if (!connectedConnection)
	{
		return link;
	}

	AActor* owner = connectedConnection->GetOwner();
	if (!owner || owner == sourceBelt)
	{
		return link;
	}

	link.Attachment = Cast<AFGBuildableConveyorAttachment>(owner);
	link.ConnectionOnAttachment = connectedConnection;
	return link;
}

TArray<AFGBuildableConveyorBelt*> UGAFB_BPFL::GetBeltsLinkedToAttachment(AFGBuildableConveyorAttachment* attachment, UFGFactoryConnectionComponent* connectionToSkip)
{
	TArray<AFGBuildableConveyorBelt*> linkedBelts;
	if (!attachment)
	{
		return linkedBelts;
	}

	TInlineComponentArray<UFGFactoryConnectionComponent*> attachmentConnections;
	attachment->GetComponents(attachmentConnections);

	for (UFGFactoryConnectionComponent* attachmentConnection : attachmentConnections)
	{
		if (!attachmentConnection || attachmentConnection == connectionToSkip)
		{
			continue;
		}

		UFGFactoryConnectionComponent* connectedConnection = attachmentConnection->GetConnection();
		if (!connectedConnection)
		{
			continue;
		}

		AFGBuildableConveyorBelt* linkedBelt =
			Cast<AFGBuildableConveyorBelt>(connectedConnection->GetOwner());

		if (linkedBelt)
		{
			linkedBelts.Add(linkedBelt);
		}
	}

	return linkedBelts;
}

void UGAFB_BPFL::EmptyAttachmentBuffer(AFGBuildableConveyorAttachment* attachment)
{
	if (attachment && attachment->GetBufferInventory())
	{
		attachment->GetBufferInventory()->Empty();
	}
}

UGAFB_RCO* UGAFB_BPFL::GetGrabAllRemoteCallObject(AFGPlayerController* controller)
{
	if (!controller)
	{
		return nullptr;
	}

	return controller->GetRemoteCallObjectOfClass<UGAFB_RCO>();
}

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
	AFGConveyorChainActor* chain = GetBeltChain(belt);
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

		GiveChainItemsToPlayer(chain, controller);
	}

	RemoveAllItemsFromChain(chain);
}

void UGAFB_BPFL::GatherLinkedBeltNetworkRecursive(AFGBuildableConveyorBelt* belt, TSet<AFGBuildableConveyorBelt*>& visitedBelts, TSet<AFGBuildableConveyorAttachment*>& visitedAttachments, FGAFBLinkedBeltNetwork& outNetwork)
{
	if (!belt || visitedBelts.Contains(belt))
	{
		return;
	}

	AFGConveyorChainActor* chain = GetBeltChain(belt);
	if (!chain)
	{
		return;
	}

	visitedBelts.Add(belt);
	outNetwork.Belts.Add(belt);

	auto VisitChainConnection = [&](UFGFactoryConnectionComponent* beltConnection)
		{
			const FGAFBAttachmentLink link = GetAttachmentLinkedToBeltConnection(beltConnection, belt);
			AFGBuildableConveyorAttachment* attachment = link.Attachment;
			if (!attachment || visitedAttachments.Contains(attachment))
			{
				return;
			}

			visitedAttachments.Add(attachment);
			outNetwork.Attachments.Add(attachment);

			const TArray<AFGBuildableConveyorBelt*> linkedBelts =
				GetBeltsLinkedToAttachment(attachment, link.ConnectionOnAttachment);
			for (AFGBuildableConveyorBelt* linkedBelt : linkedBelts)
			{
				GatherLinkedBeltNetworkRecursive(linkedBelt, visitedBelts, visitedAttachments, outNetwork);
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

void UGAFB_BPFL::ClearLinkedBeltsAndCollect(AFGBuildableConveyorBelt* belt, AFGPlayerController* controller, TArray<AFGBuildableConveyorBelt*>& outBelts)
{
	if (!controller)
	{
		return;
	}

	FGAFBLinkedBeltNetwork network = GatherLinkedBeltNetwork(belt);
	outBelts = network.Belts;

	for (AFGBuildableConveyorAttachment* attachment : network.Attachments)
	{
		EmptyAttachmentBuffer(attachment);
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

	UGAFB_RCO* rco = GetGrabAllRemoteCallObject(controller);
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

	UGAFB_RCO* rco = GetGrabAllRemoteCallObject(controller);
	if (!rco)
	{
		return;
	}

	rco->Server_ClearSingleBelt(belt, controller);
}
