// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35Projectile.h"
#include "LD35Character.h"
#include "LD35ProjectileMovementComponent.h"

void ALD35Projectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ALD35Projectile, m_TeamId, COND_InitialOnly);
}

ALD35Projectile::ALD35Projectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ALD35Projectile::BeginPlay()
{
	Super::BeginPlay();

	m_ProjectileMovementComponent = FindComponentByClass<UProjectileMovementComponent>();

	UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(GetRootComponent());
	if (HasAuthority() && PrimComponent)
	{
		PrimComponent->IgnoreActorWhenMoving(GetInstigator(), true);
		//PrimComponent->BodyInstance.SetCollisionProfileName("Projectile");
		PrimComponent->OnComponentBeginOverlap.AddDynamic(this, &ALD35Projectile::OnComponentBeginOverlap);
	}
}

void ALD35Projectile::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	LD35_LOG(LogLD35Projectile, Log, "OnHit: Actor: %s", IsValid(OtherActor) ? *OtherActor->GetName() : *FString("Not actor"));

	if (HasAuthority())
	{
		ALD35Character* character = Cast<ALD35Character>(OtherActor);
		ALD35Projectile* projectile = Cast<ALD35Projectile>(OtherActor);

		// Character - Projectile collision
		if (IsValid(character))
		{
			const bool sameTeam = m_TeamId == character->GetTeamId();
			if (!sameTeam)
			{
				float health = character->GetCurrentHealth();
				character->Damage(DamageAmount);
				DamageAmount -= health;
				MulticastProjectileHit(OtherActor);

				if (DamageAmount <= 0)
				{
					Destroy();
				}
			}
		}
		// Projectile - Projectile collision
		else if (IsValid(projectile))
		{
			const bool sameTeam = m_TeamId == projectile->GetTeamId();
			if (!sameTeam)
			{
				if (projectile->Strength > Strength)
				{
					Destroy();
				}
				else if (projectile->Strength < Strength)
				{
					projectile->Destroy();
				}
				else
				{
					int damage = FMath::Min(DamageAmount, projectile->DamageAmount);
					DamageAmount -= damage;
					projectile->DamageAmount -= damage;

					if (DamageAmount <= 0)
					{
						Destroy();
					}
					else
					{
						MulticastProjectileHit(OtherActor);
					}
					if (projectile->DamageAmount <= 0)
					{
						projectile->Destroy();
					}
				}
			}
		}
		// Any other collisions
		else
		{
			Destroy();
		}
	}
}

void ALD35Projectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (DebugActor)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 30, FColor::Red);
	}

	AActor* instigator = GetInstigator();
	if (instigator && DebugActor)
	{
		DrawDebugLine(GetWorld(), instigator->GetActorLocation(), GetActorLocation(), FColor::Red, false, -1.0f, 0, 4.0f);
	}
}

void ALD35Projectile::Deflect(AActor* source, FVector normal, float boost)
{
	if (LastDeflector == source) return; // Don't try to bounce multiple time on the same source actor
	
	ULD35ProjectileMovementComponent* movementComponent = Cast<ULD35ProjectileMovementComponent>(m_ProjectileMovementComponent);
	if (movementComponent)
		movementComponent->Deflect(source, GetActorLocation(), normal, boost);

	LastDeflector = source;
	++DeflectionCount;

	m_TeamId = ETeamEnum::TeamCount;

	UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(GetRootComponent());
	if (IsValid(PrimComponent))
	{
		PrimComponent->IgnoreActorWhenMoving(GetInstigator(), false);
	}
}

void ALD35Projectile::SetTeamId(ETeamEnum Team)
{
	m_TeamId = Team;
	OnRep_TeamId();
}

void ALD35Projectile::OnRep_TeamId()
{
	OnTeamIdChanged.Broadcast();
}

void ALD35Projectile::MulticastProjectileHit_Implementation(AActor* OtherActor)
{
	OnProjectileHit.Broadcast(OtherActor);
}

