

#pragma once

#include "GameFramework/Actor.h"
#include "LD35Trap.generated.h"

UCLASS()
class LD35_API ALD35Trap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALD35Trap();

    UFUNCTION()
    void OnOverlapSpikeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    // Number of seconds between the moment the actor steps on the trap and the moment the trap activates
    UPROPERTY(EditAnywhere, Category=LD35Trap)
    float TrapDamageAmount = 2;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
