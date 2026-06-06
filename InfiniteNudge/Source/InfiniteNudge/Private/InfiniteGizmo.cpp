#include "InfiniteGizmo.h"
#include "InfiniteNudge_ConfigurationStruct.h"


TWeakObjectPtr<UInfiniteGizmo> _instance = nullptr;

UInfiniteGizmo::UInfiniteGizmo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInfiniteGizmo::Init(const AFGHologram* holo) {
	mHologram = holo;
	if (!mHologram.IsValid() || !mHologram->GetRootComponent())
		return;
 
	for (auto& gizmo : mGizmos) {		
		auto comp = NewObject<UStaticMeshComponent>(this, gizmo.name);
		comp->SetCastShadow(false);
		comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//render properly against the hologram
		comp->SetRenderCustomDepth(true);
		comp->SetCustomDepthStencilValue(1);
		comp->SetCustomDepthStencilWriteMask(ERendererStencilMask::ERSM_255);

		comp->SetWorldRotation(gizmo.meshRotation);
		comp->SetWorldScale3D(FVector(gizmo.meshScale, gizmo.meshScale, gizmo.meshScale));

		auto material = UMaterialInstanceDynamic::Create(gizmo.staticMesh->GetMaterial(0), this);
		material->SetVectorParameterValue(TEXT("baseColor"), gizmo.color);

		comp->SetStaticMesh(gizmo.staticMesh);
		comp->SetMaterial(0, material);

		comp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		comp->RegisterComponent();

		gizmo.component = comp;
		gizmo.material = material;
	}

	auto config = FInfiniteNudge_ConfigurationStruct::GetActiveConfig(holo->GetWorld());
	bIsVisible = config.ShowScrollModeGizmo;
	SetVisibility(bIsVisible, true);

	AttachToComponent(mHologram->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	RegisterComponent();
}

void UInfiniteGizmo::TickComponent(float delta, ELevelTick tickType, FActorComponentTickFunction* callback)
{
	auto speed = 20.f;
	//Animate the transition when EScrollMode changes
	for (auto& gizmo : mGizmos) {
//		UE_LOG(InfiniteNudge, Verbose, TEXT("Gizmo : %s (%.2f)"), *gizmo.name.ToString(), gizmo.progress);
		float idk = (float)ScrollMode::Get() == (float)gizmo.scrollMode;
		gizmo.progress = FMath::FInterpTo(gizmo.progress, idk, delta, speed);
		gizmo.material->SetScalarParameterValue(TEXT("progress"), gizmo.progress);
	}
}

bool UInfiniteGizmo::Toggle() {
	bIsVisible = !bIsVisible;
	SetVisibility(bIsVisible, true);
	return bIsVisible;
}


//STATIC FUNCTIONS
bool UInfiniteGizmo::ToggleVisibility() {
	if (!_instance.IsValid())
		return false;
	return _instance->Toggle();
}

bool UInfiniteGizmo::IsGizmoVisible() {
	if (!_instance.IsValid())
		return false;
	return _instance->bIsVisible;
}

void UInfiniteGizmo::UpdatePivot(FVector position) {
	if (!_instance.IsValid())
		return;
	_instance->SetRelativeLocation(position);
}

void UInfiniteGizmo::Create(AFGHologram* hologram) {
	auto* gizmoClass = LoadObject<UClass>(nullptr, TEXT("/InfiniteNudge/Gizmo/BP_InfiniteGizmo.BP_InfiniteGizmo_C"));

	_instance = NewObject<UInfiniteGizmo>(hologram, gizmoClass);
	_instance->Init(hologram);
}

void UInfiniteGizmo::Destroy() {
	if (_instance.IsValid()) {
		for (auto& gizmo : _instance->mGizmos) {
			if (gizmo.component) {
				gizmo.component->UnregisterComponent();
				gizmo.component->DestroyComponent();
			}
			gizmo.component = nullptr;
		} 	
		_instance->UnregisterComponent();
		_instance->DestroyComponent();
	}	
	_instance = nullptr;
}
