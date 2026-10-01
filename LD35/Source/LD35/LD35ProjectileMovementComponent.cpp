// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35ProjectileMovementComponent.h"

ULD35ProjectileMovementComponent::ULD35ProjectileMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULD35ProjectileMovementComponent::Deflect(AActor* source, FVector position, FVector normal, float boost)
{
	const FVector MoveDelta;
	const FVector OldVelocity = Velocity;

	FHitResult hit(source, nullptr, position, normal);
	Velocity = ComputeBounceResult(hit, 0.0f, MoveDelta);
	OnProjectileBounce.Broadcast(hit, OldVelocity);
}