#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HelltechPlayerController.generated.h"

/**
 * Player Controller to handle possession and input.
 * Can be extended to implement UI, camera management, non-character related input, player feedback systems, and other
 * related gameplay features.
 */
UCLASS()
class HELLTECH_API AHelltechPlayerController : public APlayerController
{
	GENERATED_BODY()
};
