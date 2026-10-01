

#pragma once

#include "GameFramework/Actor.h"
#include "LD35SpawnLocation.generated.h"

UCLASS()
class LD35_API ALD35SpawnLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALD35SpawnLocation();

	FTransform GetSpawnTransform() const;

public:
	UPROPERTY(EditAnywhere)
	ETeamEnum OwnerTeam = ETeamEnum::Neutral;

	UPROPERTY(EditAnywhere)
	float SpawnVerticalOffset = 88.0f;
};
