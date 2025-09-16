#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Characters/HelltechCharacterBase.h"
#include "HelltechCharacter.generated.h"

struct FInputActionValue;
struct FOnAttributeChangeData;
class UCameraComponent;
class UHelltechDataAsset;
class UInputAction;
class UInputMappingContext;
class UHelltechMovementComponent;

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
	explicit AHelltechCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void StopJumping() override;

protected:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	// Callbacks for when our attributes change.
	virtual void MoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void AccelerationChanged(const FOnAttributeChangeData& Data);
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	
	// Input Handlers
	void EnhancedInputMove(const FInputActionValue& InputValue);
	void EnhancedInputLook(const FInputActionValue& InputValue);
	void EnhancedInputStartSprint(const FInputActionValue& InputValue);
	void EnhancedInputStopSprint(const FInputActionValue& InputValue);
	void EnhancedInputJump(const FInputActionValue& InputValue);
	void EnhancedInputStopJump(const FInputActionValue& InputValue);
	void EnhancedInputDash(const FInputActionValue& InputValue);

private:
	// The last known movement input. Used to check if we're trying to sprint backwards.
	FVector2D LastMoveInput;

	// Tracks the player's *intent* to sprint from input, which might not be the same as actually sprinting.
	bool WantsToSprint;
	
	// The conditions required to be able to sprint.
	bool CanSprint() const;
	
	// Checks conditions and applies the sprint effect if possible.
	void TryStartSprint();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* DashInputAction;

	// The Gameplay Effect to apply when we sprint, multiplies max walk speed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> SprintEffect;

	// We use this handle to track if the sprint effect is active and to remove it.
	FActiveGameplayEffectHandle SprintEffectHandle;
	
	FDelegateHandle MoveSpeedChangedDelegate;
	FDelegateHandle AccelerationChangedDelegate;
	FDelegateHandle HealthChangedDelegate;
	
	// Cached reference to our custom movement component.
	UPROPERTY()
	TObjectPtr<UHelltechMovementComponent> HelltechMovementComponent;
};
