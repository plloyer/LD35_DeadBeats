

#include "LD35.h"
#include "LD35Character.h"
#include "LD35Trap.h"


// Sets default values
ALD35Trap::ALD35Trap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALD35Trap::BeginPlay()
{
	Super::BeginPlay();
    
    UPrimitiveComponent* pPrimComponent = Cast<UPrimitiveComponent>(GetRootComponent());
	UBoxComponent* spikes = FindComponentByClass<UBoxComponent>();
    
    if (HasAuthority())
    {
		if (pPrimComponent)
		{
			pPrimComponent->bGenerateOverlapEvents = true;
		}

		if (spikes)
		{
			spikes->bGenerateOverlapEvents = true;
			spikes->OnComponentBeginOverlap.AddDynamic(this, &ALD35Trap::OnOverlapSpikeBegin);
		}

    }
}

// Called every frame
void ALD35Trap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALD35Trap::OnOverlapSpikeBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		ALD35Character* character = Cast<ALD35Character>(OtherActor);
		if (IsValid(character))
		{
			character->Damage(TrapDamageAmount);
		}
	}
}

