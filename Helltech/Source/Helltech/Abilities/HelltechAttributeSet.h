#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "HelltechAttributeSet.generated.h"

#define HELLTECH_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base Attribute Set class that defines core character stats.
 * Manages character level and movement attributes with appropriate validation. Extend this class to add health,
 * damage, experience, and any other game-specific stats.
 */
UCLASS()
class HELLTECH_API UHelltechAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData CharacterLevel;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, CharacterLevel)

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MoveSpeed;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData Acceleration;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, Acceleration)

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData MaxHealth;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData Health;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, Health)

	// Transient attribute, used for calculations, not as the character's base damage.
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage;
	HELLTECH_ATTRIBUTE_ACCESSORS(UHelltechAttributeSet, Damage)
};
