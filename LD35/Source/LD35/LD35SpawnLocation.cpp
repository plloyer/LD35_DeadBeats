

#include "LD35.h"
#include "LD35SpawnLocation.h"


ALD35SpawnLocation::ALD35SpawnLocation()
{
	PrimaryActorTick.bCanEverTick = false;

}

FTransform ALD35SpawnLocation::GetSpawnTransform() const
{
	const float upOffset = 10.0f;
	const FVector start = GetActorLocation() + FVector::UpVector * upOffset;
	const FVector end = start - FVector::UpVector * (3 * upOffset);
	FVector location;

	FCollisionQueryParams TraceParams(TEXT("ALD35SpawnLocation"), false, this);
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, start, end, ECC_WorldStatic, TraceParams);
	if (bHit)
	{
		location = Hit.ImpactPoint + FVector::UpVector * SpawnVerticalOffset;
	}
	else
	{
		LD35_LOG(LogLD35GameMode, Log, "Could not find start location for ALD35SpawnLocation %s", *GetName());
		location = GetActorLocation() + FVector::UpVector * SpawnVerticalOffset;
	}

	FTransform transfo = GetActorTransform();
	transfo.SetLocation(location);
	return transfo;
}

