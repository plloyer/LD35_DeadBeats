// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Teams.h"
#include "LD35Projectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProjectileTeamIdChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHit, AActor*, OtherActor);

UCLASS()
class LD35_API ALD35Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ALD35Projectile();

	/** AActor overrides */
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void Deflect(AActor* source, FVector normal, float boost);

	UFUNCTION(BlueprintPure)
	ETeamEnum GetTeamId() const { return m_TeamId; }

	void SetTeamId(ETeamEnum Team);

public:
	UPROPERTY(EditAnywhere, Category=LD35Projectile)
	int DamageAmount = 1;

	/** Higher means that it will destroy lower strenght projectile on collision */
	UPROPERTY(EditAnywhere, Category = LD35Projectile)
	int Strength = 0;

	UPROPERTY(EditDefaultsOnly, Category = LD35Projectile)
	bool DebugActor = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	int DeflectionCount = 0;

	UPROPERTY(EditAnywhere, Category = LD35Projectile)
	bool CollideWithTeam = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LD35Projectile)
	float AOERadius = 0.0f;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "LD35Projectile")
	FOnProjectileTeamIdChanged OnTeamIdChanged;

	UPROPERTY(BlueprintAssignable, Category = "LD35Projectile")
	FOnProjectileHit OnProjectileHit;
	/** Events end*/

	/** RPC */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastProjectileHit(AActor* OtherActor);
	/** RPC end*/

private:
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);


	UFUNCTION()
	void OnRep_TeamId();

private:
	class UProjectileMovementComponent* m_ProjectileMovementComponent = nullptr;
	AActor* LastDeflector = nullptr;


	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamId)
	ETeamEnum m_TeamId = ETeamEnum::Neutral;
};
