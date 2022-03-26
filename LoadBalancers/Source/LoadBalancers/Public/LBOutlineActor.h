

#pragma once

#include "CoreMinimal.h"
#include "FGColoredInstanceMeshProxy.h"
#include "LoadBalancersModule.h"
#include "LBOutlineActor.generated.h"

/**
Actor for represend outlines
*/
UCLASS()
class LOADBALANCERS_API ALBOutlineActor : public AActor
{
	GENERATED_BODY()
	public:

	ALBOutlineActor()
	{
		bReplicates = false;
		//NetDormancy = DORM_Awake;
	}

	virtual void BeginPlay() override
	{
		Super::BeginPlay();

		SetActorEnableCollision(false);
	};

	private:
	FORCEINLINE void CreateOutlineMesh(UStaticMeshComponent* OtherMeshComponent, UMaterialInterface* OutlineMaterial)
	{
		if(OtherMeshComponent && OutlineMaterial)
			if(UStaticMesh* Mesh = OtherMeshComponent->GetStaticMesh())
			{
				FString MeshName = OtherMeshComponent->GetStaticMesh()->GetName();
				
				if(MeshName != FString("ClearanceBox") && MeshName != FString("Arrows"))
					if(UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(this))
					{
						NewComponent->SetRelativeTransform(OtherMeshComponent->GetComponentTransform());
						NewComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
						NewComponent->SetStaticMesh(Mesh);

						for(int i = 0; i < NewComponent->GetNumMaterials(); ++i)
							NewComponent->SetMaterial(i, OutlineMaterial);
					
						NewComponent->RegisterComponent();
					
						NewComponent->SetRenderCustomDepth(true);
						NewComponent->SetCustomDepthStencilValue(250);

						NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

						TArray<float> CustomPrimitivDatas;
						//OtherMeshComponent->SetHiddenInGame(true);
						if(UFGColoredInstanceMeshProxy* MeshProxy = Cast<UFGColoredInstanceMeshProxy>(OtherMeshComponent))
							if(MeshProxy->mInstanceHandle.IsInstanced())
							{
								//MeshProxy->SetInstanced(false);
								CustomPrimitivDatas = MeshProxy->mInstanceHandle.GetCustomData();
							}

						if(CustomPrimitivDatas.Num() == 0)
							CustomPrimitivDatas = OtherMeshComponent->GetCustomPrimitiveData().Data;

						if(CustomPrimitivDatas.Num() > 0)
						{
							for(int i = 0; i < CustomPrimitivDatas.Num(); ++i)
								NewComponent->SetCustomPrimitiveDataFloat(i, CustomPrimitivDatas[i]);
						}
					}
				}
			
	}
	

	FORCEINLINE virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		ResetOther();
		Super::EndPlay(EndPlayReason);
	};

	public:
	FORCEINLINE void CreateOutlineFromActor(AActor* Actor, UMaterialInterface* OutlineMaterial)
	{
		if(Actor && OutlineMaterial)
		{
			for (UActorComponent* ComponentsByClass : Actor->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
				if(UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ComponentsByClass))
				{
					CreateOutlineMesh(MeshComponent, OutlineMaterial);
				}
			mOutlinedActor = Actor;
		}
	}
	
	FORCEINLINE void ResetOther()
	{
		if(mOutlinedActor.IsValid())
			for (UActorComponent* ComponentsByClass : mOutlinedActor.Get()->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
				if(UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ComponentsByClass))
				{
					//if(UFGColoredInstanceMeshProxy* MeshProxy = Cast<UFGColoredInstanceMeshProxy>(ComponentsByClass))
						//if(!MeshProxy->mInstanceHandle.IsInstanced() && !MeshProxy->mBlockInstancing)
							//MeshProxy->SetInstanced(true);
					//MeshComponent->SetHiddenInGame(false);
				}
	}

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> mOutlinedActor;
};
