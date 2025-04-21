#pragma once

#include "CoreMinimal.h"
#include "Characters/HelltechCharacterBase.h"
#include "HelltechCharacter.generated.h"

struct FOnAttributeChangeData;
class UHelltechDataAsset;
class UInputAction;
class UInputMappingContext;

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
	AHelltechCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UHelltechDataAsset* HelltechDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	UInputAction* MoveInputAction;

	FDelegateHandle MoveSpeedChangedDelegate;
	FDelegateHandle AccelerationChangedDelegate;
	FDelegateHandle HealthChangedDelegate;

	virtual void MoveSpeedChanged(const FOnAttributeChangeData& Data);
	virtual void AccelerationChanged(const FOnAttributeChangeData& Data);
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
};
