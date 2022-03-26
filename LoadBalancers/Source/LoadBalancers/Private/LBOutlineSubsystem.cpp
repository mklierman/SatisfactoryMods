#include "LBOutlineSubsystem.h"

#include "FGGameUserSettings.h"
#include "LBDefaultRCO.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystem/SubsystemActorManager.h"

ALBOutlineSubsystem* ALBOutlineSubsystem::Get(UObject* worldContext)
{
	if(worldContext)
	{
		const UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(worldContext);
		USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
		check(SubsystemActorManager);
		
		for (auto Subsystem : SubsystemActorManager->SubsystemActors)
			if(Subsystem.Key->IsChildOf(StaticClass()))
				return Cast<ALBOutlineSubsystem>(Subsystem.Value);
	}
	return nullptr;
}

void ALBOutlineSubsystem::BeginPlay()
{
	Super::BeginPlay();

	mGameUserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if(mGameUserSettings)
	{
		mCachedDismantleColor = mGameUserSettings->mDismantleHologramColour;
	}
}

void ALBOutlineSubsystem::MultiCast_CreateOutlineForActor_Implementation(AActor* Actor)
{
	CreateOutline(Actor);
}

void ALBOutlineSubsystem::MultiCast_ClearOutlines_Implementation()
{
	ClearOutlines();
	ResetOutlineColor();
}

void ALBOutlineSubsystem::MultiCast_SetOutlineColor_Implementation(FLinearColor Color)
{
	SetOutlineColor(Color);
}

void ALBOutlineSubsystem::MultiCast_ResetOutlineColor_Implementation()
{
	ResetOutlineColor();
}

void ALBOutlineSubsystem::CreateOutline(AActor* Actor, bool Multicast)
{
	if(Multicast)
	{
		if(HasAuthority())
			MultiCast_CreateOutlineForActor(Actor);
		else
		{
			ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld());
			if(RCO)
				RCO->Server_CreateOutlineForActor(this, Actor);
		}
	}
	else
	{
		if(!mOutlineMap.Contains(Actor))
			if(Actor)
			{
				FTransform Transform = Actor->GetTransform();
				Transform.SetScale3D(Transform.GetScale3D() + FVector(.1));
				if(ALBOutlineActor* OutlineActor = GetWorld()->SpawnActorDeferred<ALBOutlineActor>(ALBOutlineActor::StaticClass(), Transform, GetWorld()->GetFirstPlayerController()))
				{
					OutlineActor->CreateOutlineFromActor(Actor, mHoloMaterial);
					OutlineActor->FinishSpawning(Transform, true);
					mOutlineMap.Add(Actor, OutlineActor);
				}
			}
	}
}

void ALBOutlineSubsystem::ClearOutlines(bool Multicast)
{
	if(Multicast)
	{
		if(HasAuthority())
			MultiCast_ClearOutlines();
		else
		{
			ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld());
			if(RCO)
				RCO->Server_ClearOutlines(this);
		}
	}
	else
	{
		if(mOutlineMap.Num() > 0)
			for (auto OutlineActor : mOutlineMap)
				if(OutlineActor.Value)
				{
					OutlineActor.Value->ResetOther();
					OutlineActor.Value->Destroy();
				}
		mOutlineMap.Empty();
	}
}

void ALBOutlineSubsystem::SetOutlineColor(FLinearColor Color, bool Multicast)
{
	if(Multicast)
	{
		if(HasAuthority())
			MultiCast_SetOutlineColor(Color);
		else
		{
			ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld());
			if(RCO)
				RCO->Server_SetOutlineColor(this, UKismetMathLibrary::Conv_LinearColorToVector(Color));
		}
	}
	else
	{
		if(!mGameUserSettings)
			mGameUserSettings = UFGGameUserSettings::GetFGGameUserSettings();
		
		if(mGameUserSettings)
		{
			mGameUserSettings->SetDismantleHologramColour(UKismetMathLibrary::Conv_LinearColorToVector(Color));
			mGameUserSettings->ApplyHologramColoursToCollectionParameterInstance(GetWorld());
		}
	}
}

void ALBOutlineSubsystem::ResetOutlineColor(bool Multicast)
{
	if(Multicast)
	{
		if(HasAuthority())
			MultiCast_ResetOutlineColor();
		else
		{
			ULBDefaultRCO* RCO = ULBDefaultRCO::Get(GetWorld());
			if(RCO)
				RCO->Server_ResetOutlineColor(this);
		}
	}
	else
	{
		if(!mGameUserSettings)
			mGameUserSettings = UFGGameUserSettings::GetFGGameUserSettings();
		
		if(mGameUserSettings)
		{
			mGameUserSettings->SetDismantleHologramColour(mCachedDismantleColor);
			mGameUserSettings->ApplyHologramColoursToCollectionParameterInstance(GetWorld());
		}
	}
}
