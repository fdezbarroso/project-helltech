#include "HelltechAttributeSet.h"

void UHelltechAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxMoveSpeedAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("Set MaxMoveSpeed attribute to: %f"), NewValue);
	}
}
