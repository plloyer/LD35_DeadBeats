// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "GameInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class LD35_API UGameInputComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	void Reset();
	void SetConsumeInput(bool Value) { m_ConsumeInputs = Value; }

	const FVector& GetMovement() const { return m_Movement; }
	//const FVector& GetLookDirection() const { return m_LookDirection; }
	float GetYaw() const { return m_Yaw; }
	bool WantsToShoot(bool consume);
	bool WantsToPowerShoot(bool consume);
	bool WantsToDeflect();
	bool WantsToAllyFollow() { return m_Follow; }
	bool WantsToAllyReturn() { return m_Return; }
	bool WantsToStartGame();

	void SetMovementHorizontal(float Value) { m_Movement.X = Value; }
	void SetMovementVertical(float Value) { m_Movement.Y =  Value; }
	void SetMovementDirection(FVector Direction) { m_Movement = Direction; }
	void SetYaw(float Value) { m_Yaw = Value; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetWantsToShoot(bool Value) { m_WantsToShoot = Value; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetWantsToPowerShoot(bool Value) { m_WantsToPowerShoot = Value; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetWantsToDeflect(bool Value) { m_WantsToDeflect = Value; }

	void SetWantsToAllyFollow(bool Value) { m_Follow = Value; }
	void SetWantsToAllyReturn(bool Value) { m_Return = Value; }
	void SetWantsToStartGame(bool value) { m_StartGame = value; }

private:
	FVector m_Movement = FVector::ZeroVector;
	//FVector m_LookDirection = FVector::ZeroVector;
	float m_Yaw = 0.0f;
	bool m_WantsToShoot = false;
	bool m_WantsToPowerShoot = false;
	bool m_WantsToDeflect = false;
	bool m_Follow = false;
	bool m_Return = false;
	bool m_StartGame = false;

	bool m_ConsumeInputs = false;
};
