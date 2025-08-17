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
class USpringArmComponent;

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

	virtual void PossessedBy(AController* NewController) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> SprintEffect;

	FActiveGameplayEffectHandle SprintEffectHandle;

	FDelegateHandle MoveSpeedChangedDelegate;
	FDelegateHandle AccelerationChangedDelegate;
	FDelegateHandle HealthChangedDelegate;

	virtual void StopJumping() override;

	void EnhancedInputMove(const FInputActionValue& InputValue);
	void EnhancedInputLook(const FInputActionValue& InputValue);
	void EnhancedInputStartSprint(const FInputActionValue& InputValue);
	void EnhancedInputStopSprint(const FInputActionValue& InputValue);
	void EnhancedInputJump(const FInputActionValue& InputValue);
	void EnhancedInputStopJump(const FInputActionValue& InputValue);

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void MoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void AccelerationChanged(const FOnAttributeChangeData& Data);
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	bool CanSprint() const;

private:
	FVector2D LastMoveInput;

	bool WantsToSprint;

	void TryStartSprint();
};
