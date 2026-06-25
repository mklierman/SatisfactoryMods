// Copyright Epic Games, Inc. All Rights Reserved.

#include "LightSettings.h"
#include <Logging/StructuredLog.h>
#include "Patching/NativeHookManager.h"
#include "FGBackgroundThread.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableLightSource.h"
#include "EngineUtils.h"
#include <SessionSettings/SessionSettingsManager.h>

#define LOCTEXT_NAMESPACE "FLightSettingsModule"

DEFINE_LOG_CATEGORY(LightSettings_Log);
void FLightSettingsModule::StartupModule()
{
#if !WITH_EDITOR
	static FPoolLightComponent plc;

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(FPoolLightComponent::Update,	&plc, [this](const bool& result, FPoolLightComponent* self, const TArray<FPoolItem*>& entries)
		{
			if (self)
			{
				SetLightStuff(self->GetActorComponent()->GetWorld());
			}
		});


	//FFGBackgroundThread* fgbgt = GetMutableDefault<FFGBackgroundThread>();
	//SUBSCRIBE_METHOD_AFTER(FFGBackgroundThread::RegisterActor, [this](AActor* NewActor, float instancePriority = 1.f)
	//	{
	//		SetLightStuff(NewActor->GetWorld());
	//	});

	//SUBSCRIBE_METHOD_EXPLICIT_AFTER(void(*)(FPoolHandle*&, bool, UWorld*), FFGBackgroundThread::UpdateItemStateViaHandle, [this](FPoolHandle*& handleToUpdate, bool newStatus, UWorld* WorldContext)
	//	{
	//		SetLightStuff(WorldContext);
	//	});
	//SUBSCRIBE_METHOD_EXPLICIT_AFTER(void(*)(TArray<FPoolHandle*>&, bool, UWorld*), FFGBackgroundThread::UpdateItemStateViaHandle, [this](TArray<FPoolHandle*>& handleToUpdate, bool newStatus, UWorld* WorldContext)
	//	{
	//		SetLightStuff(WorldContext);
	//	});

	//SUBSCRIBE_METHOD_AFTER(APoolRoot::UpdateBuildingState, [this](APoolRoot* self, class AFGBuildable* Building, int32 FlagsToSet)
	//	{
	//		SetLightStuff(self->GetWorld());
	//	});

	//AFGBuildableLightSource* mbls = GetMutableDefault<AFGBuildableLightSource>();
	//SUBSCRIBE_METHOD_AFTER(AFGBuildableLightSource::IsLightEnabled,[this](bool retval, const AFGBuildableLightSource* self)
	//	{
	//		SetLightStuff(self->GetWorld());
	//	});
	//SUBSCRIBE_METHOD_AFTER(AFGBuildableLightSource::ShouldLightBeOn, [this](bool retval, const AFGBuildableLightSource* self)
	//	{
	//		SetLightStuff(self->GetWorld());
	//	});
	//SUBSCRIBE_METHOD_AFTER(AFGBuildableLightSource::SetLightEnabled, [this](AFGBuildableLightSource* self, bool isEnabled)
	//	{
	//		SetLightStuff(self->GetWorld());
	//	});
	//SUBSCRIBE_METHOD_AFTER(AFGBuildableLightSource::SetLightControlData, [this](AFGBuildableLightSource* self, FLightSourceControlData data)
	//	{
	//		SetLightStuff(self->GetWorld());
	//	});
	//SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGBuildableLightSource::BeginPlay, mbls, [this](AFGBuildableLightSource* self)
	//	{
	//		FTimerHandle TimerHandle;

	//		self->GetWorld()->GetTimerManager().SetTimer(
	//			TimerHandle,
	//			FTimerDelegate::CreateLambda([this, self]()
	//				{
	//					UE_LOGFMT(LightSettings_Log, Display, "BeginPlay");
	//					SetLightStuff(self->GetWorld());
	//				}),
	//			1.0f,
	//			false
	//		);
	//	});
#endif
}

void FLightSettingsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FLightSettingsModule::SetLightStuff(UWorld* world)
{
	if (world)
	{
		USessionSettingsManager* SessionSettings = world->GetSubsystem<USessionSettingsManager>();

		for (TActorIterator<APoolRoot> It(world); It; ++It)
		{
			APoolRoot* PoolRoot = *It;
			if (!PoolRoot)
			{
				continue;
			}

			for (UActorComponent* Component : PoolRoot->Components)
			{
				if (USpotLightComponent* Light = Cast<USpotLightComponent>(Component))
				{
					Light->SetInnerConeAngle(SessionSettings->GetFloatOptionValue("LightSettings.InnerConeAngle"));
					Light->SetOuterConeAngle(SessionSettings->GetFloatOptionValue("LightSettings.OuterConeAngle"));
					Light->SetLightFalloffExponent(SessionSettings->GetFloatOptionValue("LightSettings.LightFalloff"));
					Light->SetAttenuationRadius(SessionSettings->GetFloatOptionValue("LightSettings.AttenuationRadius"));
					Light->SetSourceRadius(SessionSettings->GetFloatOptionValue("LightSettings.SourceRadius"));
					Light->SetSoftSourceRadius(SessionSettings->GetFloatOptionValue("LightSettings.SoftSourceRadius"));
					Light->SetUseInverseSquaredFalloff(false);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLightSettingsModule, LightSettings)