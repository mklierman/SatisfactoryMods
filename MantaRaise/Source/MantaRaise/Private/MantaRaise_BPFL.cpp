#include "MantaRaise_BPFL.h"
#include "FGSplineComponent.h"
#include "Buildables/FGBuildable.h"
#include "FGSplinePath.h"
#include <Kismet/GameplayStatics.h>


/// Loop through each manta
/// Get Spline
/// Get spline points
/// Add extra points along spline
/// Check if any points collide with buildings
/// Add height, recheck x7
/// Add any additional height to original spline points
/// Reload spline

#pragma optimize("", off)
void UMantaRaise_BPFL::RaiseMantas(AActor* actor)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(actor->GetWorld(), AFGManta::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		//Loop through each Manta
		for (auto actor1 : FoundActors)
		{
			auto manta = Cast< AFGManta>(actor1);
			if (manta)
			{
				//Get Spline
				USplineComponent* spline = manta->mSplinePath->mSpline;
				if (spline)
				{
					TArray<FMantaPoint_Struct> PointsStruct;
					TMap<int, float> PointsNewZ;
					//Loop through existing spline points and save to struct array
					for (int i = 0; i < spline->GetNumberOfSplinePoints(); i++)
					{
						FSplinePointData splineData = spline->GetSplinePointData(i, ESplineCoordinateSpace::World);
						auto splinePointLocation = splineData.Location;
						PointsStruct.Add(FMantaPoint_Struct(i, splinePointLocation, 0));

						TArray<FMantaPoint_Struct> newPoints = PointsStruct;
						TArray<FVector> pointLocations;


						//for (auto pointStruct : newPoints)
						//{
						//	if (pointStruct.pointId == i)
						//	{
						//		pointLocations.Add(pointStruct.pointLocation);
						//	}
						//}
						
						//Add current and next point to array so we can add inbetween
						pointLocations.Add(splinePointLocation);
						if (i == spline->GetNumberOfSplinePoints() - 1)
						{
							pointLocations.Add(spline->GetSplinePointData(0, ESplineCoordinateSpace::World).Location);
						}
						else
						{
							pointLocations.Add(spline->GetSplinePointData(i + 1, ESplineCoordinateSpace::World).Location);
						}

						
						//Add new "points" along spline to check collision
						for (int n = 0; n < 4; n++)
						{
							TArray<FVector> points;
							pointLocations = GetNewSplinePoints(i, pointLocations, spline);
						}

						//Check collision and get highest point
						double newZ = GetRaisedZ(pointLocations, manta, i);
						PointsNewZ.Add(i, newZ);

						FVector newLocation = spline->GetSplinePointData(i, ESplineCoordinateSpace::World).Location;
						newLocation.Z = PointsNewZ[i];
						spline->SetLocationAtSplinePoint(i, newLocation, ESplineCoordinateSpace::World, false);
						//spline->AddSplinePoint(point, ESplineCoordinateSpace::World);
						pointLocations.Empty();
					}

					//Loop through spline points again
					for (int i = 0; i < spline->GetNumberOfSplinePoints(); i++)
					{

					}
					spline->UpdateSpline();
				}
			}
		}
	}
}

TArray<FVector> UMantaRaise_BPFL::GetNewSplinePoints(int currentPointIndex, TArray<FVector> existingPoints, USplineComponent* spline)
{
	TArray<FVector> newPoints = existingPoints;
	for (int i = 0; i < existingPoints.Num(); i++)
	{
		FVector firstPoint = existingPoints[i];
		FVector secondPoint;
		if (i == existingPoints.Num()-1)
		{
			return newPoints;
		}
		else
		{
			secondPoint = existingPoints[i + 1];
		}
		firstPoint = firstPoint / 2;
		secondPoint = secondPoint / 2;
		FVector newPoint = firstPoint + secondPoint;
		newPoints.AddUnique(newPoint);
	}
	return newPoints;
}

double UMantaRaise_BPFL::GetRaisedZ(TArray<FVector> points, AActor* actor, int currentIdx)
{
	double newZ = 0.0;
	TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes;
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	traceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	TArray<AActor*> ignoreActors;
	TArray<AActor*> outActors;
	for (int i = 0; i < points.Num(); i++)
	{
		bool needsCheck = true;
		FVector checkLocation = points[i];
		
		while (needsCheck)
		{
			needsCheck = UKismetSystemLibrary::SphereOverlapActors(actor->GetWorld(), checkLocation, 3500, traceObjectTypes, AFGBuildable::StaticClass(), ignoreActors, outActors);
			if (needsCheck)
			{
				checkLocation.Z = checkLocation.Z + 1000.0;
			}
			outActors.Empty();
		}
		if (checkLocation.Z > newZ)
		{
			newZ = checkLocation.Z;
		}
	}
	return newZ;
}
#pragma optimize("", on)
