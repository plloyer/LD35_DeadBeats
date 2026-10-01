// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/GameMode.h"
#include "hsm/include/hsm.h"
#include "LD35GameModeArena.generated.h"

/**
*  Represent the game mode, the logic of how to play the game. This class only the server.
*  The game state must be used to share data to clients.
*/

UCLASS(minimalapi)
class ALD35GameModeArena : public AGameMode
{
	GENERATED_BODY()

public:
	ALD35GameModeArena();

	/** AActor overrides */
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	/** AActor overrides end */

	/** AGameMode overrides */
	virtual void PostLogin(APlayerController* NewPlayer) override;
	/** AGameMode overrides end */

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnAIMage(FTransform Transform, ETeamEnum Team);

public:
	/** The main character class used by the PlayerController for players when playing. */
	UPROPERTY(EditAnywhere, noclear, BlueprintReadOnly, Category = LD35GameMode)
	TSubclassOf<class ALD35Character> MainCharacterClass;
	
	/** The main character class used by the PlayerController for players when playing. */
	UPROPERTY(EditAnywhere, noclear, BlueprintReadOnly, Category = LD35GameMode)
	TSubclassOf<class ALD35Character> MainCharacterGhostClass;

private:
	void InitHSM();
	
private:
	friend struct GameModeStatesArena;
	hsm::StateMachine m_StateMachine;

	/** List of data per team */
	TArray<ALD35Character*> m_TeamMages;

	/** List of data per team */
	TArray<class ALD35PlayerController*> m_PlayerToSpawn;

	UPROPERTY(EditAnywhere, Category = "LD35GameMode")
	float TimeBeforeRespawn = 4.0f;
};



