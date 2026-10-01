// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/GameMode.h"
#include "hsm/include/hsm.h"
#include "LD35MenuGameMode.generated.h"

/**
*  Represent the game mode, the logic of how to deal with main menu. This class only the server.
*  The game state must be used to share data to clients.
*/

UCLASS(minimalapi)
class ALD35MenuGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALD35MenuGameMode();

	/** AActor overrides */
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	/** AActor overrides end */
	
private:
	void InitHSM();

private:
	friend struct GameModeStates;
	hsm::StateMachine m_StateMachine;
};



