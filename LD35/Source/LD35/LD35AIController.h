// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "LD35AIController.generated.h"

/**
 * 
 */
UCLASS()
class LD35_API ALD35AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	// AActor override
	virtual void Possess(APawn* InPawn) override;
	// End AActor override

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetFireFocus(ALD35Character* Target);

	UFUNCTION(BlueprintPure, Category = "Character")
	ALD35Character* GetFireFocus() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetMoveFocus(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Character")
	AActor* GetMoveFocus() const;

	UFUNCTION()
	void ResetFireFocus();

	virtual void Tick(float DeltaTime) override;

public:
	/** Whether or not this NPC should have a player state (only NPC representing player can have one) */
	UPROPERTY(EditAnywhere)
	bool CreatePlayerState = false;
	
private:
	TWeakObjectPtr<class ALD35Character>	FireFocus;

	TWeakObjectPtr<class AActor>	MoveFocus;
};
