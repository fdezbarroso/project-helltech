#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EHelltechAbilityID : uint8
{
	// 0 None
	None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	// 0 None
	None UMETA(DisplayName = "None"),

	// 1 Helltech
	Helltech UMETA(DisplayName = "Helltech"),
};
