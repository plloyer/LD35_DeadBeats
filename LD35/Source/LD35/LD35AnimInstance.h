// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimInstance.h"
#include "LD35AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class LD35_API ULD35AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float MoveSpeed;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FVector Velocity;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FRotator DesiredRotation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsDead;

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
};
