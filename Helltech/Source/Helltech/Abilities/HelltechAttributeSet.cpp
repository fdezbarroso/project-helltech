#include "HelltechAttributeSet.h"

#include "GameplayEffectExtension.h"

void UHelltechAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetCharacterLevelAttribute())
	{
		NewValue = FMath::Max(1, FMath::RoundToInt(NewValue));
		UE_LOG(LogTemp, Warning, TEXT("Set CharacterLevel (1-) attribute to: %f"), NewValue);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max<float>(NewValue, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("Set MoveSpeed (0.0-) attribute to: %f"), NewValue);
	}
	else if (Attribute == GetAccelerationAttribute())
	{
		NewValue = FMath::Max<float>(NewValue, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("Set Acceleration (0.0-) attribute to: %f"), NewValue);
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max<float>(NewValue, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("Set MaxHealth (0.0-) attribute to: %f"), NewValue);
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0.0f, GetMaxHealth());
		UE_LOG(LogTemp, Warning, TEXT("Set Health (0.0-MaxHealth) attribute to: %f"), NewValue);
	}
}

void UHelltechAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float DamageAmount = GetDamage();

		// Set to 0 to ensure future damaging events to use leftover values.
		SetDamage(0.0f);

		if (DamageAmount > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() - DamageAmount, 0.0f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Health loss should go through Damage, avoid this.
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}
