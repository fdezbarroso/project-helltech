#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "HelltechAbilitySystemComponent.generated.h"

/**
 * Base Ability System Component for all characters.
 * Tracks ability initialization and effect application state. Currently minimal, but can be extended to handle damage
 * events, cooldown management, and other ability system interactions.
 */
UCLASS()
class HELLTECH_API UHelltechAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UHelltechAbilitySystemComponent();

	bool GetCharacterAbilitiesGiven() const;

	void SetCharacterAbilitiesGiven(const bool AbilitiesGiven);

	UFUNCTION(BlueprintCallable, Category = "GameplayCue")
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag,
	                             const FGameplayCueParameters& GameplayCueParameters) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayCue")
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag,
	                         const FGameplayCueParameters& GameplayCueParameters) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayCue")
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag,
	                            const FGameplayCueParameters& GameplayCueParameters) const;

protected:
	bool CharacterAbilitiesGiven;
};
