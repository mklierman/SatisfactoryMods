
#include "OutlineLootSubsystem.h"
#include "Patching/NativeHookManager.h"
#include "FGItemPickup.h"
#include "FGInventoryComponent.h"
#include <Logging/StructuredLog.h>
#include "Resources/FGItemDescriptor.h"
#include <Kismet/GameplayStatics.h>
#include <SessionSettingsManager.h>
#include <OutlineLoot_ConfigurationStruct.h>

DEFINE_LOG_CATEGORY(LogOutlineLoot);
//void AOutlineLootSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(AOutlineLootSubsystem, OnItemPickupBeginPlay);
//}

void AOutlineLootSubsystem::BeginPlay()
{
	Super::BeginPlay();

	auto config = FOutlineLoot_ConfigurationStruct::GetActiveConfig(this);
	UConfigManager* ConfigManager = this->GetGameInstance()->GetSubsystem<UConfigManager>();
	FConfigId configId;
	configId.ModReference = "OutlineLoot";
	auto section = ConfigManager->GetConfigurationRootSection(configId);
	auto colorSection = section->SectionProperties["OutlineColor"];
	auto scaleSection = section->SectionProperties["OutlineSize"];
	colorChanged.BindUFunction(this, FName("OnColorChanged"));
	sizeChanged.BindUFunction(this, FName("OnSizeChanged"));
	colorSection->OnPropertyValueChanged.AddUnique(colorChanged);
	scaleSection->OnPropertyValueChanged.AddUnique(sizeChanged);

	material = LoadObject<UMaterialInstance>(NULL, TEXT("/OutlineLoot/Materials/MI_Outline.MI_Outline"));
	ApplyToAll();

#if !WITH_EDITOR
#endif

	AFGItemPickup_Spawnable* ips = GetMutableDefault<AFGItemPickup_Spawnable>();
	Hook = SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGItemPickup_Spawnable::BeginPlay, ips, [this](AFGItemPickup_Spawnable* self)
		{
			//UE_LOGFMT(LogOutlineLoot, Display, "Pickup Item: {0}", name);
			ApplySingle(self);
		});
}

void AOutlineLootSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !WITH_EDITOR
	if (Hook.IsValid())
	{
		UNSUBSCRIBE_METHOD(AFGItemPickup_Spawnable::BeginPlay, Hook);
	}
#endif
}

FLinearColor AOutlineLootSubsystem::GetColor(AActor* actor)
{
	auto config = FOutlineLoot_ConfigurationStruct::GetActiveConfig(actor);
	auto colorInt = config.OutlineColor;
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

float AOutlineLootSubsystem::GetScale(AActor* actor)
{
	auto config = FOutlineLoot_ConfigurationStruct::GetActiveConfig(actor);
	auto scale = config.OutlineSize;
	return scale;
}

void AOutlineLootSubsystem::ApplyToAll()
{
	TArray<AActor*> out_Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AFGItemPickup_Spawnable::StaticClass(), out_Actors);
	for (auto actor : out_Actors)
	{
		auto pickup = Cast<AFGItemPickup_Spawnable>(actor);
		if (pickup)
		{
			ApplySingle(pickup);
		}
	}
}

void AOutlineLootSubsystem::ApplySingle(AFGItemPickup_Spawnable* pickup)
{
	UActorComponent* component = nullptr;
	auto components = pickup->GetComponents();
	for (auto comp : components)
	{
		auto name = comp->GetName();
		if (name == "OutlineLootMesh")
		{
			component = comp;
		}
	}
	if (!component)
	{
		component = pickup->AddComponentByClass(UStaticMeshComponent::StaticClass(), false, FTransform(FRotator(0), FVector(0), FVector(1)), false);
		component->Rename(TEXT("OutlineLootMesh"));
	}
	if (component)
	{
		auto staticMeshComponent = Cast<UStaticMeshComponent>(component);
		staticMeshComponent->SetStaticMesh(pickup->mMeshComponent->GetStaticMesh());
		staticMeshComponent->SetCullDistance(pickup->GetSignificanceRange() * 5);
		auto dmi = staticMeshComponent->CreateDynamicMaterialInstance(0, material);
		dmi->SetVectorParameterValue("Color", GetColor(pickup));
		dmi->SetScalarParameterValue("Scale", GetScale(pickup));
	}
}

void AOutlineLootSubsystem::OnColorChanged()
{
	ApplyToAll();
}

void AOutlineLootSubsystem::OnSizeChanged()
{
	ApplyToAll();
}
