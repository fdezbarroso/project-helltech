#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HelltechPlayerState.generated.h"

/**
 * Player State that manages persistent data.
 * Can be used to track player progression, statistics, and any other data that should persist between character
 * respawns or level transition.
 */
UCLASS()
class HELLTECH_API AHelltechPlayerState : public APlayerState
{
	GENERATED_BODY()
};
