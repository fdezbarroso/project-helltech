#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Engine/DataAsset.h"
#include "CharacterBaseDataAsset.generated.h"

class UAbilitySystemComponent;

/**
 * Data asset to hold the initial values for attributes common to all characters and their initialization function.
 */
UCLASS()
class HELLTECH_API UCharacterBaseDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UCharacterBaseDataAsset(): MoveSpeed(0.0f), Acceleration(0.0f), MaxHealth(0.0f)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common | Movement")
	FScalableFloat MoveSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common | Movement")
	FScalableFloat Acceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common | Health")
	FScalableFloat MaxHealth;

	virtual void InitializeAttributes(UAbilitySystemComponent* AbilitySystemComponent, const int Level = 1) const;
};
