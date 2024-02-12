#include "CSB_MarkerActor.h"

ACSB_MarkerActor::ACSB_MarkerActor()
{
}

bool ACSB_MarkerActor::AddAsRepresentation()
{
    return false;
}

bool ACSB_MarkerActor::UpdateRepresentation()
{
    return false;
}

bool ACSB_MarkerActor::RemoveAsRepresentation()
{
    return false;
}

bool ACSB_MarkerActor::IsActorStatic()
{
    return false;
}

FVector ACSB_MarkerActor::GetRealActorLocation()
{
    return FVector();
}

FRotator ACSB_MarkerActor::GetRealActorRotation()
{
    return FRotator();
}

UTexture2D* ACSB_MarkerActor::GetActorRepresentationTexture()
{
    return nullptr;
}

FText ACSB_MarkerActor::GetActorRepresentationText()
{
    return FText();
}

void ACSB_MarkerActor::SetActorRepresentationText(const FText& newText)
{
}

FLinearColor ACSB_MarkerActor::GetActorRepresentationColor()
{
    return FLinearColor();
}

void ACSB_MarkerActor::SetActorRepresentationColor(FLinearColor newColor)
{
}

ERepresentationType ACSB_MarkerActor::GetActorRepresentationType()
{
    return ERepresentationType();
}

bool ACSB_MarkerActor::GetActorShouldShowInCompass()
{
    return false;
}

bool ACSB_MarkerActor::GetActorShouldShowOnMap()
{
    return false;
}

EFogOfWarRevealType ACSB_MarkerActor::GetActorFogOfWarRevealType()
{
    return EFogOfWarRevealType();
}

float ACSB_MarkerActor::GetActorFogOfWarRevealRadius()
{
    return 0.0f;
}

ECompassViewDistance ACSB_MarkerActor::GetActorCompassViewDistance()
{
    return ECompassViewDistance();
}

void ACSB_MarkerActor::SetActorCompassViewDistance(ECompassViewDistance compassViewDistance)
{
}
