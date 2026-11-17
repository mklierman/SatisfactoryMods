#include "FGBuildableGraffitiContainer.h"

#include "Components/SceneComponent.h"
#include "GraffitiConstants.h"
#include "GraffitiSubsystem.h"
#include "Net/UnrealNetwork.h"

AFGBuildableGraffitiContainer::AFGBuildableGraffitiContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GraffitiContainerRoot"));
	SetRootComponent(SceneRoot);
}

FGuid AFGBuildableGraffitiContainer::AddGraffiti(
	const FTransform& WorldTransform,
	const FPersistentGlobalIconId& IconId,
	const FLinearColor& Tint,
	float Size,
	bool bMirrorHorizontal)
{
	if (!HasAuthority())
	{
		return {};
	}

	FGraffitiContainerEntry& Entry = Entries.AddDefaulted_GetRef();
	Entry.EntryId = FGuid::NewGuid();
	Entry.RelativeTransform = WorldTransform.GetRelativeTransform(GetActorTransform());
	Entry.IconId = IconId;
	Entry.Tint = Tint.GetClamped();
	Entry.Size = FMath::Clamp(Size, Graffiti::MinSize, Graffiti::MaxSize);
	Entry.bMirrorHorizontal = bMirrorHorizontal;

	if (AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this))
	{
		Subsystem->RegisterContainerEntry(
			this,
			Entry.EntryId,
			Entry.RelativeTransform * GetActorTransform(),
			Entry.IconId,
			Entry.Tint,
			Entry.Size,
			Entry.bMirrorHorizontal,
			Entry.LayerOrder);
		bEntriesRegistered = true;
	}

	ForceNetUpdate();
	return Entry.EntryId;
}

bool AFGBuildableGraffitiContainer::RemoveGraffitiEntry(const FGuid& EntryId)
{
	if (!HasAuthority())
	{
		return false;
	}

	const int32 EntryIndex = Entries.IndexOfByPredicate([&EntryId](const FGraffitiContainerEntry& Entry)
	{
		return Entry.EntryId == EntryId;
	});
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	if (AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this))
	{
		Subsystem->UnregisterContainerEntry(this, EntryId);
	}
	Entries.RemoveAt(EntryIndex);
	ForceNetUpdate();

	if (Entries.IsEmpty())
	{
		Destroy();
	}
	return true;
}

bool AFGBuildableGraffitiContainer::ContainsEntry(const FGuid& EntryId) const
{
	return Entries.ContainsByPredicate([&EntryId](const FGraffitiContainerEntry& Entry)
	{
		return Entry.EntryId == EntryId;
	});
}

void AFGBuildableGraffitiContainer::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority() || RegisterEntries())
	{
		SetActorTickEnabled(false);
	}
}

void AFGBuildableGraffitiContainer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && EndPlayReason == EEndPlayReason::Destroyed)
	{
		UnregisterEntries();
	}
	Super::EndPlay(EndPlayReason);
}

void AFGBuildableGraffitiContainer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority() || RegisterEntries())
	{
		SetActorTickEnabled(false);
	}
}

void AFGBuildableGraffitiContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFGBuildableGraffitiContainer, Entries);
}

void AFGBuildableGraffitiContainer::OnRep_Entries() {}

bool AFGBuildableGraffitiContainer::RegisterEntries()
{
	if (bEntriesRegistered)
	{
		return true;
	}

	AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this);
	if (!Subsystem)
	{
		return false;
	}

	for (FGraffitiContainerEntry& Entry : Entries)
	{
		if (!Entry.EntryId.IsValid())
		{
			Entry.EntryId = FGuid::NewGuid();
		}
		Subsystem->RegisterContainerEntry(
			this,
			Entry.EntryId,
			Entry.RelativeTransform * GetActorTransform(),
			Entry.IconId,
			Entry.Tint,
			Entry.Size,
			Entry.bMirrorHorizontal,
			Entry.LayerOrder);
	}
	bEntriesRegistered = true;
	ForceNetUpdate();
	return true;
}

void AFGBuildableGraffitiContainer::UnregisterEntries()
{
	if (!bEntriesRegistered)
	{
		return;
	}

	if (AGraffitiSubsystem* Subsystem = AGraffitiSubsystem::Get(this))
	{
		for (const FGraffitiContainerEntry& Entry : Entries)
		{
			Subsystem->UnregisterContainerEntry(this, Entry.EntryId);
		}
	}
	bEntriesRegistered = false;
}

