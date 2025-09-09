#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HelltechMovementComponent.generated.h"

/**
 * Custom character movement component to control HOW the player moves.
 * Introduces some game-feel improvements like coyote time, jump buffering, and custom acceleration curves.
 */
UCLASS()
class HELLTECH_API UHelltechMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UHelltechMovementComponent();

	// This curve lets us fine-tune acceleration. X-axis is speed ([0-1] walk, (1-2] sprint),
	// Y-axis is the acceleration multiplier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | Plane")
	UCurveFloat* AccelerationCurve;

	// Jump velocity cutoff factor on button release.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump")
	float JumpCutoffFactor;

	// Higher gravity when falling to make for a less floaty, snappier jump.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump")
	float FallingGravityScale;

	// How quickly the player stops upon landing.
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BrakingDecelerationLanding;

	void SetBaseWalkSpeed(float InWalkSpeed);

	void SetBaseSprintSpeed(float InSprintSpeed);

	void SetIsSprinting(bool NewIsSprinting);

	// Starts a timer to 'buffer' a jump. If the player lands before it runs out, they'll jump automatically.
	void TryBufferJump();

	// Have they just walked off a ledge and can still jump?
	bool IsInCoyoteTime() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual bool CanAttemptJump() const override;

	virtual bool IsFalling() const override;

	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

	virtual bool DoJump(bool bReplayingMoves) override;

private:
	// How long the player can hang in the air after walking off a ledge and still jump.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump", meta = (AllowPrivateAccess = "true"))
	float CoyoteTimeDuration;

	// How early before landing the player can press jump and have it count.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump", meta = (AllowPrivateAccess = "true"))
	float JumpBufferDuration;

	float BaseWalkSpeed;

	float BaseSprintSpeed;

	// Cache the original gravity so we can reset to it.
	float DefaultGravityScale;

	// Flag to stop coyote time from triggering on a regular jump.
	bool IsJumping;

	bool IsSprinting;

	FTimerHandle CoyoteTimeTimerHandle;
	FTimerHandle JumpBufferTimerHandle;
};
