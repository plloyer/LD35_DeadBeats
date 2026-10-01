// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35AIController.h"
#include "LD35Character.h"
#include "GameInputComponent.h"


void ALD35AIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	if (GetCharacter())
	{
		UGameInputComponent* gameInput = GetCharacter()->FindComponentByClass<UGameInputComponent>();
		check(gameInput != nullptr);
		gameInput->SetConsumeInput(true);
	}
}

void ALD35AIController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	bWantsPlayerState = CreatePlayerState;
}

void ALD35AIController::SetFireFocus(ALD35Character* Target)
{
	if (Target == FireFocus)
		return;

	if (FireFocus.IsValid())
	{
		FireFocus->DiedDelegate.RemoveDynamic(this, &ALD35AIController::ResetFireFocus);
	}

	FireFocus = Target;

	if (FireFocus.IsValid())
	{
		FireFocus->DiedDelegate.AddDynamic(this, &ALD35AIController::ResetFireFocus);
	}
}

void ALD35AIController::ResetFireFocus()
{
	SetFireFocus(nullptr);
}

ALD35Character* ALD35AIController::GetFireFocus() const
{
	if (FireFocus.IsValid())
	{
		return FireFocus.Get();
	}
	return nullptr;
}

void ALD35AIController::SetMoveFocus(AActor* Target)
{
	MoveFocus = Target;
}

AActor* ALD35AIController::GetMoveFocus() const
{
	if (MoveFocus.IsValid())
	{
		return MoveFocus.Get();
	}
	return nullptr;
}

void ALD35AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


