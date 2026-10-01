#pragma once

#include "GameFramework/PlayerController.h"
#include "LD35PlayerController.generated.h"

// Forward Declarations
class UUserWidget;

UENUM(BlueprintType)
enum class ELD35PlayerState : uint8
{
	SplashScreen,
	Ready
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimplePlayerDelegate);

/**
 * Player logic for controller pawn.
 */
UCLASS()
class LD35_API ALD35PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// AActor override
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void Possess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds);
	// End AActor override

	void TransitionToPlayingCamera();

	UFUNCTION(BlueprintPure)
	ECameraType GetCameraType() const { return m_CameraType; }
public:
	UPROPERTY(EditAnywhere)
	float ThirdPersonYawMultiplier = 3.0;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FSimplePlayerDelegate OnCallMinions;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FSimplePlayerDelegate OnReleaseMinions;

	UPROPERTY(ReplicatedUsing=OnRep_MyPlayerState, Transient, BlueprintReadOnly)
	ELD35PlayerState MyPlayerState = ELD35PlayerState::SplashScreen;

protected:
	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLookAt(FVector2D to);

private:
	void OnMoveHorizontal(float val);
	void OnMoveVertical(float val);
	void OnLookUp(float val);
	void OnLookRight(float val);
	void OnShootStart();
	void OnShootStop();
	void OnDeflectStart();
	void OnDeflectStop();
	void OnFollowStart();
	void OnFollowStop();
	void OnReturnStart();
	void OnReturnStop();
	void OnStartGameStart();
	void OnStartGameStop();
	void OnChangeCamera();
	void OnBehindLookRight(float val);

	void UpdateMovementFromLocalDirection();
	void UpdateYawFromScreenSpaceDirection();

	void SetCameraType(ECameraType CameraType);
	void UpdateCamera();

	ALD35Character* GetLD35Character() const;

	void SetDefaultCamera();
	bool ValidateGameInputComponent();
	
	// RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayerReady();

	//#if WITH_EDITOR
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTestTakeDamage();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCallMinions();
	
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerReleaseMinions();

	UFUNCTION(Reliable, Client)
	void ClientInvalidateGameInputComponent();
	//#endif

	// OnRep
	UFUNCTION()
	void OnRep_MyPlayerState();

private:
	class UGameInputComponent* m_GameInputComponent;

	float m_StartShootTime = -1.0f;
	bool  m_PowerShootNotified = false;
	bool  m_PowerShootChargingNotified = false;
	ECameraType m_CameraType = ECameraType::TopDown;
	FVector2D m_PreviousMousePosition = FVector2D::ZeroVector;
	FVector m_LocalMovementDirection = FVector::ZeroVector;
	FVector m_ScreenSpaceLookDirection = FVector::ZeroVector;
};
