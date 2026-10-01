// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "LD35ProjectileMovementComponent.generated.h"

UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent), ShowCategories = (Velocity))
class LD35_API ULD35ProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_UCLASS_BODY()

public:

	void Deflect(AActor* source, FVector position, FVector normal, float boost);

};
