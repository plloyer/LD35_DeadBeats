// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "LD35.h"
#include "LD35GameInstance.h"
#include "LD35MenuGameMode.h"

using namespace hsm;

ALD35MenuGameMode::ALD35MenuGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALD35MenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitHSM();

	GetLD35GameInstance()->SetApplicationState(EApplicationState::MainMenu);
}

void ALD35MenuGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	m_StateMachine.ProcessStateTransitions();
	m_StateMachine.UpdateStates(DeltaSeconds);
}

/**********************************************************************************************************************
* HSM
**********************************************************************************************************************/
struct MenuGameModeStates
{
	/**
	*  Base state which provides functionality for all substates. A way to share utils function
	*/
	struct BaseState : StateWithOwner<ALD35MenuGameMode>
	{

		virtual void OnEnter() override
		{
			FString stateName = GetStateDebugName();
			LD35_LOG(LogLD35GameMode, Log, "%s::OnEnter()", *stateName);
			GameState().MyGameState = ELD35State::Menu;
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
	};

	/**
	*  Root state which is always on the stack
	*/
	struct Root : BaseState
	{
		virtual Transition GetTransition() override
		{
			return NoTransition();
		}
	};
};

void ALD35MenuGameMode::InitHSM()
{
	m_StateMachine.Initialize<MenuGameModeStates::Root>(this);
	m_StateMachine.SetDebugInfo("MenuGameModeHsm", TraceLevel::Basic);
}