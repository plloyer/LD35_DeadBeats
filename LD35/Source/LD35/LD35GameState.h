#pragma once

#include "GameFramework/GameState.h"
#include "LD35GameState.generated.h"

UENUM(BlueprintType)
enum class ELD35State : uint8
{
	Menu,
	Spawning,
	Playing,
	EndMatch
};

UENUM(BlueprintType)
enum class EGameType : uint8
{
	None,
	Arena,
	Cpature
};

/**
*  Structure that holds the data for a team during a game.
*/
USTRUCT()
struct FTeamState
{
	GENERATED_USTRUCT_BODY()
	FTeamState()
	{ 
		Reset();
	}

	void Reset()
	{
		IsDeadForTheGame = false;
		Score = 0.0f;
	}

	/** This can be null if the team lost */
	UPROPERTY()
	class ALD35PlayerState* PlayerState = nullptr;

	/** Tells if the current team is out of the game */
	UPROPERTY()
	bool IsDeadForTheGame;

	/** Score of the team for this game */
	UPROPERTY()
	float Score;
};

/**
 *  Represent the game state, the data for the game. This class is on every clients and on the server.
 */
UCLASS()
class LD35_API ALD35GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ALD35GameState();

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	bool IsInMenu() const { return MyGameState == ELD35State::Menu; }

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	bool IsSpawning() const { return MyGameState == ELD35State::Spawning; }

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	bool IsPlaying() const { return MyGameState == ELD35State::Playing; }

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	bool IsEndOfMatch() const { return MyGameState == ELD35State::EndMatch; }

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	float GetNormalizedScore(ETeamEnum Team) const;

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	ETeamEnum GetWinnerTeam() const;

	UFUNCTION(BlueprintPure, Category = "LD35GameState")
	bool IsDeadForTheGame(ETeamEnum Team) const;

	const FTeamState* GetTeamState(ETeamEnum Team) const;

	FTeamState* GetTeamState(ETeamEnum Team);

	void SetTeamPlayerState(ETeamEnum Team, ALD35PlayerState* PlayerState);
	void SetTeamDeadForTheGame(ETeamEnum Team, bool Dead);

	void ResetAllTeamState();

public:
	/** Current state of the game */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "LD35GameState")
	ELD35State MyGameState = ELD35State::Menu;

	/** List of all the teams */
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "LD35GameState")
	TArray<FTeamState> TeamList;

	/** Score to win the game */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LD35GameState")
	float ScoreToWin = 100.0f;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "LD35GameState")
	EGameType GameType = EGameType::None;
};
