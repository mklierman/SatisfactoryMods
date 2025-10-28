#include "NoZFightingModule.h"
#include "Patching/NativeHookManager.h"
#include "Hologram/FGBuildableHologram.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Buildables/FGBuildableWall.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGLightweightBuildableSubsystem.h"
#include <Buildables/FGBuildableWalkway.h>
#include <Hologram/FGFoundationHologram.h>


//#pragma optimize("", off)
void FNoZFightingModule::StartupModule() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(AAbstractInstanceManager::SetInstanced, [this](auto& scope, AAbstractInstanceManager* self, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceOwnerHandlePtr& OutHandle, bool bInitializeHidden)
		{
			SetInstanced(self, OwnerActor, ActorTransform, InstanceData, OutHandle, bInitializeHidden);
		});
#endif
}

void FNoZFightingModule::SetInstanced(AAbstractInstanceManager* manager, AActor* OwnerActor, const FTransform& ActorTransform, const FInstanceData& InstanceData, FInstanceOwnerHandlePtr& OutHandle, bool bInitializeHidden)
{

	FName Name;
	if (InstanceData.OverridenMaterials.Num() > 0)
	{
		uint32 Hash = 0;
		for (const UMaterialInterface* Material : InstanceData.OverridenMaterials)
		{
			if (Material)
			{
				Hash = HashCombine(Hash, GetTypeHash(Material->GetFName()));
			}
		}

		Name = FName(InstanceData.StaticMesh->GetName(), Hash);
	}
	else
	{
		Name = InstanceData.StaticMesh->GetFName();
	}

	FInstanceComponentData* InstanceEntry = manager->InstanceMap.Find(Name);
	FInstanceData* ptr;
	ptr = (FInstanceData*)(&InstanceData);

	FRandomStream RandStream;
	RandStream.Initialize(GetTypeHash(ActorTransform));
	
	if (Name.ToString().ToLower().Contains("foundation") || Name.ToString().ToLower().Contains("block")
		|| Name.ToString().ToLower().Contains("quarterpipe") || Name.ToString().ToLower().Contains("ramp"))
	{
		// Foundations and Ramps
		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Foundations.FoundationX * -1.f;
		auto xMax = config.Foundations.FoundationX;
		auto yMin = config.Foundations.FoundationY * -1.f;
		auto yMax = config.Foundations.FoundationY;
		auto zMin = config.Foundations.FoundationZ * -1.f;
		auto zMax = config.Foundations.FoundationZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);

		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("wall"))
	{
		// Walls
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Walls.WallX * -1.f;
		auto xMax = config.Walls.WallX;
		auto yMin = config.Walls.WallY * -1.f;
		auto yMax = config.Walls.WallY;
		auto zMin = config.Walls.WallZ * -1.f;
		auto zMax = config.Walls.WallZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("catwalk") || Name.ToString().ToLower().Contains("walkway"))
	{
		// Catwalks
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Catwalks.CatwalkX * -1.f;
		auto xMax = config.Catwalks.CatwalkX;
		auto yMin = config.Catwalks.CatwalkY * -1.f;
		auto yMax = config.Catwalks.CatwalkY;
		auto zMin = config.Catwalks.CatwalkZ * -1.f;
		auto zMax = config.Catwalks.CatwalkZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("beam"))
	{
		// Beams
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Beams.BeamX * -1.f;
		auto xMax = config.Beams.BeamX;
		auto yMin = config.Beams.BeamY * -1.f;
		auto yMax = config.Beams.BeamY;
		auto zMin = config.Beams.BeamZ * -1.f;
		auto zMax = config.Beams.BeamZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("pillar") || Name.ToString().ToLower().Contains("smallMiddle"))
	{
		// Pillars
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Pillars.PillarX * -1.f;
		auto xMax = config.Pillars.PillarX;
		auto yMin = config.Pillars.PillarY * -1.f;
		auto yMax = config.Pillars.PillarY;
		auto zMin = config.Pillars.PillarZ * -1.f;
		auto zMax = config.Pillars.PillarZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
	}
	else if (Name.ToString().ToLower().Contains("roof"))
	{
		// Roofs
		ptr->bApplyRandomOffsetOnInstance = false;

		auto config = FNoZFighting_ConfigStruct::GetActiveConfig(OwnerActor->GetWorld());
		auto xMin = config.Roofs.RoofX * -1.f;
		auto xMax = config.Roofs.RoofX;
		auto yMin = config.Roofs.RoofY * -1.f;
		auto yMax = config.Roofs.RoofY;
		auto zMin = config.Roofs.RoofZ * -1.f;
		auto zMax = config.Roofs.RoofZ;
		float xRandomFloat = RandStream.FRandRange(xMin, xMax);
		float yRandomFloat = RandStream.FRandRange(yMin, yMax);
		float zRandomFloat = RandStream.FRandRange(zMin, zMax);
		auto loc = ptr->RelativeTransform.GetLocation();
		loc.X = loc.X + xRandomFloat;
		loc.Y = loc.Y + yRandomFloat;
		loc.Z = loc.Z + zRandomFloat;
		ptr->RelativeTransform.SetLocation(loc);
		}
	return;
}

//#pragma optimize("", on)

IMPLEMENT_GAME_MODULE(FNoZFightingModule, NoZFighting);