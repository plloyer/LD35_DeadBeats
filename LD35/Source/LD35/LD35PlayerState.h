#pragma once

#include "GameFramework/PlayerState.h"
#include "LD35PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LD35_API ALD35PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ALD35Character* Character;

	UPROPERTY(Transient)
	bool bIsGhost = false;

public:
	void Reset();
};
