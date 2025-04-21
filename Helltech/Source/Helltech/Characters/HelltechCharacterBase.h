#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HelltechCharacterBase.generated.h"

enum class EHelltechAbilityID : uint8;
class UGameplayEffect;
class UHelltechAbilitySystemComponent;
class UHelltechAttributeSet;
class UHelltechGameplayAbility;

/**
* Base Character class for all entities that use the Gameplay Ability System.
 * Directly owns and manages the character's Ability System Component and Attribute Set.
 * Handles ability and effect initialization, attribute management, and provides a common interface for character stats
 * and ability interactions.
 * Inherit from this class for both the player and enemy types.
 */
UCLASS()
class HELLTECH_API AHelltechCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AHelltechCharacterBase();

	UFUNCTION(BlueprintCallable, Category = "Character|Damage")
	virtual void ApplyDamageToTarget(AHelltechCharacterBase* Target, const float DamageAmount);

	virtual UHelltechAbilitySystemComponent* GetAbilitySystemComponent() const;

	virtual UHelltechAttributeSet* GetAttributeSet() const;

	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual int GetAbilityLevel(EHelltechAbilityID AbilityID) const;

	UFUNCTION(BlueprintCallable, Category = "Character | Attributes")
	int GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Character | Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Character | Attributes")
	float GetAcceleration() const;

	UFUNCTION(BlueprintCallable, Category = "Character | Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Character | Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Character | Effects")
	TSubclassOf<UGameplayEffect> GetDamageEffect() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities")
	UHelltechAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Abilities")
	UHelltechAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UHelltechGameplayAbility>> CharacterAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities | Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// Call in BeginPlay for non-player controlled characters, in PossessedBy for player character.
	virtual void SetupAbilitySystemComponent(AActor* OwnerActor, AActor* AvatarActor);

	virtual void InitializeCharacterAbilities();

	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void Die();
};
