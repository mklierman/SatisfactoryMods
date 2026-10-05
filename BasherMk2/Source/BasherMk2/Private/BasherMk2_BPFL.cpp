#include "BasherMk2_BPFL.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
#include "FGEquipmentChild.h"

void UBasherMk2_BPFL::SetMeshComponentVectorParameter(UMeshComponent* meshComponent, FName parameterName, FLinearColor value)
{
	if (!meshComponent)
		return;

	int32 numMaterials = meshComponent->GetNumMaterials();
	for (int32 i = 0; i < numMaterials; ++i)
	{
		auto currentMat = meshComponent->GetMaterial(i);
		if (!currentMat)
			continue;

		auto dynamicMat = Cast<UMaterialInstanceDynamic>(currentMat);
		if (!dynamicMat)
		{
			dynamicMat = meshComponent->CreateDynamicMaterialInstance(i, currentMat);
		}

		if (dynamicMat)
		{
			dynamicMat->SetVectorParameterValue(parameterName, value);
		}
	}
}

void UBasherMk2_BPFL::SetActorMeshVectorParameter(AActor* targetActor, FName parameterName, FLinearColor value)
{
	if (!targetActor)
		return;

	TArray<UMeshComponent*> meshComponents;
	targetActor->GetComponents<UMeshComponent>(meshComponents);

	for (auto meshComp : meshComponents)
	{
		SetMeshComponentVectorParameter(meshComp, parameterName, value);
	}
}

void UBasherMk2_BPFL::UpdateMaterialMapInternal(TMap<FName, FFirstPersonMaterialArray>& inOutMaterialMap, FName parameterName, FLinearColor value, UObject* worldContext)
{
	for (auto& pair : inOutMaterialMap)
	{
		for (auto& matRef : pair.Value.FirstPersonMaterials)
		{
			if (!matRef)
				continue;

			auto dynamicMat = Cast<UMaterialInstanceDynamic>(matRef.Get());
			if (!dynamicMat)
			{
				dynamicMat = UMaterialInstanceDynamic::Create(matRef.Get(), worldContext);
				matRef = dynamicMat;
			}

			if (dynamicMat)
			{
				dynamicMat->SetVectorParameterValue(parameterName, value);
			}
		}
	}
}

void UBasherMk2_BPFL::SetEquipmentVectorParameter(AFGEquipment* equipment, FName parameterName, FLinearColor value)
{
	if (!equipment)
		return;

	SetActorMeshVectorParameter(equipment, parameterName, value);

	UpdateMaterialMapInternal(equipment->mComponentNameToFirstPersonMaterials, parameterName, value, equipment);
	UpdateMaterialMapInternal(equipment->mSwappedOutThirdPersonMaterials, parameterName, value, equipment);

	if (equipment->mChildEquipment)
	{
		SetActorMeshVectorParameter(equipment->mChildEquipment, parameterName, value);
		UpdateMaterialMapInternal(equipment->mChildEquipment->mComponentNameToFirstPersonMaterials, parameterName, value, equipment->mChildEquipment);
		UpdateMaterialMapInternal(equipment->mChildEquipment->mSwappedOutThirdPersonMaterials, parameterName, value, equipment->mChildEquipment);
	}
}
