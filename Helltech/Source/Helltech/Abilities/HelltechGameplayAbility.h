#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HelltechGameplayAbility.generated.h"

enum class EHelltechAbilityID : uint8;

/**
 * Base Gameplay Ability Class.
 * Holds IDs to map abilities in the Gameplay Ability System can be extended to hold other related core information.
 * Inherit from this class to create new Gameplay Abilities.
 */
UCLASS()
class HELLTECH_API UHelltechGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHelltechGameplayAbility();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	EHelltechAbilityID AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldown")
	FScalableFloat CooldownDuration;

	// Add tags for each of the abilities' CooldownTags through their corresponding blueprint editors.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldown")
	FGameplayTagContainer CooldownTags;

	FGameplayTagContainer TempCooldownTags;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) const override;
};
