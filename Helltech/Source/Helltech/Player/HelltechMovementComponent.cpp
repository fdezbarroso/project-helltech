#include "Player/HelltechMovementComponent.h"

#include "GameFramework/Character.h"

UHelltechMovementComponent::UHelltechMovementComponent() : AccelerationCurve(nullptr), JumpCutoffFactor(0.0f),
                                                           FallingGravityScale(0.0f), BrakingDecelerationLanding(0.0f),
                                                           CoyoteTimeDuration(0.0f), JumpBufferDuration(0.0f),
                                                           BaseWalkSpeed(0.0f), BaseSprintSpeed(0.0f),
                                                           DefaultGravityScale(0.0f), IsJumping(false),
                                                           IsSprinting(false)
{
}

void UHelltechMovementComponent::SetBaseWalkSpeed(float NewWalkSpeed)
{
	BaseWalkSpeed = NewWalkSpeed;
}

void UHelltechMovementComponent::SetBaseSprintSpeed(float NewSprintSpeed)
{
	BaseSprintSpeed = NewSprintSpeed;
}

void UHelltechMovementComponent::SetIsSprinting(bool NewIsSprinting)
{
	IsSprinting = NewIsSprinting;
}

void UHelltechMovementComponent::TryBufferJump()
{
	GetWorld()->GetTimerManager().SetTimer(JumpBufferTimerHandle, JumpBufferDuration, false);
}

bool UHelltechMovementComponent::IsInCoyoteTime() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(CoyoteTimeTimerHandle);
}

void UHelltechMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultGravityScale = GravityScale;
}

void UHelltechMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(CoyoteTimeTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(JumpBufferTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void UHelltechMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (!HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	Friction = FMath::Max(0.0f, Friction);
	const float CurrentMaxAcceleration = GetMaxAcceleration();
	const float CurrentMaxSpeed = GetMaxSpeed();

	// No input, brake.
	if (Acceleration.IsZero())
	{
		if (Velocity.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			Velocity = FVector::ZeroVector;
		}
		else
		{
			const float ActualBrakingFriction = bUseSeparateBrakingFriction ? BrakingFriction : Friction;
			ApplyVelocityBraking(DeltaTime, ActualBrakingFriction, BrakingDeceleration);
		}
	}
	// Input, accelerate.
	else
	{
		const FVector AccelerationDirection = Acceleration.GetSafeNormal();
		const float VelocitySize = Velocity.Size();

		// Apply friction only to our sideways velocity.
		// Lets us make sharp turns without losing all our forward speed.
		Velocity = Velocity - (Velocity - AccelerationDirection * VelocitySize) *
			FMath::Min(DeltaTime * Friction, 1.0f);

		float FinalAcceleration = CurrentMaxAcceleration;
		if (AccelerationCurve && CurrentMaxSpeed > 0.0f)
		{
			float SpeedRatio = 0.0f;

			if (IsSprinting)
			{
				// For the curve, walk is [0-1] and sprint is (1-2].
				// Lets us define a different 'feel' for sprinting in the same curve.
				if (BaseSprintSpeed > BaseWalkSpeed)
				{
					const float SpeedInRange = FMath::Clamp(VelocitySize, BaseWalkSpeed, BaseSprintSpeed);
					const float RangeCompletion = (SpeedInRange - BaseWalkSpeed) / (BaseSprintSpeed - BaseWalkSpeed);
					SpeedRatio = 1.0f + RangeCompletion;
				}
				else
				{
					SpeedRatio = 1.0f;
				}
			}
			else
			{
				SpeedRatio = FMath::Clamp(Velocity.Size() / BaseWalkSpeed, 0.0f, 1.0f);
			}

			const float CurveMultiplier = AccelerationCurve->GetFloatValue(SpeedRatio);
			FinalAcceleration *= CurveMultiplier;
		}

		Velocity += AccelerationDirection * FinalAcceleration * DeltaTime;

		const float CurrentVelocitySize = Velocity.Size();
		if (CurrentVelocitySize > CurrentMaxSpeed)
		{
			// If we've gone over our max speed (f.e. Sprint), gently brake back down.
			const FVector VelocityDirection = Velocity.GetSafeNormal();
			const float BrakeSpeed = FMath::Max(0.0f, BrakingDecelerationWalking * DeltaTime);
			Velocity = VelocityDirection * FMath::Max(CurrentMaxSpeed, CurrentVelocitySize - BrakeSpeed);
		}
	}
}

void UHelltechMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// If we just walked off a ledge (and didn't jump), start a timer that
	// gives us a short window to jump anyway.
	if (!IsJumping && PreviousMovementMode == MOVE_Walking && MovementMode == MOVE_Falling)
	{
		GetWorld()->GetTimerManager().SetTimer(CoyoteTimeTimerHandle, CoyoteTimeDuration, false);
	}

	if (PreviousMovementMode == MOVE_Falling && MovementMode == MOVE_Walking)
	{
		// Landed, reset gravity back to normal.
		GravityScale = DefaultGravityScale;

		// Player wanted to jump right as we landed.
		if (GetWorld()->GetTimerManager().IsTimerActive(JumpBufferTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(JumpBufferTimerHandle);
			if (CharacterOwner)
			{
				CharacterOwner->Jump();
			}
		}
	}
}

bool UHelltechMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsInCoyoteTime();
}

bool UHelltechMovementComponent::IsFalling() const
{
	if (IsInCoyoteTime())
	{
		return false;
	}

	return Super::IsFalling();
}

void UHelltechMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
	// On the way down from a jump, crank up gravity to make it feel snappier.
	// On the way up, use normal gravity.
	if (Velocity.Z < 0.0f)
	{
		GravityScale = FallingGravityScale;
	}
	else
	{
		GravityScale = DefaultGravityScale;
	}

	Super::PhysFalling(deltaTime, Iterations);
}

bool UHelltechMovementComponent::DoJump(bool bReplayingMoves)
{
	// Flag that we're in the middle of a jump. This stops OnMovementModeChanged
	// from starting coyote time if we jump off a ledge.
	IsJumping = true;

	const bool DidJump = Super::DoJump(bReplayingMoves);

	if (DidJump)
	{
		GetWorld()->GetTimerManager().ClearTimer(CoyoteTimeTimerHandle);
	}

	IsJumping = false;

	return DidJump;
}
