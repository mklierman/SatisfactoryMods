


#include "MSS_BPFL.h"

void UMSS_BPFL::AddSignSpotToDecorator(UClass* DecoClass, FName Name, FTransform Transform, EAttachmentPointUsage Usage, TSubclassOf< class UFGAttachmentPointType > Type)
{
	auto* blueprintClass = CastChecked<UBlueprintGeneratedClass>(DecoClass);
	if (!blueprintClass)
	{
		return;
	}

	auto* AttachmentPointComp = NewObject<UFGAttachmentPointComponent>(blueprintClass, Name, RF_ArchetypeObject);
	AttachmentPointComp->SetRelativeTransform_Direct(Transform);
	AttachmentPointComp->mUsage = Usage;
	AttachmentPointComp->mType = Type;

	USimpleConstructionScript* constructionScript = blueprintClass->SimpleConstructionScript;
	USCS_Node* node = NewObject<USCS_Node>(constructionScript, Name);
	node->ComponentClass = UFGAttachmentPointComponent::StaticClass();
	node->ComponentTemplate = AttachmentPointComp;

	constructionScript->AddNode(node);
}
