// Copyright Epic Games, Inc. All Rights Reserved.

#include "OutlineLoot.h"
#include "Patching/NativeHookManager.h"
#include "FGItemPickup.h"
#include "FGInventoryComponent.h"
#include <Logging/StructuredLog.h>
#include "Resources/FGItemDescriptor.h"
#include "FGItemPickup_Spawnable.h"
#include <SessionSettings/SessionSettingsManager.h>
#include <Kismet/GameplayStatics.h>

#define LOCTEXT_NAMESPACE "FOutlineLootModule"
//DEFINE_LOG_CATEGORY(LogOutlineLoot);

void FOutlineLootModule::StartupModule()
{

	//material = LoadObject<UMaterialInstance>(NULL, TEXT("/OutlineLoot/Materials/MI_Outline.MI_Outline"));
	//TArray<AActor*> out_Actors;
	//UGameplayStatics::GetAllActorsOfClass(this, AFGItemPickup_Spawnable::StaticClass(), out_Actors);
	//for (auto actor : out_Actors)
	//{
	//	auto pickup = Cast<AFGItemPickup_Spawnable>(actor);
	//	if (pickup)
	//	{
	//		auto component = pickup->AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform(FRotator(0), FVector(0),FVector(1)), false);
	//		auto staticMeshComponent = Cast<UStaticMeshComponent>(component);
	//		staticMeshComponent->SetStaticMesh(pickup->mMeshComponent->GetStaticMesh());
	//		staticMeshComponent->SetCullDistance(pickup->GetSignificanceRange());
	//		auto dmi = staticMeshComponent->CreateDynamicMaterialInstance(0, material);
	//		dmi->SetVectorParameterValue("Color", GetColor(pickup));
	//		dmi->SetScalarParameterValue("Scale", GetScale(pickup));
	//	}
	//}
#if !WITH_EDITOR
#endif

	//AFGItemPickup_Spawnable* ips = GetMutableDefault<AFGItemPickup_Spawnable>();
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup_Spawnable::BeginPlay, ips, [this](AFGItemPickup_Spawnable* self)
	//	{
	//		//UE_LOGFMT(LogOutlineLoot, Display, "Pickup Item: {0}", name);

	//		auto component = self->AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform(FRotator(0), FVector(0), FVector(1)), false);
	//		auto staticMeshComponent = Cast<UStaticMeshComponent>(component);
	//		staticMeshComponent->SetStaticMesh(self->mMeshComponent->GetStaticMesh());
	//		staticMeshComponent->SetCullDistance(self->GetSignificanceRange());
	//		auto dmi = staticMeshComponent->CreateDynamicMaterialInstance(0, material);
	//		dmi->SetVectorParameterValue("Color", GetColor(self));
	//		dmi->SetScalarParameterValue("Scale", GetScale(self));
	//	});
}

void FOutlineLootModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

FLinearColor FOutlineLootModule::GetColor(AActor* actor)
{
	USessionSettingsManager* SessionSettings = actor->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto colorInt = SessionSettings->GetIntOptionValue("OutlineLoot.OutlineColor");
	switch (colorInt)
	{
	case 0:
		return FLinearColor::White;
	case 1:
		return FLinearColor::Red;
	case 2:
		return FLinearColor::Green;
	case 3:
		return FLinearColor::Yellow;
	case 4:
		return FLinearColor::Blue;
	case 5:
		return FLinearColor(0.913726, 0.0, 0.952941, 1.0);
	}
	return FLinearColor::White;
}

float FOutlineLootModule::GetScale(AActor* actor)
{
	USessionSettingsManager* SessionSettings = actor->GetWorld()->GetSubsystem<USessionSettingsManager>();
	auto scale = SessionSettings->GetFloatOptionValue("OutlineLoot.OutlineSize");
	return scale;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOutlineLootModule, OutlineLoot)