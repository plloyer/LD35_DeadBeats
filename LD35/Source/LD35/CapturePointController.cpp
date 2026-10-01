// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "CapturePointController.h"

using namespace hsm;

// Sets default values
ACapturePointController::ACapturePointController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	m_SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SetRootComponent(m_SphereComponent);
	m_SphereComponent->InitSphereRadius(CaptureRadius);
	m_SphereComponent->SetCollisionProfileName(TEXT("Trigger"));
	EnableCapturePoint(false);
}

void ACapturePointController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACapturePointController, m_Replicated);
}

void ACapturePointController::SetupCollisionCylinder()
{
	m_SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ACapturePointController::ServerOnBeginOverlap);
	m_SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ACapturePointController::ServerOnBeginOverlap);

	m_SphereComponent->OnComponentEndOverlap.RemoveDynamic(this, &ACapturePointController::ServerOnEndOverlap);
	m_SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ACapturePointController::ServerOnEndOverlap);
}

void ACapturePointController::PostActorCreated()
{
	Super::PostActorCreated();

	if (HasAuthority())
	{
		SetupCollisionCylinder();
	}
}

void ACapturePointController::PostLoad()
{
	Super::PostLoad();

	if (HasAuthority())
	{
		SetupCollisionCylinder();
	}
}

// Called when the game starts or when spawned
void ACapturePointController::BeginPlay()
{
	Super::BeginPlay();

	InitHSM();

	if (m_Replicated.OwnerTeam != ETeamEnum::Neutral)
	{
		CapturePercent = 1.0f;
	}

	if (HasAuthority())
	{
		m_SphereComponent->SetSphereRadius(CaptureRadius);
	}
}

void ACapturePointController::EnableCapturePoint(bool Value)
{
	ensure(HasAuthority());
	m_SphereComponent->SetCollisionEnabled(Value ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	m_Replicated.Enabled = Value;
}

// Called every frame
void ACapturePointController::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Update state machine
	m_StateMachine.ProcessStateTransitions();
	m_StateMachine.UpdateStates(DeltaTime);
}

FColor ACapturePointController::GetOwnerColor() const
{
	return GetTeamColor(m_Replicated.OwnerTeam);
}

FColor ACapturePointController::GetCapturingColor() const
{
	return GetTeamColor(m_Replicated.CapturingTeam);
}

FColor ACapturePointController::GetTeamColor(ETeamEnum Team) const
{
	const int teamIndex = static_cast<int>(Team);
	const int teamCount = static_cast<int>(ETeamEnum::TeamCount);
	static const FColor color[] = { FColor(0x7e, 0x35, 0x36), FColor(0x7e, 0x35, 0x36), FColor(0x7e, 0x35, 0x36), FColor(0x7e, 0x35, 0x36) };
	if (teamIndex < teamCount)
	{
		return color[teamIndex];
	}

	return FColor(0, 0, 0);
}

void ACapturePointController::ServerOnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ALD35Character* character = Cast<ALD35Character>(Other);
	if (character)
	{
		m_CharactersInTrigger.Add(character);
	}
}

void ACapturePointController::ServerOnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ALD35Character* character = Cast<ALD35Character>(Other);
	if (character)
	{
		m_CharactersInTrigger.Remove(character);
	}
}

void ACapturePointController::ServerRemoveInvalidCharacters()
{
	for (int i = m_CharactersInTrigger.Num() - 1; i >= 0; --i)
	{
		if (!m_CharactersInTrigger[i].IsValid())
		{
			m_CharactersInTrigger.RemoveAt(i);
		}
	}
}

ETeamEnum ACapturePointController::GetUncontestedOwner() const
{
	if (m_ServerCanGivePoints)
	{
		return m_Replicated.OwnerTeam;
	}

	return ETeamEnum::Neutral;
}

/**********************************************************************************************************************
* HSM
**********************************************************************************************************************/
struct CapturePointStates
{
	struct BaseState : StateWithOwner<ACapturePointController>
	{
		ETeamEnum GetOwnerTeam()
		{
			return Owner().m_Replicated.OwnerTeam;
		}

		ETeamEnum GetCapturingTeam()
		{
			return Owner().m_Replicated.CapturingTeam;
		}

		ETeamEnum GetTeamInTrigger() const 
		{
			return Owner().GetTeamInTrigger();
		}

		uint32 GetTeamCountInTrigger() const
		{
			return Owner().GetTeamCountInTrigger();
		}

		bool HasAuthority() const
		{
			return Owner().HasAuthority();
		}

		ECapturePointState GetState() const
		{
			return Owner().m_Replicated.State;
		}

		void SetState(ECapturePointState CapturePointState)
		{
			if (Owner().HasAuthority())
			{
				Owner().m_Replicated.State = CapturePointState;
			}
		}

		void LogState()
		{
			FString enabledText = Owner().m_Replicated.Enabled ? TEXT("Enabled") : TEXT("Disabled");
			LD35_VLOG(&Owner(), LogCapturePoint, Log, "%s", *enabledText);

			if (Owner().m_Replicated.Enabled)
			{
				FString state = ULD35Helper::EnumToString("ECapturePointState", Owner().m_Replicated.State, "NotCaptured");
				LD35_VLOG(&Owner(), LogCapturePoint, Log, "State: %s", *state);

				FString owner = ULD35Helper::EnumToString("ETeamEnum", Owner().m_Replicated.OwnerTeam, "Neutral");
				LD35_VLOG(&Owner(), LogCapturePoint, Log, "Owner: %s", *owner);

				FString capturing = ULD35Helper::EnumToString("ETeamEnum", Owner().m_Replicated.CapturingTeam, "Neutral");
				LD35_VLOG(&Owner(), LogCapturePoint, Log, "CapturingTeam: %s", *capturing);

				LD35_VLOG(&Owner(), LogCapturePoint, Log, "TeamCountInTrigger: %i", Owner().m_Replicated.TeamCountInTrigger);

				FString teamInTrigger = ULD35Helper::EnumToString("ETeamEnum", Owner().m_Replicated.TeamInTrigger, "Neutral");
				LD35_VLOG(&Owner(), LogCapturePoint, Log, "TeamInTrigger: %s", *teamInTrigger);
			}
		}
	};

	struct Disabled : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (Owner().IsEnabled())
			{
				return SiblingTransition<Active>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			LogState();
			Owner().OnCaptureDone.Broadcast(true);
		}
	};

	struct Active : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (!Owner().IsEnabled())
			{
				return SiblingTransition<Disabled>();
			}

			if (GetOwnerTeam() != ETeamEnum::Neutral)
			{
				return InnerEntryTransition<Captured>();
			}
			return InnerEntryTransition<NotCaptured>();
		}

		virtual void OnEnter() override
		{
			Owner().OnCaptureDone.Broadcast(true);
		}

		virtual void Update(float DeltaTime)
		{
			if (HasAuthority())
			{
				Owner().ServerRemoveInvalidCharacters();
				ServerPopulateCharactersInTrigger(m_CharacterList, Owner().m_Replicated.TeamCountInTrigger, Owner().m_Replicated.TeamInTrigger);
			}
		}
		
	private:
		void ServerPopulateCharactersInTrigger(CharacterList* List, uint32& TeamInTrigger, ETeamEnum& Team) const
		{
			for (uint32 i = 0; i < (int)ETeamEnum::TeamCount; ++i)
			{
				List[i].Empty();
			}

			for (auto& character : Owner().m_CharactersInTrigger)
			{
				ETeamEnum team = character->GetTeamId();
				if (team < ETeamEnum::TeamCount)
				{
					List[(int)team].Add(character);
				}
			}

			TeamInTrigger = 0;
			for (uint32 i = 0; i < (int)ETeamEnum::TeamCount; ++i)
			{
				int32 count = List[i].Num();
				if (count > 0)
				{
					++TeamInTrigger;
					Team = ETeamEnum(i);
				}
			}
		}

	private:
		CharacterList m_CharacterList[(int)ETeamEnum::TeamCount];
	};

	struct NotCaptured : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (HasAuthority())
			{
				if (GetTeamCountInTrigger() != 0)
				{
					return SiblingTransition<Capturing>();
				}
			}
			else if (GetState() != ECapturePointState::NotCaptured)
			{
				if (GetState() == ECapturePointState::Captured)
					return SiblingTransition<Captured>();
				else
					return SiblingTransition<Capturing>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			LogState();
			SetState(ECapturePointState::NotCaptured);

			if (HasAuthority())
			{
				Owner().m_Replicated.OwnerTeam = ETeamEnum::Neutral;
				Owner().m_Replicated.CapturingTeam = ETeamEnum::Neutral;
			}
		}
	};

	struct Capturing : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (HasAuthority())
			{
				// No one left, abort capture
				const uint32 teamCountInTrigger = GetTeamCountInTrigger();
				if (teamCountInTrigger == 0)
				{
					Owner().OnCaptureCancelled.Broadcast();
					if (GetOwnerTeam() == ETeamEnum::Neutral)
						return SiblingTransition<NotCaptured>();
					else
						return SiblingTransition<Captured>();
				}

				// Only owner is left, abort capture
				if (teamCountInTrigger == 1 && GetTeamInTrigger() == GetOwnerTeam())
				{
					Owner().OnCaptureCancelled.Broadcast();
					if (GetOwnerTeam() == ETeamEnum::Neutral)
						return SiblingTransition<NotCaptured>();
					else
						return SiblingTransition<Captured>();
				}

				// The capturing team changed, we restart the capturing from scratch
				if (teamCountInTrigger == 1 && GetTeamInTrigger() != GetCapturingTeam())
				{
					Owner().OnCaptureCancelled.Broadcast();
					return SiblingTransition<Capturing>();
				}

				// Capture is done
				if (m_CaptureTimeLeft <= 0)
				{
					if (Owner().HasAuthority())
					{
						Owner().m_Replicated.OwnerTeam = GetTeamInTrigger();
					}

					Owner().OnCaptureDone.Broadcast(false);
					return SiblingTransition<Captured>();
				}
			}
			else
			{
				if (GetState() == ECapturePointState::Captured)
				{
					Owner().OnCaptureDone.Broadcast(false);
					return SiblingTransition<Captured>();
				}
				if (GetState() == ECapturePointState::NotCaptured)
					return SiblingTransition<NotCaptured>();
			}
			return InnerEntryTransition<Capturing_InProgress>();
		}

		virtual void OnEnter() override
		{
			SetState(ECapturePointState::Capturing);

			m_CaptureTimeLeft = Owner().CaptureTime;

			if (Owner().HasAuthority())
			{
				Owner().m_Replicated.CapturingTeam = GetTeamInTrigger();
			}

			Owner().CapturePercent = 0.0f;

			Owner().OnCaptureStart.Broadcast(Owner().m_Replicated.CapturingTeam);
		}

		virtual void OnExit() override
		{
			Owner().CapturePercent = 0.0f;
		}

		float m_CaptureTimeLeft;
	};

	struct Capturing_InProgress : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (HasAuthority())
			{
				if (GetTeamCountInTrigger() > 1)
				{
					return SiblingTransition<Capturing_Contested>();
				}
			}
			else
			{
				if (GetState() == ECapturePointState::Contested)
					return SiblingTransition<Capturing_Contested>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			LogState();
		}

		virtual void Update(float DeltaTime) override
		{
			GetOuterState<Capturing>()->m_CaptureTimeLeft -= DeltaTime;
			Owner().CapturePercent = 1.0f - (GetOuterState<Capturing>()->m_CaptureTimeLeft / Owner().CaptureTime);
		}
	};

	struct Capturing_Contested : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (HasAuthority())
			{
				if (GetTeamCountInTrigger() == 1)
				{
					Owner().OnCaptureResumed.Broadcast();
					return SiblingTransition<Capturing_InProgress>();
				}
			}
			else
			{
				if (GetState() == ECapturePointState::Capturing)
					return SiblingTransition<Capturing_InProgress>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			SetState(ECapturePointState::Contested);

			LogState();
			Owner().OnCapturePaused.Broadcast();
		}

		virtual void OnExit() override
		{
			SetState(ECapturePointState::Capturing);
		}
	};

	struct Captured : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (HasAuthority())
			{
				if (GetTeamCountInTrigger() == 1 && GetTeamInTrigger() != GetOwnerTeam())
				{
					return SiblingTransition<Capturing>();
				}

				// Changed to neutral by script
				if (GetOwnerTeam() == ETeamEnum::Neutral)
				{
					return SiblingTransition<NotCaptured>();
				}

				// Team changed by script
				if (GetOwnerTeam() != CapturedOwner)
				{
					return SiblingTransition<Captured>();
				}
			}
			else
			{
				if (GetState() == ECapturePointState::NotCaptured)
					return SiblingTransition<NotCaptured>();
				if (GetState() != ECapturePointState::Captured)
					return SiblingTransition<Capturing>();
			}

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			SetState(ECapturePointState::Captured);

			Owner().CapturePercent = 1.0f;
			Owner().m_ServerCanGivePoints = true;
			CapturedOwner = Owner().m_Replicated.CapturingTeam = GetOwnerTeam();

			m_SpawnTimer = Owner().TimeBetweenSpawn;

			LogState();
			Owner().SpawnDeletage.Broadcast();
		}

		virtual void Update(float DeltaTime) override
		{
			m_SpawnTimer -= DeltaTime;
			if (m_SpawnTimer <= 0.f)
			{
				m_SpawnTimer += Owner().TimeBetweenSpawn;
				Owner().SpawnDeletage.Broadcast();
			}
		}

		virtual void OnExit() override
		{
			Owner().m_ServerCanGivePoints = false;
		}

	private:
		float m_SpawnTimer;

		// For scripted team change
		ETeamEnum CapturedOwner;
	};
};

void ACapturePointController::InitHSM()
{
	m_StateMachine.Initialize<CapturePointStates::Disabled>(this);
	m_StateMachine.SetDebugInfo("CapturePointHsm", TraceLevel::Basic);
}