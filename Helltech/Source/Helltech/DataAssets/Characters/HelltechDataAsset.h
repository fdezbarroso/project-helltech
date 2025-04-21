#pragma once

#include "CoreMinimal.h"
#include "DataAssets/Characters/CharacterBaseDataAsset.h"
#include "HelltechDataAsset.generated.h"

/**
 * Data asset to hold initial values specific to the main character, along their initialization function.
 */
UCLASS()
class HELLTECH_API UHelltechDataAsset : public UCharacterBaseDataAsset
{
	GENERATED_BODY()

public:
	UHelltechDataAsset()
	{
	}

	virtual void
	InitializeAttributes(UAbilitySystemComponent* AbilitySystemComponent, const int Level = 1) const override;
};
