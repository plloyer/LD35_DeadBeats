// Fill out your copyright notice in the Description page of Project Settings.

#include "LD35.h"
#include "LD35PlayerController.h"
#include "GameInputComponent.h"
#include "LD35Character.h"

namespace Bindings
{
	static FName MoveVertical(TEXT("MoveForward"));
	static FName MoveHorizontal(TEXT("MoveRight"));
	static FName LookUp(TEXT("LookUp"));
	static FName LookRight(TEXT("LookRight"));
	static FName Shoot(TEXT("Shoot"));
	static FName Deflect(TEXT("Deflect"));
	static FName Follow(TEXT("Follow"));
	static FName Return(TEXT("Return"));
	static FName StartGame(TEXT("StartGame"));
	static FName ChangeCamera(TEXT("ChangeCamera"));
	static FName BehindLookRight(TEXT("BehindLookRight"));
}

void ALD35PlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALD35PlayerController, MyPlayerState, COND_AutonomousOnly);
}

void ALD35PlayerController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	if (GetLD35Character())
	{
		m_GameInputComponent = GetPawn()->FindComponentByClass<UGameInputComponent>();
		check(m_GameInputComponent != nullptr);
		m_GameInputComponent->SetConsumeInput(false);
		ClientInvalidateGameInputComponent();
		GetLD35Character()->OnCameraTypeChanged.Broadcast(m_CameraType);
	}
}

void ALD35PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ALD35PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (IsLocalController())
	{
		UpdateCamera();

		ALD35GameState* gameState = Cast<ALD35GameState>(GetWorld()->GetGameState());
		if (IsValid(gameState) && gameState->IsInMenu())
		{
			SetDefaultCamera();
		}
	}

	auto character = GetLD35Character();
	if (DeltaSeconds > 0.0f && m_StartShootTime > 0.0f && IsValid(character))
	{
		float timeDelta = GetWorld()->GetTimeSeconds() - m_StartShootTime;

		// Check of we need to trigger PowerShot notification
		if (!m_PowerShootNotified && timeDelta > character->ChargeAttackTime)
		{
			character->OnPowerShootReady.Broadcast();
			m_PowerShootNotified = true;
		}

		if (!m_PowerShootChargingNotified && timeDelta > 0.5f)
		{
			character->OnPowerShootStartCharging.Broadcast();
			m_PowerShootChargingNotified = true;
		}
	}
}

void ALD35PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis(Bindings::MoveVertical, this, &ALD35PlayerController::OnMoveVertical).bConsumeInput = false;
	InputComponent->BindAxis(Bindings::MoveHorizontal, this, &ALD35PlayerController::OnMoveHorizontal).bConsumeInput = false;
	InputComponent->BindAxis(Bindings::LookUp, this, &ALD35PlayerController::OnLookUp).bConsumeInput = false;
	InputComponent->BindAxis(Bindings::LookRight, this, &ALD35PlayerController::OnLookRight).bConsumeInput = false;
	InputComponent->BindAxis(Bindings::BehindLookRight, this, &ALD35PlayerController::OnBehindLookRight).bConsumeInput = false;

	InputComponent->BindAction(Bindings::Shoot, IE_Pressed, this, &ALD35PlayerController::OnShootStart).bConsumeInput = false;
	InputComponent->BindAction(Bindings::Shoot, IE_Released, this, &ALD35PlayerController::OnShootStop).bConsumeInput = false;

	InputComponent->BindAction(Bindings::Deflect, IE_Pressed, this, &ALD35PlayerController::OnDeflectStart).bConsumeInput = false;
	InputComponent->BindAction(Bindings::Deflect, IE_Released, this, &ALD35PlayerController::OnDeflectStop).bConsumeInput = false; 

	InputComponent->BindAction(Bindings::Follow, IE_Pressed, this, &ALD35PlayerController::OnFollowStart).bConsumeInput = false;
	InputComponent->BindAction(Bindings::Follow, IE_Released, this, &ALD35PlayerController::OnFollowStop).bConsumeInput = false;

	InputComponent->BindAction(Bindings::Return, IE_Pressed, this, &ALD35PlayerController::OnReturnStart).bConsumeInput = false;
	InputComponent->BindAction(Bindings::Return, IE_Released, this, &ALD35PlayerController::OnReturnStop).bConsumeInput = false;

	InputComponent->BindAction(Bindings::StartGame, IE_Pressed, this, &ALD35PlayerController::OnStartGameStart).bConsumeInput = false;
	InputComponent->BindAction(Bindings::StartGame, IE_Released, this, &ALD35PlayerController::OnStartGameStop).bConsumeInput = false;

	InputComponent->BindAction(Bindings::ChangeCamera, IE_Pressed, this, &ALD35PlayerController::OnChangeCamera).bConsumeInput = false;

#if WITH_EDITOR
	InputComponent->BindAction("TestTakeDamage", IE_Pressed, this, &ALD35PlayerController::ServerTestTakeDamage).bConsumeInput = false;
#endif

	//Activate mouse cursor by default
	//bShowMouseCursor = true;
}

void ALD35PlayerController::SetDefaultCamera()
{
	for (TActorIterator<ACameraActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ACameraActor* camera = *ActorItr;
		if (IsValid(camera) && camera->ActorHasTag("DefaultCamera"))
		{
			SetViewTarget(camera);
			return;
		}
	}
}

ALD35Character* ALD35PlayerController::GetLD35Character() const
{
	return Cast<ALD35Character>(GetPawn());
}

bool ALD35PlayerController::ValidateGameInputComponent()
{
	bool bIsValid = IsValid(m_GameInputComponent);
	if (!bIsValid && GetPawn())
	{
		m_GameInputComponent = GetPawn()->FindComponentByClass<UGameInputComponent>();
		bIsValid = IsValid(m_GameInputComponent);
	}
	return bIsValid;
}

void ALD35PlayerController::OnMoveHorizontal(float val)
{
	if (ValidateGameInputComponent())
	{
		if (m_CameraType == ECameraType::TopDown)
		{
			m_GameInputComponent->SetMovementHorizontal(val);
		}
		else if (m_CameraType == ECameraType::Behind)
		{
			m_LocalMovementDirection.Y = val;
			UpdateMovementFromLocalDirection();
		}
	}
}

void ALD35PlayerController::OnMoveVertical(float val)
{
	if (ValidateGameInputComponent())
	{
		if (m_CameraType == ECameraType::TopDown)
		{
			m_GameInputComponent->SetMovementVertical(-val);
		}
		else if (m_CameraType == ECameraType::Behind)
		{
			m_LocalMovementDirection.X = val;
			UpdateMovementFromLocalDirection();
		}
	}
}

void ALD35PlayerController::UpdateMovementFromLocalDirection()
{
	ALD35Character* character = GetLD35Character();
	if (IsValid(character) && ValidateGameInputComponent())
	{
		const FVector worldDirection = character->GetActorTransform().TransformVector(m_LocalMovementDirection);
		m_GameInputComponent->SetMovementDirection(worldDirection);
	}
}

void ALD35PlayerController::OnLookUp(float val)
{
	if (ValidateGameInputComponent())
	{
		if (m_CameraType == ECameraType::TopDown)
		{
			m_ScreenSpaceLookDirection.Y = val;
			UpdateYawFromScreenSpaceDirection();
		}
	}
}

void ALD35PlayerController::OnLookRight(float val)
{
	if (ValidateGameInputComponent())
	{
		if (m_CameraType == ECameraType::TopDown)
		{
			m_ScreenSpaceLookDirection.X = val;
			UpdateYawFromScreenSpaceDirection();
		}
	}
}

void ALD35PlayerController::UpdateYawFromScreenSpaceDirection()
{
	ALD35Character* character = GetLD35Character();
	if (IsValid(character) && ValidateGameInputComponent())
	{
		float delta = 0.0f;
		if (m_ScreenSpaceLookDirection.SizeSquared() > 0.001f)
		{
			const float originalTargetYaw = m_ScreenSpaceLookDirection.Rotation().Yaw;
			float targetYaw = originalTargetYaw;
			if (targetYaw < 0.0f)
			{
				targetYaw = 360.0f + targetYaw;
			}

			const float originalActorYaw = character->GetControlRotation().Yaw;
			float actorYaw = originalActorYaw;
			delta = targetYaw - actorYaw;
			delta = delta > 180.0f ? delta - 360.0f : delta < -180.0f ? delta + 360.0f : delta;
		}
		m_GameInputComponent->SetYaw(delta);
	}
}

void ALD35PlayerController::OnBehindLookRight(float val)
{
	if (ValidateGameInputComponent())
	{
		if (m_CameraType == ECameraType::Behind)
		{
			m_GameInputComponent->SetYaw(val * ThirdPersonYawMultiplier);
		}
	}
}

void ALD35PlayerController::OnLookAt(FVector2D to)
{
	OnLookUp(to.Y);
	OnLookRight(to.X);
}

void ALD35PlayerController::OnShootStart()
{
	if (ValidateGameInputComponent())
	{
		LD35_LOG(LogLD35PlayerController, Log, "Start Shoot mode");
		//m_GameInputComponent->SetWantsToShoot(true);
		m_StartShootTime = GetWorld()->GetTimeSeconds();
		m_PowerShootNotified = false;
		m_PowerShootChargingNotified = false;
	}
}

void ALD35PlayerController::OnShootStop()
{
	if (ValidateGameInputComponent())
	{
		if (m_PowerShootNotified)
		{
			LD35_LOG(LogLD35PlayerController, Log, "Power Shoot !");
			m_GameInputComponent->SetWantsToPowerShoot(true);
		}
		else
		{
			LD35_LOG(LogLD35PlayerController, Log, "Shoot !");
			m_GameInputComponent->SetWantsToShoot(true);
		}

		if (m_PowerShootChargingNotified)
		{
			GetLD35Character()->OnPowerShootStopCharging.Broadcast();
		}

		// Reset shot variables
		m_StartShootTime = -1;
		m_PowerShootNotified = false;
		m_PowerShootChargingNotified = false;
	}
}

void ALD35PlayerController::OnDeflectStart()
{
	if (ValidateGameInputComponent())
	{
		LD35_LOG(LogLD35PlayerController, Log, "Deflect");
		m_GameInputComponent->SetWantsToDeflect(true);
	}
}

void ALD35PlayerController::OnDeflectStop()
{
	if (ValidateGameInputComponent())
	{
		LD35_LOG(LogLD35PlayerController, Log, "Stop Deflect");
		m_GameInputComponent->SetWantsToDeflect(false);
	}
}

void ALD35PlayerController::OnFollowStart()
{
	ServerCallMinions();
	if (ValidateGameInputComponent())
	{
		LD35_LOG(LogLD35PlayerController, Log, "Follow");
		m_GameInputComponent->SetWantsToAllyFollow(true);
	}
}

void ALD35PlayerController::OnFollowStop()
{
	if (ValidateGameInputComponent())
	{
		LD35_LOG(LogLD35PlayerController, Log, "Follow Stop");
		m_GameInputComponent->SetWantsToAllyFollow(false);
	}
}

void ALD35PlayerController::OnReturnStart()
{
	ServerReleaseMinions();
	if (ValidateGameInputComponent())
	{
		m_GameInputComponent->SetWantsToAllyReturn(true);
	}
}

void ALD35PlayerController::OnReturnStop()
{
	if (ValidateGameInputComponent())
	{
		m_GameInputComponent->SetWantsToAllyReturn(false);
	}
}

void ALD35PlayerController::OnStartGameStart()
{
	ServerPlayerReady();
}

void ALD35PlayerController::OnStartGameStop()
{

}

void ALD35PlayerController::OnChangeCamera()
{
	if (m_CameraType == ECameraType::TopDown)
	{
		SetCameraType(ECameraType::Behind);
	}
	else if (m_CameraType == ECameraType::Behind)
	{
		SetCameraType(ECameraType::TopDown);
	}
	else
	{
		check(false);
	}
}

bool ALD35PlayerController::ServerPlayerReady_Validate()
{
	return true;
}

void ALD35PlayerController::ServerPlayerReady_Implementation()
{
	MyPlayerState = ELD35PlayerState::Ready;
}

void ALD35PlayerController::OnRep_MyPlayerState()
{
	if (MyPlayerState == ELD35PlayerState::Ready)
	{
		if (ValidateGameInputComponent())
		{
			m_GameInputComponent->SetWantsToStartGame(true);
		}
	}
}

void ALD35PlayerController::SetCameraType(ECameraType CameraType)
{
	if (m_CameraType != CameraType)
	{
		m_CameraType = CameraType;

		if (m_CameraType == ECameraType::TopDown)
		{
			bShowMouseCursor = true;
		}
		else if (m_CameraType == ECameraType::Behind)
		{
			bShowMouseCursor = false;
		}
		else
		{
			check(false);
		}

		ALD35Character* character = GetLD35Character();
		if (IsValid(character))
		{
			character->OnCameraTypeChanged.Broadcast(m_CameraType);
		}
	}
}

void ALD35PlayerController::UpdateCamera()
{
	if (m_CameraType == ECameraType::TopDown)
	{
		ALD35Character* character = GetLD35Character();
		if (IsValid(character))
		{
			FVector2D mouseLocation;
			GetMousePosition(mouseLocation.X, mouseLocation.Y);

			FVector2D characterLocation;
			if (!m_PreviousMousePosition.Equals(mouseLocation, 0.1f) && ProjectWorldLocationToScreen(character->GetActorLocation(), characterLocation))
			{
				m_PreviousMousePosition = mouseLocation;
				FVector2D toMouse = mouseLocation - characterLocation;
				toMouse.Normalize();
				OnLookAt(toMouse);
			}
		}
	}
	else if (m_CameraType == ECameraType::Behind)
	{

	}
	else
	{
		check(false);
	}
}

bool ALD35PlayerController::ServerCallMinions_Validate()
{
	return true;
}

void ALD35PlayerController::ServerCallMinions_Implementation()
{
	OnCallMinions.Broadcast();
}

bool ALD35PlayerController::ServerReleaseMinions_Validate()
{
	return true;
}

void ALD35PlayerController::ServerReleaseMinions_Implementation()
{
	OnReleaseMinions.Broadcast();
}

bool ALD35PlayerController::ServerTestTakeDamage_Validate()
{
	return true;
}

void ALD35PlayerController::ServerTestTakeDamage_Implementation()
{
	GetLD35Character()->Damage(1);
}

void ALD35PlayerController::ClientInvalidateGameInputComponent_Implementation()
{
	m_GameInputComponent = nullptr;
}

void ALD35PlayerController::TransitionToPlayingCamera()
{
	AActor* currentViewTarget = GetViewTarget();
	ALD35Character* owner = GetLD35Character();
	owner->OnCameraTypeChanged.Broadcast(m_CameraType);
	SetViewTargetWithBlend(owner, 0.8f);
}
