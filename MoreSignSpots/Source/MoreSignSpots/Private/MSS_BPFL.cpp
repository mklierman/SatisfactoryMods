


#include "MSS_BPFL.h"

void UMSS_BPFL::AddSignSpotToDecorator(UClass* DecoClass, FName Name, FTransform Transform, EAttachmentPointUsage Usage, TSubclassOf< class UFGAttachmentPointType > Type)
{
	auto* BlueprintClass = Cast<UBlueprintGeneratedClass>(DecoClass);
	if (!IsValid(BlueprintClass))
	{
		return;
	}

	USimpleConstructionScript* ConstructionScript = BlueprintClass->SimpleConstructionScript;
	if (!IsValid(ConstructionScript))
	{
		return;
	}

	for (USCS_Node* ExistingNode : ConstructionScript->GetAllNodes())
	{
		if (IsValid(ExistingNode) && ExistingNode->GetVariableName() == Name)
		{
			return;
		}
	}

	auto* AttachmentPointComp = NewObject<UFGAttachmentPointComponent>(
		BlueprintClass,
		Name,
		RF_ArchetypeObject | RF_Transactional);
	if (!IsValid(AttachmentPointComp) || AttachmentPointComp->GetOuter() != BlueprintClass)
	{
		return;
	}

	USCS_Node* Node = NewObject<USCS_Node>(
		ConstructionScript,
		Name,
		RF_Transactional);
	if (!IsValid(Node) || Node->GetOuter() != ConstructionScript)
	{
		return;
	}

	Node->ComponentClass = UFGAttachmentPointComponent::StaticClass();
	Node->ComponentTemplate = AttachmentPointComp;

	AttachmentPointComp->SetRelativeTransform_Direct(Transform);
	AttachmentPointComp->mUsage = Usage;
	AttachmentPointComp->mType = Type;

	ConstructionScript->AddNode(Node);
}
