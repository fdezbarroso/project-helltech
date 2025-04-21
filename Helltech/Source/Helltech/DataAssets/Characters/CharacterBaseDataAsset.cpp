#include "CharacterBaseDataAsset.h"

#include "AbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"

void UCharacterBaseDataAsset::InitializeAttributes(UAbilitySystemComponent* AbilitySystemComponent,
                                                   const int Level) const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->
		SetNumericAttributeBase(UHelltechAttributeSet::GetCharacterLevelAttribute(), Level);

	AbilitySystemComponent->SetNumericAttributeBase(UHelltechAttributeSet::GetMoveSpeedAttribute(),
	                                                MoveSpeed.GetValueAtLevel(Level));

	AbilitySystemComponent->SetNumericAttributeBase(UHelltechAttributeSet::GetAccelerationAttribute(),
	                                                Acceleration.GetValueAtLevel(Level));

	AbilitySystemComponent->SetNumericAttributeBase(UHelltechAttributeSet::GetHealthAttribute(),
	                                                MaxHealth.GetValueAtLevel(Level));
}
