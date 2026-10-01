// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35AnimInstance.h"
#include "LD35Character.h"

PRAGMA_DISABLE_OPTIMIZATION

void ULD35AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	const auto PawnOwner = TryGetPawnOwner();
	const auto CharacterOwner = Cast<ALD35Character>(PawnOwner);
	if (IsValid(CharacterOwner))
	{
		Velocity = CharacterOwner->GetVelocity();
		MoveSpeed = Velocity.Size2D();
		DesiredRotation = Velocity.Rotation();
		DeltaRotation = CharacterOwner->GetCharacterMovement()->GetDeltaRotation(DeltaSeconds);
		bIsDead = CharacterOwner->IsDead();
	}
}

PRAGMA_ENABLE_OPTIMIZATION

