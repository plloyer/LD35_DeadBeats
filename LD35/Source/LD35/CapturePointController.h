// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "LD35Character.h"
#include "Teams.h"
#include "hsm/include/hsm.h"

#include "CapturePointController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCaptureDoneDelegate, bool, Silent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCaptureStartDelegate, ETeamEnum, CapturingTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCapturePausedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureResumedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCaptureCancelledDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnDelegate);

typedef TArray<TWeakObjectPtr<ALD35Character>> CharacterList;

UENUM()
enum class ECapturePointState : uint8
{
	NotCaptured,
	Capturing,
	Contested,
	Captured
};

USTRUCT()
struct FCapturePointShared
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	bool Enabled = false;

	UPROPERTY(Transient)
	ECapturePointState State = ECapturePointState::NotCaptured;
	
	UPROPERTY(EditAnywhere)
	ETeamEnum OwnerTeam = ETeamEnum::Neutral;

	UPROPERTY(Transient)
	ETeamEnum CapturingTeam = ETeamEnum::Neutral;

	UPROPERTY(Transient)
	uint32 TeamCountInTrigger;

	UPROPERTY(Transient)
	ETeamEnum TeamInTrigger;
};

UCLASS()
class LD35_API ACapturePointController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACapturePointController();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/// Begin UObject Interface
	virtual void PostLoad() override;
	/// End UObject Interface

	/// Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostActorCreated() override;
	/// End AActor Interface

	void EnableCapturePoint(bool Value);

	ETeamEnum GetUncontestedOwner() const;

	UFUNCTION(BlueprintPure, Category = "Capture")
	bool IsEnabled() const { return m_Replicated.Enabled; }

	UFUNCTION(BlueprintPure, Category = "Capture")
	ETeamEnum GetOwnerTeam() const { return m_Replicated.OwnerTeam; }

	UFUNCTION()
	void SetOwnerTeam(ETeamEnum Team) { m_Replicated.OwnerTeam = Team; }

	UFUNCTION(BlueprintPure, Category = "Capture")
	FColor GetOwnerColor() const;

	UFUNCTION(BlueprintPure, Category = "Capture")
	FColor GetCapturingColor() const;

	UFUNCTION(BlueprintPure, Category = "Capture")
	float GetCapturePercent() const { return CapturePercent; }

	UFUNCTION()
	uint32 GetTeamCountInTrigger() const { return m_Replicated.TeamCountInTrigger; }

	UFUNCTION()
	ETeamEnum GetTeamInTrigger() const { return m_Replicated.TeamInTrigger; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
	FTransform RespawnTransform;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Capture")
	float CapturePercent = 0.0f;
	
	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FOnCaptureStartDelegate OnCaptureStart;
	
	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FOnCapturePausedDelegate OnCapturePaused;

	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FOnCaptureResumedDelegate OnCaptureResumed;

	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FOnCaptureCancelledDelegate OnCaptureCancelled;
	
	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FOnCaptureDoneDelegate OnCaptureDone;
	
	UPROPERTY(BlueprintAssignable, Category = "Capture")
	FSpawnDelegate SpawnDeletage;

protected:
	UFUNCTION()
	void ServerOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void ServerOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void InitHSM();

	void SetupCollisionCylinder();
	void ServerRemoveInvalidCharacters();

	FColor GetTeamColor(ETeamEnum Team) const;
	
private:
	friend struct CapturePointStates;
	hsm::StateMachine m_StateMachine;

	USphereComponent* m_SphereComponent;

	CharacterList m_CharactersInTrigger;
	
	UPROPERTY(EditAnywhere, Category = "Capture")
	float CaptureRadius = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Capture")
	float CaptureTime = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Capture")
	float TimeBetweenSpawn = 15.0f;

	UPROPERTY(Transient)
	float m_CaptureTimeLeft = 0.0f;

	UPROPERTY(Transient)
	bool m_ServerCanGivePoints = false;

	UPROPERTY(Replicated, EditAnywhere, Category = "Capture")
	FCapturePointShared m_Replicated;
};
