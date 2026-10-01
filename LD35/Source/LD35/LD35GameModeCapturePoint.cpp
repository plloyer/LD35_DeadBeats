// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "LD35.h"
#include "LD35Character.h"
#include "LD35GameInstance.h"
#include "LD35GameModeCapturePoint.h"
#include "LD35PlayerController.h"
#include "LD35PlayerState.h"
#include "CapturePointController.h"

using namespace hsm;

ALD35GameModeCapturePoint::ALD35GameModeCapturePoint()
{
	PrimaryActorTick.bCanEverTick = true;

	m_TeamMages.AddZeroed(static_cast<int>(ETeamEnum::TeamCount));
}

void ALD35GameModeCapturePoint::BeginPlay()
{
	Super::BeginPlay();

	ULD35Helper::GetGameState(GetWorld()).GameType = EGameType::Cpature;

	InitHSM();

	GetLD35GameInstance()->SetApplicationState(EApplicationState::Game);
}

void ALD35GameModeCapturePoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	m_StateMachine.ProcessStateTransitions();
	m_StateMachine.UpdateStates(DeltaSeconds);
}

void ALD35GameModeCapturePoint::PostLogin(APlayerController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		LD35_LOG(LogLD35GameMode, Error, "Player logged in by has no APlayerController");
	}

	if (!IsValid(NewPlayer->PlayerState))
	{
		LD35_LOG(LogLD35GameMode, Error, "Player logged in by has no PlayerState");
	}

	LD35_LOG(LogLD35GameMode, Log, "Player logged in: %s", *NewPlayer->PlayerState->PlayerName);

	// Find the first team that has no player assigned to it
	ETeamEnum team = ULD35Helper::GetFirstEmptyTeam(GetWorld());
	if (team != ETeamEnum::Neutral)
	{
		LD35_LOG(LogLD35GameMode, Log, "Assining new player to team: %s", *ULD35Helper::EnumToString("ETeamEnum", team, "Error"));

		ACapturePointController* capturePoint = ULD35Helper::GetFirstCapturePointOfTeam(GetWorld(), team);
		if (IsValid(capturePoint))
		{
			ALD35Character* character = GetWorld()->SpawnActor<ALD35Character>(MainCharacterClass, capturePoint->RespawnTransform);
			if (IsValid(character))
			{
				character->SetTeamId(team);
				NewPlayer->Possess(character);
				LD35_LOG(LogLD35GameMode, Log, "Matrix(R)\t%s", *character->GetTransform().GetRotation().ToString());
			}
			else
			{
				LD35_LOG(LogLD35GameMode, Error, "Could not spawn character");
			}
		}
		else
		{
			LD35_LOG(LogLD35GameMode, Error, "Could not find a capture point for team: %s", *ULD35Helper::EnumToString("ETeamEnum", team, "Error"));
		}
	}
	else
	{
		LD35_LOG(LogLD35GameMode, Log, "Cannot assign player to a team, all teams are full.");
	}
}

/**********************************************************************************************************************
* HSM
**********************************************************************************************************************/
struct GameModeStatesCapturePoint
{
	/**
	*  Base state which provides functionality for all substates. A way to share utils function
	*/
	struct BaseState : StateWithOwner<ALD35GameModeCapturePoint>
	{

		virtual void OnEnter() override
		{
			FString stateName = GetStateDebugName();
			LD35_LOG(LogLD35GameMode, Log, "%s::OnEnter()", *stateName);
		}

		virtual void OnExit() override
		{
			FString stateName = GetStateDebugName();
			LD35_LOG(LogLD35GameMode, Log, "%s::OnExit()", *stateName);
		}

		ALD35GameState& GameState() const
		{
			return ULD35Helper::GetGameState(Owner().GetWorld());
		}

		void SetCapturePointEnabled(bool Value)
		{
			ACapturePointController* teamCapturePoint = nullptr;
			for (TActorIterator<ACapturePointController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ACapturePointController* capture = *ActorItr;
				capture->EnableCapturePoint(Value);
			}
		}

		bool PopulateRespawnTransform(const ALD35Character& character, FTransform& transform)
		{
			ACapturePointController* closest = nullptr;
			float closestDistance = 9999999999.0f;
			const FVector location = character.GetActorLocation();

			for (TActorIterator<ACapturePointController> ActorItr(character.GetWorld()); ActorItr; ++ActorItr)
			{
				ACapturePointController* capturePoint = *ActorItr;

				if (capturePoint->GetOwnerTeam() == character.GetTeamId())
				{
					const float distSquare = FVector::DistSquaredXY(capturePoint->GetActorLocation(), location);
					if (distSquare < closestDistance)
					{
						closestDistance = distSquare;
						closest = capturePoint;
					}
				}
			}

			if (closest != nullptr)
			{
				transform = closest->RespawnTransform;
				return true;
			}

			return false;
		}

		void DeleteAllNonePlayerCharacters()
		{
			for (TActorIterator<ALD35Character> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35Character* character = *ActorItr;
				if (IsValid(character))
				{
					DeleteNonePlayerCharacter(*character);
				}
			}
		}

		void DeleteNonePlayerCharacter(ALD35Character& Character)
		{
			ALD35PlayerController* playerController = Cast<ALD35PlayerController>(Character.GetController());
			if (playerController == nullptr)
			{
				Character.Controller->Destroy();
				Character.Destroy();
			}
		}

		void ResetMagesPosition()
		{
			for (TActorIterator<ALD35Character> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35Character* character = *ActorItr;
				if (IsValid(character))
				{
					FTransform transform;
					if (PopulateRespawnTransform(*character, transform))
					{
						character->Respawn(transform);
					}
				}
			}
		}

		void ApplyDefaultCapturePoints()
		{
			for (TActorIterator<ACapturePointController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ACapturePointController* capture = *ActorItr;
				const ETeamEnum team = Owner().m_DefaultCapturePointTeam[capture];
				capture->SetOwnerTeam(team);
			}
		}

		void UpdateDefaultCapturePoint()
		{
			Owner().m_DefaultCapturePointTeam.Empty();
			for (TActorIterator<ACapturePointController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ACapturePointController* capture = *ActorItr;
				Owner().m_DefaultCapturePointTeam.Add(capture, capture->GetOwnerTeam());
			}
		}
	};

	/**
	*  Root state which is always on the stack
	*/
	struct Root : BaseState
	{
		virtual Transition GetTransition() override
		{
			return InnerEntryTransition<Spawning>();
		}

		virtual void OnEnter() override
		{
			BaseState::OnEnter();
			UpdateDefaultCapturePoint();
		}
	};

	/**
	*  State in which we are waiting for all players to be ready to go to playing
	*/
	struct Spawning : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (AreAllPlayersReady())
			{
				return SiblingTransition<Playing>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			BaseState::OnEnter();
			GameState().MyGameState = ELD35State::Spawning;
		}

		bool AreAllPlayersReady() const
		{
			bool atLeadOnePlayer = false;
			for (TActorIterator<ALD35PlayerController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				atLeadOnePlayer = true;
				ALD35PlayerController* player = *ActorItr;
				if (player->MyPlayerState == ELD35PlayerState::SplashScreen)
				{
					return false;
				}
			}

			return atLeadOnePlayer;
		}
	};

	/**
	*  State in which the game actually happens
	*/
	struct Playing : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (IsMatchDone())
			{
				return SiblingTransition<EndMatch>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			BaseState::OnEnter();
			GameState().MyGameState = ELD35State::Playing;

			SetupPlayers();
			SetCapturePointEnabled(true);
		}

		virtual void Update(float DeltaTime) override
		{
			ALD35GameState& gameState = GameState();
			UpdateRespawn(DeltaTime, gameState);
			UpdateScore(DeltaTime, gameState);
		}

	private:
		bool IsMatchDone() const
		{
			ETeamEnum team = GetLastStandingTeam();
			ALD35GameState& gameState = GameState();
			if (team != ETeamEnum::Neutral)
			{
				return true;
			}

			team = gameState.GetWinnerTeam();
			return team != ETeamEnum::Neutral;
		}

		ETeamEnum GetLastStandingTeam() const
		{
			ETeamEnum lastStandingTeam = ETeamEnum::Neutral;
			for (const ALD35Character* character : Owner().m_TeamMages)
			{
				if (character != nullptr)
				{
					if (lastStandingTeam != ETeamEnum::Neutral)
					{
						return ETeamEnum::Neutral;
					}

					lastStandingTeam = character->GetTeamId();
				}
			}

			return lastStandingTeam;
		}

		void UpdateRespawn(float DeltaSeconds, ALD35GameState& gameState)
		{
			for (TActorIterator<ALD35Character> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
				//for (ALD35Character* character : Owner().m_TeamMages)
			{
				ALD35Character* character = *ActorItr;
				if (character != nullptr && character->IsDead())
				{
					const float deathTimer = character->GetDeathTimer();
					if (deathTimer >= Owner().TimeBeforeRespawn)
					{
						if (character->PlayerState)
						{
							FTransform transform;
							if (PopulateRespawnTransform(*character, transform))
							{
								character->Respawn(transform);
							}
							else
							{
								gameState.SetTeamDeadForTheGame(character->GetTeamId(), true);
								character = nullptr;
							}
						}
						else
						{
							character->Controller->Destroy();
							character->Destroy();
						}
					}
				}
			}
		}

		void UpdateScore(float DeltaSeconds, ALD35GameState& gameState)
		{
			for (TActorIterator<ACapturePointController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ETeamEnum owner = ActorItr->GetUncontestedOwner();
				if (owner != ETeamEnum::Neutral)
				{
					gameState.TeamList[static_cast<int>(owner)].Score += Owner().CapturePointsGainedPerSecond * DeltaSeconds;
				}
			}
		}

		void SetupPlayers()
		{
			ALD35GameState& gameState = GameState();
			// Create all missing mages
			for (int i = 0; i < static_cast<int>(ETeamEnum::TeamCount); ++i)
			{
				ETeamEnum team = static_cast<ETeamEnum>(i);
				ALD35Character* character = GetMageInTeam(team);
				if (character == nullptr)
				{
					if (SpawnNpcForTeam(team))
					{
						character = GetMageInTeam(team);
						check(character);
					}
				}

				if (character != nullptr)
				{
					Owner().m_TeamMages[i] = character;
					gameState.SetTeamPlayerState(team, CastChecked<ALD35PlayerState>(character->PlayerState));
				}
			}

			gameState.ResetAllTeamState();
		}

		ALD35Character* GetMageInTeam(ETeamEnum Team) const
		{
			for (TActorIterator<ALD35Character> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35Character* character = *ActorItr;
				if (IsValid(character) && character->GetTeamId() == Team)
				{
					return character;
				}
			}

			return nullptr;
		}

		bool SpawnNpcForTeam(ETeamEnum Team)
		{
			ACapturePointController* teamCapturePoint = nullptr;
			for (TActorIterator<ACapturePointController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ACapturePointController* capture = *ActorItr;
				if (capture->GetOwnerTeam() == Team)
				{
					teamCapturePoint = capture;
					break;
				}
			}

			if (teamCapturePoint != nullptr)
			{
				Owner().SpawnAIMage(teamCapturePoint->RespawnTransform, Team);
				return true;
			}

			return false;
		}
	};

	/**
	*  State in which we are showing the result for each player
	*/
	struct EndMatch : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (AreAllPlayersReady())
			{
				return SiblingTransition<Playing>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			BaseState::OnEnter();
			GameState().MyGameState = ELD35State::EndMatch;

			SetCapturePointEnabled(false);

			for (TActorIterator<ALD35PlayerController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35PlayerController* player = *ActorItr;
				player->MyPlayerState = ELD35PlayerState::SplashScreen;
			}

			DeleteAllNonePlayerCharacters();
		}

		virtual void Update(float DeltaTime) override
		{
		}

		virtual void OnExit() override
		{
			BaseState::OnExit();
			DeleteAllNonePlayerCharacters();
			ApplyDefaultCapturePoints();
			ResetMagesPosition();
		}

	private:
		bool AreAllPlayersReady() const
		{
			for (TActorIterator<ALD35PlayerController> ActorItr(Owner().GetWorld()); ActorItr; ++ActorItr)
			{
				ALD35PlayerController* player = *ActorItr;
				if (player->MyPlayerState == ELD35PlayerState::SplashScreen)
				{
					return false;
				}
			}

			return true;
		}
	};
};

void ALD35GameModeCapturePoint::InitHSM()
{
	m_StateMachine.Initialize<GameModeStatesCapturePoint::Root>(this);
	m_StateMachine.SetDebugInfo("GameModeHsm", TraceLevel::Basic);
}