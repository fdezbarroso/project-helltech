#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Characters/HelltechCharacterBase.h"
#include "UI/DashProgressBarWidget.h"
#include "HelltechCharacter.generated.h"

struct FInputActionValue;
struct FOnAttributeChangeData;
class UCameraComponent;
class UHelltechDataAsset;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;

USTRUCT()
struct FMovementKeys2D
{
	GENERATED_BODY()
	UPROPERTY()
	bool bRight = false;
	UPROPERTY()
	bool bLeft = false;
	UPROPERTY()
	bool bUp = false;
	UPROPERTY()
	bool bDown = false;
};


/**
 * Player Character class for the main character.
 * Implements input through the Enhanced Input System. Handles Gameplay Ability System initialization during possession.
 * Extend this class to add player-specific abilities, visual effects, and state management.
 */
UCLASS()
class HELLTECH_API AHelltechCharacter : public AHelltechCharacterBase
{
	GENERATED_BODY()

public:
	AHelltechCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	// Holds the data to initialize our character's default attributes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UHelltechDataAsset* HelltechDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* SprintInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* JumpInputAction;

	// The Gameplay Effect to apply when we sprint.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> SprintEffect;

	// We use this handle to track if the sprint effect is active and to remove it.
	FActiveGameplayEffectHandle SprintEffectHandle;

	FDelegateHandle MoveSpeedChangedDelegate;
	FDelegateHandle AccelerationChangedDelegate;
	FDelegateHandle HealthChangedDelegate;

#pragma region Dash
	//Dash variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DashDistance = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DashTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DashCooldown = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	bool DashWithMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float FinalInertiaMultiplicator = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DodgeAngleTolerance = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta=(UIMin=0, UIMax=100, Units="Percent"))
	float ForwardMovementCameraTolerance = 20.f;
	
	bool bIsDashing = false;
	bool bCanDash = true;

	bool bTestGroundTouchedAfterDash = false;
	bool bCooldownStarted = false;

	FVector DashDirection;
	FVector DashCurrentVelocity;
	float DashElapsedTime = 0.f;
	float DashCooldownElapsedTime = 0.f;

	float OriginalBrakingFrictionFactor;
	float OriginalAirControl;

	UPROPERTY()
	UDashProgressBarWidget* DashProgressBar;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamera;
	
	UPROPERTY(VisibleAnywhere)
	FMovementKeys2D MovementKeys;
	

	//Dash functions
	UFUNCTION(BlueprintCallable, Category="Dash")
	void Dash();

	void ResetDash();

#pragma endregion
	bool IsWidgetClassInViewport(UWorld* World, TSubclassOf<UUserWidget> WidgetClass);
	void DetectMovement(const FInputActionValue& Value);

	// Override to implement variable jump height.
	virtual void StopJumping() override;

	void EnhancedInputMove(const FInputActionValue& InputValue);
	void EnhancedInputLook(const FInputActionValue& InputValue);
	void EnhancedInputStartSprint(const FInputActionValue& InputValue);
	void EnhancedInputStopSprint(const FInputActionValue& InputValue);
	void EnhancedInputJump(const FInputActionValue& InputValue);
	void EnhancedInputStopJump(const FInputActionValue& InputValue);

	// Used to re-evaluate sprint state on landing.
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	// Callbacks for when our attributes change.
	virtual void MoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void AccelerationChanged(const FOnAttributeChangeData& Data);
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	// The conditions required to be able to sprint.
	bool CanSprint() const;

private:
	// The last known movement input. Used to check if we're trying to sprint backwards.
	FVector2D LastMoveInput;

	// Tracks the player's *intent* to sprint from input, which might not be the same as actually sprinting.
	bool WantsToSprint;

	// Checks conditions and applies the sprint effect if possible.
	void TryStartSprint();

#pragma region DEBUG_ZONE

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	bool DashDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	FColor DashDebugColor = FColor::Red;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG")
	FColor InertiaDebugColor = FColor::Magenta;
	bool DebugPostDashUntilTouchGround = false;
	float ElapsedTimePostDashUntilTouchGround = 0.f;
	float CooldownDebugUntilTouchGround = 0.2f;
	FVector DebugLastPlayerPositionUntilTouchGround = FVector::ZeroVector;

#pragma endregion
};
