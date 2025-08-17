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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement | Plane")
	UCurveFloat* AccelerationCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump")
	float JumpCutoffFactor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump")
	float FallingGravityScale;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BrakingDecelerationLanding;

	void SetBaseWalkSpeed(float InWalkSpeed);

	void SetBaseSprintSpeed(float InSprintSpeed);

	void SetIsSprinting(bool NewIsSprinting);

	void TryBufferJump();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump", meta = (AllowPrivateAccess = "true"))
	float CoyoteTimeDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement | Jump", meta = (AllowPrivateAccess = "true"))
	float JumpBufferDuration;

	float BaseWalkSpeed;

	float BaseSprintSpeed;

	float DefaultGravityScale;

	bool IsJumping;

	bool IsSprinting;

	FTimerHandle CoyoteTimeTimerHandle;
	FTimerHandle JumpBufferTimerHandle;
};
