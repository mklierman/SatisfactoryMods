// ILikeBanas

#include "UniGrid_BPLibrary.h"
#include "UnifiedGridModule.h"
#include "Subsystem/SubsystemActorManager.h"


AActor* UUniGrid_BPLibrary::SpawnAndBuildFromRecipe(AFGBuildableSubsystem* buildableSubsystem, TSubclassOf<class UFGRecipe> inRecipe, AActor* hologramOwner,FTransform nodeTransform, FVector nodeLocation, AActor* primaryBuilding, AActor* node, float alignmentHeight, float extraHeight)
{
	//auto unrotatedVector = primaryBuilding->GetActorLocation();
	//
	//auto primaryRotation = primaryBuilding->GetActorRotation(); 


	 long double GRID_SIZE = 800.0;
	 long double HEIGHT_SIZE = StaticCast<long double>(alignmentHeight);
	 long double EXTRA_HEIGHT = StaticCast<long double>(extraHeight);


	 //FVector RelativeOffset = primaryBuilding->GetActorRotation().UnrotateVector(node->GetActorLocation() - primaryBuilding->GetActorLocation());
	 //FVector SnappedOffset = FVector(round(RelativeOffset.X / GRID_SIZE) * GRID_SIZE, round(RelativeOffset.Y / GRID_SIZE) * GRID_SIZE, (round(RelativeOffset.Z / 200) * 200)) + 200;
	 //FVector FinalLocation = primaryBuilding->GetActorLocation() + primaryBuilding->GetActorRotation().RotateVector(SnappedOffset);
	// 
	//UE_LOG(UnifiedGrid_Log, Display, TEXT("Relative Offset: %f, %f, %f"), RelativeOffset.X, RelativeOffset.Y, RelativeOffset.Z);
	//UE_LOG(UnifiedGrid_Log, Display, TEXT("Snapped Offset: %f, %f, %f"), SnappedOffset.X, SnappedOffset.Y, SnappedOffset.Z);
	//UE_LOG(UnifiedGrid_Log, Display, TEXT("Final Location: %f, %f, %f"), FinalLocation.X, FinalLocation.Y, FinalLocation.Z);
	 long double primaryX = primaryBuilding->GetActorLocation().X;
	 long double primaryY = primaryBuilding->GetActorLocation().Y;
	 long double primaryZ = primaryBuilding->GetActorLocation().Z;
	 long double secondaryX = node->GetActorLocation().X;
	 long double secondaryY = node->GetActorLocation().Y;
	 long double secondaryZ = node->GetActorLocation().Z + EXTRA_HEIGHT;


	// UE_LOG(UnifiedGrid_Log, Display, TEXT("Primary Location: %f, %f, %f"), primaryLocation.X, primaryLocation.Y, primaryLocation.Z);
	// UE_LOG(UnifiedGrid_Log, Display, TEXT("Primary Rotation: %f, %f, %f"), primaryRotation.Pitch, primaryRotation.Roll, primaryRotation.Yaw);
	long double newX = primaryX - (roundl((primaryX - secondaryX) / GRID_SIZE) * GRID_SIZE);
	long double newY = primaryY - (roundl((primaryY - secondaryY) / GRID_SIZE) * GRID_SIZE);
	long double newZ = (primaryZ - (roundl((primaryZ - secondaryZ) / HEIGHT_SIZE) * HEIGHT_SIZE));

	if (newZ < secondaryZ)
	{
		newZ += HEIGHT_SIZE;
	}

	//newZ += EXTRA_HEIGHT;

	//newX = newX - xyOffset.X;
	//newY = newY - xyOffset.Y;

	//newX = newX - (roundf((newX - secondaryX) / GRID_SIZE) * GRID_SIZE);
	//newY = newY - (roundf((newY - secondaryY) / GRID_SIZE) * GRID_SIZE);


	 //UE_LOG(UnifiedGrid_Log, Display, TEXT("Outpost Location: %f, %f, %f"), newX, newY, newZ);
	// UE_LOG(UnifiedGrid_Log, Display, TEXT("Outpost Rotation: %f, %f, %f"), transformedRotation.Quaternion().X, transformedRotation.Quaternion().Y, transformedRotation.Quaternion().Z);
	 //FTransform spawnTransform;
	 //spawnTransform.SetRotation(primaryBuilding->GetActorRotation().Quaternion());
	auto spawnLocation = FVector(newX, newY, newZ);
	
	//auto rotatedLocation = spawnLocation.ForwardVector.RotateAngleAxis(primaryBuilding->GetActorRotation().Quaternion().Z, spawnLocation.UpVector);
	//spawnLocation.X = rotatedLocation.X;
	//spawnLocation.Y = rotatedLocation.Y;
	//spawnTransform.SetLocation(FVector(newX, newY, newZ));
	//auto spawnLocation = spawnTransform.GetLocation();
	auto Hologram = AFGHologram::SpawnHologramFromRecipe(inRecipe, hologramOwner, spawnLocation);
	//Hologram->SetActorRotation(primaryBuilding->GetActorRotation());
	//Hologram->AddActorLocalRotation(primaryBuilding->GetActorRotation());
	//Hologram->SetActorLocationAndRotation(spawnLocation, primaryBuilding->GetActorRotation());
	//Hologram->SetActorTransform(nodeTransform);
	TArray<AActor*> Children;
	auto constID = buildableSubsystem->GetNewNetConstructionID();
	auto Actor = Hologram->Construct(Children, constID);
	Hologram->Destroy();
	//Actor->SetActorRotation(primaryBuilding->GetActorRotation());
	//Actor->SetActorLocation(spawnLocation);
	//primaryBuilding->SetActorRotation(primaryBuilding->GetActorRotation());
	return Actor;
}
