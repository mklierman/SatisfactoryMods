#include "Graffiti.h"

#include "FGBuildableGraffitiContainer.h"
#include "GraffitiBlueprintHologramVisualizer.h"
#include "Hologram/FGBlueprintHologram.h"

namespace
{
	void CreateGraffitiBlueprintVisualization(
		AFGBlueprintHologram* BlueprintHologram,
		AFGBuildable* Buildable,
		USceneComponent* BuildableRoot)
	{
		const AFGBuildableGraffitiContainer* Container = Cast<AFGBuildableGraffitiContainer>(Buildable);
		if (!IsValid(BlueprintHologram) || !IsValid(Container) || !IsValid(BuildableRoot))
		{
			return;
		}

		UGraffitiBlueprintHologramVisualizer* Visualizer = NewObject<UGraffitiBlueprintHologramVisualizer>(BlueprintHologram);
		BlueprintHologram->AddInstanceComponent(Visualizer);
		Visualizer->RegisterComponent();
		Visualizer->Initialize(BlueprintHologram, Container, BuildableRoot);
	}
}

void FGraffitiModule::StartupModule()
{
	AFGBlueprintHologram::RegisterCustomBuildableVisualization(
		AFGBuildableGraffitiContainer::StaticClass(),
		AFGBlueprintHologram::FCreateBuildableVisualizationDelegate::CreateStatic(&CreateGraffitiBlueprintVisualization));
}

void FGraffitiModule::ShutdownModule() {}

IMPLEMENT_MODULE(FGraffitiModule, Graffiti)
