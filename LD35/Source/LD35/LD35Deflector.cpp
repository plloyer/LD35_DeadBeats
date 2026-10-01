#include "LD35.h"
#include "LD35Deflector.h"
#include "LD35Projectile.h"

void ALD35Deflector::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALD35Deflector, TeamId, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ALD35Deflector, Cooldown, COND_InitialOnly);
}
// Sets default values
ALD35Deflector::ALD35Deflector()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALD35Deflector::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ALD35Deflector::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (HasAuthority())
	{
		Cooldown -= DeltaTime;
		if (Cooldown < 0)
		{
			this->Destroy();
		}
		else
		{
			float outterRadiusSquare = OutterRadius*OutterRadius;
			float innerRadiusSquare = InnerRadius*InnerRadius;
			FVector location = GetOwner()->GetActorLocation();

			for (TActorIterator<ALD35Projectile> ActorItr(GetOwner()->GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35Projectile* projectile = *ActorItr;

				if (projectile->GetTeamId() != TeamId)
				{
					const float distSquare = FVector::DistSquaredXY(projectile->GetActorLocation(), location);
					if (distSquare < outterRadiusSquare && distSquare > innerRadiusSquare)
					{
						//Bam !
						FVector normal = projectile->GetActorLocation() - location;
						//DrawDebugLine(GetWorld(), location, projectile->GetActorLocation(), FColor::Green, false, 3.0f, 0, 2.0f);
						normal.Z = 0.0; // only reflect on XY
						normal.Normalize();
						projectile->Deflect(this, normal, DeflectionBoost);
					}
				}
			}
		}
	}
}

