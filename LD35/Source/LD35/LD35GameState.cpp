// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35GameState.h"
#include "LD35Character.h"
#include "GameFramework/PlayerState.h"

ALD35GameState::ALD35GameState()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	TeamList.AddZeroed(TEAM_COUNT);
}

void ALD35GameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALD35GameState, MyGameState);
	DOREPLIFETIME(ALD35GameState, TeamList);
}

float ALD35GameState::GetNormalizedScore(ETeamEnum Team) const
{
	if (const FTeamState* state = GetTeamState(Team))
		return state->Score / ScoreToWin;

	return 0.0f;
}

ETeamEnum ALD35GameState::GetWinnerTeam() const
{
	ETeamEnum onlyTeamAlive = ETeamEnum::Neutral;
	int teamAlive = 0;
	for (int i = 0; i < TeamList.Num(); ++i)
	{
		const FTeamState& teamState = TeamList[i];
		ETeamEnum currentTeam = static_cast<ETeamEnum>(i);
		if (teamState.Score >= ScoreToWin)
		{
			return currentTeam;
		}

		if (!teamState.IsDeadForTheGame && teamState.PlayerState != nullptr)
		{
			++teamAlive;
			onlyTeamAlive = currentTeam;
		}
	}

	return teamAlive == 1 ? onlyTeamAlive : ETeamEnum::Neutral;
}

const FTeamState* ALD35GameState::GetTeamState(ETeamEnum Team) const
{
	if (Team < ETeamEnum::TeamCount)
		return &TeamList[static_cast<int>(Team)];

	return nullptr;
}

FTeamState* ALD35GameState::GetTeamState(ETeamEnum Team)
{
	if (Team < ETeamEnum::TeamCount)
		return &TeamList[static_cast<int>(Team)];

	return nullptr;
}

void ALD35GameState::SetTeamDeadForTheGame(ETeamEnum Team, bool Dead)
{
	if (FTeamState* state = GetTeamState(Team))
	{
		state->IsDeadForTheGame = Dead;
	}
}

void ALD35GameState::SetTeamPlayerState(ETeamEnum Team, ALD35PlayerState* PlayerState)
{
	if (FTeamState* state = GetTeamState(Team))
	{
		state->PlayerState = PlayerState;
	}
}

bool ALD35GameState::IsDeadForTheGame(ETeamEnum Team) const
{
	if (const FTeamState* state = GetTeamState(Team))
	{
		return state->IsDeadForTheGame;
	}

	return true;
}

void ALD35GameState::ResetAllTeamState()
{
	for (auto& state : TeamList)
	{
		state.Reset();
	}
}