// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Teams.h"
#include "LD35Types.h"
#include "hsm/include/hsm.h"
#include "LD35Character.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeamIdChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPowerShootStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterRespawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathPostProcessUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraTypeChanged, ECameraType, CameraType);

class UWorld;

UCLASS()
class LD35_API ALD35Character : public ACharacter
{
	GENERATED_UCLASS_BODY()

public:
	/** UObject override */
	virtual void PostInitProperties() override;

	/** AActor overrides */
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	/** AActor overrides end */

	/** APawn override */
	virtual void PossessedBy(AController* NewController) override;
	/** APawn overrides end */
	
	UFUNCTION(BlueprintPure, Category =  "LD35Character")
	float GetCurrentHealth() { return m_RepCurrentHealth; }

	UFUNCTION(BlueprintPure, Category =  "LD35Character")
	float GetMaxHealth() { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "LD35Character")
	float GetNormalizedMana() const { return ManaCount / ManaMax; }

	UFUNCTION(BlueprintPure, Category =  "LD35Character")
	bool IsDead() const { return m_RepCurrentHealth <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "LD35Character")
	bool IsGhost() const;

	UFUNCTION(BlueprintCallable, Category =  "LD35Character")
	void Damage(int DamageAmount);

	UFUNCTION(BlueprintPure, Category = "LD35Character")
	ETeamEnum GetTeamId() const { return m_TeamId; }

	UFUNCTION(BlueprintCallable, Category = "LD35Character")
	void SetTeamId(ETeamEnum Team);

	UFUNCTION(BlueprintCallable, Category =  "LD35Character")
	void SpawnProjectle();

	UFUNCTION(BlueprintCallable, Category =  "LD35Character")
	void SpawnPowerProjectle();

	UFUNCTION(BlueprintCallable, Category =  "LD35Character")
	void SpawnDeflect();

	float GetDeathTimer() const { return m_DeathTimer; }

	bool IsAliveAndPlaying() const;

	void Respawn(const FTransform& transform);

	void ResetHealth();

	void ResetMana();

public:
	
	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnPowerShootStatus OnPowerShootReady;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnPowerShootStatus OnPowerShootStartCharging;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnPowerShootStatus OnPowerShootStopCharging;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnCharacterRespawned OnRespawned;

	UPROPERTY(BlueprintReadWrite)
	class UGameInputComponent* GameInputComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	float MaxHealth = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LD35Mana")
	float ManaCount = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LD35Mana")
	float ManaMax = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LD35Mana")
	float ManaRegenRate = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LD35Mana")
	float ManaShootCost = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LD35Mana")
	float ManaPowerShootCost = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LD35Mana")
	float ManaDeflectCost = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	TSubclassOf<class ALD35Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	TSubclassOf<class ALD35Projectile> PowerProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	TSubclassOf<class ALD35Deflector> DeflectorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	FVector WeaponDefaultOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	float ShootCooldown = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	float DeflectCooldown = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "LD35Character")
	float ChargeAttackTime = 2.0;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite, Category = "LD35Character")
	int32 ShootCount = 0;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite, Category = "LD35Character")
	int32 PowerShootCount = 0;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite, Category = "LD35Character")
	int32 DeflectCount = 0;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnTeamIdChanged OnTeamIdChanged;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnDiedDelegate DiedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnDeathPostProcessUpdate OnDeathPostProcessUpdate;

	UPROPERTY(BlueprintAssignable, Category = "LD35Character")
	FOnCameraTypeChanged OnCameraTypeChanged;
	/** Events end */

	UPROPERTY(BlueprintReadOnly, Category = "LD35Character")
	float DeathPostProcessVisibility = 0.0f;
		
private:
	void InitHSM();

	bool CanDoAction() const;
	bool CanShoot() const;
	bool CanPowerShoot() const;
	bool CanDeflect() const;
	void SpawnProjectle(UClass* projectileClass, const FVector& Position, const FRotator& Rotation);

	FVector GetShootLocation() const;

	// RPCs
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerShoot();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerPowerShoot();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerDeflect();

	// OnRep
	UFUNCTION()
	void OnRep_TeamId();

	UFUNCTION()
	void OnRep_PlayerState();
	
private:
	friend struct CharacterStates;
	hsm::StateMachine m_StateMachine;

	UPROPERTY(ReplicatedUsing = OnRep_TeamId, EditAnywhere, Category = "LD35Character")
	ETeamEnum m_TeamId = ETeamEnum::Team1;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerState, EditAnywhere, Category = "LD35Character")
	class ALD35PlayerState* m_PlayerState;

	UPROPERTY(Replicated, Transient)
	float m_RepCurrentHealth = 0;

	float m_ActionCooldownTimer;
	float m_DeathTimer = 0.0f;

	float mDeathPostProcessVisibilityTarget;
	float mDeathPostProcessVisibilityTransitionTime;
};
