// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35Character.h"
#include "LD35Deflector.h"
#include "LD35Projectile.h"
#include "LD35PlayerController.h"
#include "LD35PlayerState.h"
#include "CapturePointController.h"
#include "GameInputComponent.h"
#include "GameFramework/PlayerState.h"

#include <algorithm>

using namespace hsm;

void ALD35Character::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALD35Character, m_RepCurrentHealth);
	DOREPLIFETIME(ALD35Character, ShootCount);
	DOREPLIFETIME(ALD35Character, PowerShootCount);
	DOREPLIFETIME(ALD35Character, DeflectCount);

	DOREPLIFETIME(ALD35Character, m_TeamId);
	DOREPLIFETIME(ALD35Character, m_PlayerState);
}

// Sets default values
ALD35Character::ALD35Character(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GameInputComponent = ObjectInitializer.CreateDefaultSubobject<UGameInputComponent>(this, TEXT("GameInputComponent"));
}

void ALD35Character::PostInitProperties()
{
	Super::PostInitProperties();

	m_RepCurrentHealth = MaxHealth;
}

void ALD35Character::BeginPlay()
{
	InitHSM();

	Super::BeginPlay();
}

void ALD35Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	m_PlayerState = Cast<ALD35PlayerState>(PlayerState);

	if (IsLocallyControlled())
	{
		DeathPostProcessVisibility = 0.0f;
		mDeathPostProcessVisibilityTarget = 0.0f;
		mDeathPostProcessVisibilityTransitionTime = 1.0f;
		OnDeathPostProcessUpdate.Broadcast();
	}

	OnRep_PlayerState();
}

bool ALD35Character::IsGhost() const
{ 
	return IsValid(m_PlayerState) && m_PlayerState->bIsGhost; 
}

void ALD35Character::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Update state machine
	ALD35GameState* gameState = Cast<ALD35GameState>(GetWorld()->GetGameState());
	if (IsValid(gameState))
	{
		m_StateMachine.ProcessStateTransitions();
		m_StateMachine.UpdateStates(DeltaTime);
	}

 	float visibilityDiff = fabs(DeathPostProcessVisibility - mDeathPostProcessVisibilityTarget);
 	if (IsLocallyControlled() && visibilityDiff > SMALL_NUMBER)
 	{
 		if (visibilityDiff > KINDA_SMALL_NUMBER)
 		{
 			float delta = std::min(visibilityDiff, DeltaTime / mDeathPostProcessVisibilityTransitionTime);
 
			DeathPostProcessVisibility += (DeathPostProcessVisibility > mDeathPostProcessVisibilityTarget ? -delta : delta);
 		}
 		else
 		{
			DeathPostProcessVisibility = mDeathPostProcessVisibilityTarget;
 		}
		OnDeathPostProcessUpdate.Broadcast();
 	}
}

void ALD35Character::Respawn(const FTransform& transform)
{
	TeleportTo(transform.GetLocation(), FRotator(transform.GetRotation()));
	ResetHealth();
}

void ALD35Character::ResetHealth()
{
	m_RepCurrentHealth = MaxHealth;
}

void ALD35Character::ResetMana()
{
	if (HasAuthority() || IsLocallyControlled())
	{
		ManaCount = ManaMax;
	}
}

void ALD35Character::Damage(int DamageAmount)
{
	if (HasAuthority())
	{
		m_RepCurrentHealth -= DamageAmount;
	}
}

bool ALD35Character::ServerShoot_Validate()
{
	return true;
}

void ALD35Character::ServerShoot_Implementation()
{
	// TODO: Handle security on shooting without preventing the shoot
	if (CanShoot())
	{
		++ShootCount;
		m_ActionCooldownTimer = ShootCooldown;
		ManaCount -= ManaShootCost;
	}
}

bool ALD35Character::ServerPowerShoot_Validate()
{
	return true;
}

void ALD35Character::ServerPowerShoot_Implementation()
{
	// TODO: Handle security on shooting without preventing the shoot
	if (CanPowerShoot())
	{
		++PowerShootCount;
		m_ActionCooldownTimer = ShootCooldown;
		ManaCount -= ManaPowerShootCost;
	}
}

bool ALD35Character::ServerDeflect_Validate()
{
	return true;
}

void ALD35Character::ServerDeflect_Implementation()
{
	// TODO: Handle security on shooting without preventing the shoot
	if (CanDeflect())
	{
		++DeflectCount;
		m_ActionCooldownTimer = DeflectCooldown;
		ManaCount -= ManaDeflectCost;
	}
}

void ALD35Character::SetTeamId(ETeamEnum Team)
{
	m_TeamId = Team;
	OnTeamIdChanged.Broadcast();
}

void ALD35Character::OnRep_TeamId()
{
	OnTeamIdChanged.Broadcast();
}

void ALD35Character::OnRep_PlayerState()
{
	if (IsValid(m_PlayerState))
	{
		m_PlayerState->Character = this;
	}
}

FVector ALD35Character::GetShootLocation() const
{
	FVector ShootLocation;
	if (WeaponSocketName != NAME_None)
	{
		ShootLocation = GetMesh()->GetSocketLocation(WeaponSocketName);
	}
	else
	{
		auto ShootOffset = WeaponDefaultOffset;
		ShootOffset.RotateAngleAxis(GetActorRotation().Yaw, FVector(0, 0, 1));
		ShootLocation = GetActorLocation() + ShootOffset;
	}

	return ShootLocation;
}

void ALD35Character::SpawnProjectle()
{
	if (HasAuthority())
	{
		const FVector ShootLocation = GetShootLocation();
		SpawnProjectle(*ProjectileClass, ShootLocation, GetActorRotation());
	}
}

void ALD35Character::SpawnProjectle(UClass* projectileClass, const FVector& Position, const FRotator& Rotation)
{
	if (IsValid(ProjectileClass))
	{
		FActorSpawnParameters params;
		params.Instigator = this;
		auto Projectile = Cast<ALD35Projectile>(GetWorld()->SpawnActor(projectileClass, &Position, &Rotation, params));
		if (IsValid(Projectile))
		{
			UE_LOG(LogLD35, Verbose, TEXT("[%s] Fired Projectile %s"), *GetName(), *Projectile->GetName());
			Projectile->SetTeamId(m_TeamId);
		}
		else
		{
			UE_LOG(LogLD35, Verbose, TEXT("[%s] Failed to fire Projectile"), *GetName());
		}
	}
}

void ALD35Character::SpawnPowerProjectle()
{
	if (HasAuthority())
	{
		const FVector ShootLocation = GetShootLocation();
		SpawnProjectle(*PowerProjectileClass, ShootLocation, GetActorRotation());
	}
}

void ALD35Character::SpawnDeflect()
{
	if (HasAuthority())
	{
		if (IsValid(DeflectorClass))
		{
			const FVector Position = GetActorLocation();
			const FRotator Rotation = GetActorRotation();
			FActorSpawnParameters params;
			params.Instigator = this;
			params.Owner = this;
			ALD35Deflector* deflector = Cast<ALD35Deflector>(GetWorld()->SpawnActor(*DeflectorClass, &Position, &Rotation, params));
			if (IsValid(deflector))
			{
				UE_LOG(LogLD35, Verbose, TEXT("[%s] Fired a Deflector %s"), *GetName(), *deflector->GetName());
				deflector->TeamId = m_TeamId;
				deflector->Cooldown = DeflectCooldown;
				deflector->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
			else
			{
				UE_LOG(LogLD35, Verbose, TEXT("[%s] Failed to deflect"), *GetName());
			}
		}
	}
}

bool ALD35Character::CanDoAction() const
{
	return m_ActionCooldownTimer <= 0.0f;
}

bool ALD35Character::CanShoot() const
{
	return ManaCount >= ManaShootCost;
}

bool ALD35Character::CanPowerShoot() const
{
	return ManaCount >= ManaPowerShootCost;
}

bool ALD35Character::CanDeflect() const
{
	return ManaCount >= ManaDeflectCost;
}

/**********************************************************************************************************************
* HSM
**********************************************************************************************************************/
struct CharacterStates
{
	struct BaseState : StateWithOwner<ALD35Character>
	{
		bool IsReadyPlaying() const
		{
			return Owner().GameInputComponent->WantsToStartGame();
		}

		void TransitionTopPlayingCamera()
		{
			ALD35PlayerController* playerController = Cast<ALD35PlayerController>(Owner().GetController());
			if (playerController != nullptr && Owner().IsLocallyControlled())
			{
				playerController->TransitionToPlayingCamera();
				Owner().mDeathPostProcessVisibilityTarget = 0.0f;
				Owner().mDeathPostProcessVisibilityTransitionTime = 0.8;
			}
		}

		void TransitionToDeadCamera()
		{
			ALD35PlayerController* playerController = Cast<ALD35PlayerController>(Owner().GetController());
			if (playerController != nullptr && Owner().IsLocallyControlled())
			{
				static const FString name = "PlayerDeadCam";
				AActor* currentViewTarget = playerController->GetViewTarget();
				ACameraActor* newCamera = ULD35Helper::GetCameraByName(name);
				if (currentViewTarget != newCamera)
				{
					playerController->SetViewTargetWithBlend(newCamera, 3.0f);
				}
				Owner().mDeathPostProcessVisibilityTarget = 1.0f;
				Owner().mDeathPostProcessVisibilityTransitionTime = 3.0f;
			}
		}

		void SetSpawnCamera()
		{
			ALD35PlayerController* playerController = Cast<ALD35PlayerController>(Owner().GetController());
			if (playerController && Owner().IsLocallyControlled())
			{
				AActor* pCamera = GetSpawnCameraByTeam(Owner().m_TeamId);
				if (IsValid(pCamera))
				{
					playerController->SetViewTarget(pCamera);
				}
			}
		}

		AActor* GetSpawnCameraByTeam(ETeamEnum Team) const
		{
			static FString startingCamName[TEAM_COUNT] = { "PlayerStartCam", "PlayerStartCam2", "PlayerStartCam3", "PlayerStartCam4" };
			int team = static_cast<int>(Team);
			if (team < TEAM_COUNT)
			{
				ACameraActor* camera = ULD35Helper::GetCameraByName(startingCamName[team]);
				if (camera)
				{
					return camera;
				}

				ACapturePointController* capturePoint = ULD35Helper::GetFirstCapturePointOfTeam(Owner().GetWorld(), Team);
				if (capturePoint)
				{
					return capturePoint;
				}
			}

			return nullptr;
		}

		bool IsEndOfMatch() const
		{
			return ULD35Helper::GetGameState(Owner().GetWorld()).IsEndOfMatch();
		}
	};

	struct Root : BaseState
	{
		virtual Transition GetTransition() override
		{
			return InnerEntryTransition<Spawning>();
		}
	};

	struct Spawning : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (IsValid(Owner().GetController()) && (!Owner().IsLocallyControlled() || m_SpawnCameraSet)
				&& ULD35Helper::GetGameState(Owner().GetWorld()).IsPlaying())
				return SiblingTransition<Playing>();

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			LD35_LOG(LogLD35Character, Log, "Matrix(R)\t%s", *Owner().GetTransform().GetRotation().ToString());
		}

		virtual void Update(float DeltaTime) override
		{
			if (Owner().IsLocallyControlled())
			{
				SetSpawnCamera();
				m_SpawnCameraSet = true;
			}
		}

	private:
		bool m_SpawnCameraSet = false;
	};

	struct Playing : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (IsEndOfMatch())
				return SiblingTransition<EndOfMatch>();

			return InnerEntryTransition<Alive>();
		}
	};

	struct Alive : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (Owner().IsDead())
				return SiblingTransition<Dead>();

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			TransitionTopPlayingCamera();

			Owner().ResetHealth();
			Owner().ResetMana();
			Owner().m_ActionCooldownTimer = 0.0f;
			Owner().OnRespawned.Broadcast();
		}

		virtual void Update(float DeltaTime) override
		{
			Owner().m_ActionCooldownTimer -= DeltaTime;

			if (Owner().IsLocallyControlled())
			{
				// Process movement
				FVector movementDirection = Owner().GameInputComponent->GetMovement();
				Owner().AddMovementInput(movementDirection);

				const float yaw = Owner().GameInputComponent->GetYaw();

				if (FMath::Abs(yaw) > 0.0001f)
				{
					Owner().AddControllerYawInput(yaw);
				}

				if (Owner().GameInputComponent->WantsToShoot(true))
				{
					if (Owner().CanDoAction() && Owner().CanShoot())
					{
						LD35_LOG(LogLD35Character, Log, "RPC ServerShoot");
						Owner().ServerShoot();
						Owner().m_ActionCooldownTimer = Owner().ShootCooldown;
						if (Owner().IsNetMode(NM_Client))
						{
							// When running on the server, the mana will be deduced in the RPC
							// For local player client, deduce mana here.
							Owner().ManaCount -= Owner().ManaShootCost;
						}
					}
					else
					{
						LD35_LOG(LogLD35Character, VeryVerbose, "WantsToShoot but can't");
					}
				}

				if (Owner().GameInputComponent->WantsToPowerShoot(true))
				{
					if (Owner().CanDoAction() && Owner().CanPowerShoot())
					{
						LD35_LOG(LogLD35Character, Log, "RPC ServerPowerShoot");
						Owner().ServerPowerShoot();
						Owner().m_ActionCooldownTimer = Owner().ShootCooldown;
						if (Owner().IsNetMode(NM_Client))
						{
							// When running on the server, the mana will be deduced in the RPC
							// For local player client, deduce mana here.
							Owner().ManaCount -= Owner().ManaPowerShootCost;
						}
					}
					else
					{
						LD35_LOG(LogLD35Character, VeryVerbose, "WantsToShoot but can't");
					}
				}

				if (Owner().GameInputComponent->WantsToDeflect())
				{
					if (Owner().CanDoAction() && Owner().CanDeflect())
					{
						LD35_LOG(LogLD35Character, Log, "RPC ServerDeflect");
						Owner().ServerDeflect();
						Owner().m_ActionCooldownTimer = Owner().DeflectCooldown;
						if (Owner().IsNetMode(NM_Client))
						{
							// When running on the server, the mana will be deduced in the RPC
							// For local player client, deduce mana here.
							Owner().ManaCount -= Owner().ManaDeflectCost;
						}
					}
					else
					{
						LD35_LOG(LogLD35Character, VeryVerbose, "WantsToDeflect but can't");
					}
				}
			}


			if (!Owner().IsNetMode(NM_Client) || Owner().IsLocallyControlled())
			{
				Owner().ManaCount = std::min(Owner().ManaCount + DeltaTime * Owner().ManaRegenRate, Owner().ManaMax);
			}
		}
	};

	struct Dead : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (!Owner().IsDead())
				return SiblingTransition<Alive>();

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			TransitionToDeadCamera();

			Owner().DiedDelegate.Broadcast();
		}

		virtual void Update(float DeltaTime) override
		{
			Owner().m_DeathTimer += DeltaTime;
		}

		virtual void OnExit() override
		{
			Owner().m_DeathTimer = 0.0f;
		}
	};

	struct EndOfMatch : BaseState
	{
		virtual Transition GetTransition() override
		{
			if (ULD35Helper::GetGameState(Owner().GetWorld()).IsPlaying())
				return SiblingTransition<Playing>();

			return NoTransition();
		}

		virtual void OnEnter() override
		{
			TransitionToDeadCamera();
		}
	};
};

void ALD35Character::InitHSM()
{
	m_StateMachine.Initialize<CharacterStates::Root>(this);
	m_StateMachine.SetDebugInfo("CharacterHsm", TraceLevel::Basic);
}
