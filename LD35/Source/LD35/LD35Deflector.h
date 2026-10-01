#pragma once

#include "GameFramework/Actor.h"
#include "Teams.h"
#include "LD35Deflector.generated.h"

UCLASS()
class LD35_API ALD35Deflector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALD35Deflector();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	ETeamEnum TeamId = ETeamEnum::Neutral;
	
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = Deflection)
	float Cooldown = 0.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Deflection)
	float OutterRadius = 150.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Deflection)
	float InnerRadius = 10.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Deflection)
	float DeflectionBoost = 1.0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Deflection)
	bool DebugCircle = false;
	
};
