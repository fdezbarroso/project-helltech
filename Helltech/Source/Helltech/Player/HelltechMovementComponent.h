#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/DashProgressBarWidget.h"
#include "HelltechMovementComponent.generated.h"

/**
 * Custom character movement component to control HOW the player moves.
 * Introduces some game-feel improvements like coyote time, jump buffering, custom acceleration curves,
 * and a dash mechanic.
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
	
	void PerformDash();

	UFUNCTION(BlueprintPure, Category="Dash")
	bool IsDashing() const { return bIsDashing; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
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
	bool bIsJumping;
	bool bIsSprinting;

	FTimerHandle CoyoteTimeTimerHandle;
	FTimerHandle JumpBufferTimerHandle;

#pragma region Dash
private:
	void FindDashProgressBar();
	void ResetDash();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (AllowPrivateAccess = "true"))
	float DashDistance = 800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (AllowPrivateAccess = "true"))
	float DashTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (AllowPrivateAccess = "true"))
	float DashCooldown = 3.f;

	// If true, dash follows movement input. If false or no input, it follows the camera.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (AllowPrivateAccess = "true"))
	bool bDashUsesMovementInput = true;

	// The amount of inertia remaining after the dash completes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (AllowPrivateAccess = "true"))
	float FinalInertiaMultiplier = 0.3f;

	bool bIsDashing = false;
	bool bCanDash = true;
	bool bTestGroundTouchedAfterDash = false;
	bool bCooldownStarted = false;

	FVector DashDirection;
	FVector DashCurrentVelocity;
	float DashElapsedTime = 0.f;
	float DashCooldownElapsedTime = 0.f;

	float OriginalBrakingFrictionFactor;
	float OriginalAirControl;
	
	UPROPERTY()
	UDashProgressBarWidget* DashProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG", meta = (AllowPrivateAccess = "true"))
	bool bDashDebug = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG", meta = (AllowPrivateAccess = "true"))
	FColor DashDebugColor = FColor::Red;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash|DEBUG", meta = (AllowPrivateAccess = "true"))
	FColor InertiaDebugColor = FColor::Magenta;

	bool bDebugPostDashUntilTouchGround = false;
	float ElapsedTimePostDashUntilTouchGround = 0.f;
	float CooldownDebugUntilTouchGround = 0.2f;
	FVector DebugLastPlayerPositionUntilTouchGround = FVector::ZeroVector;
#pragma endregion
};
