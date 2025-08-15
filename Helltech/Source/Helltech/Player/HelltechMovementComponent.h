#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HelltechMovementComponent.generated.h"

/**
 * Custom character movement component to control HOW the player moves.
 */
UCLASS()
class HELLTECH_API UHelltechMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UHelltechMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	UCurveFloat* AccelerationCurve;

protected:
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
};
