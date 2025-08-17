#include "Player/HelltechMovementComponent.h"

#include "GameFramework/Character.h"

UHelltechMovementComponent::UHelltechMovementComponent() : AccelerationCurve(nullptr), JumpCutoffFactor(0.0f),
                                                           FallingGravityScale(0.0f), CoyoteTimeDuration(0.0f),
                                                           JumpBufferDuration(0.0f), DefaultGravityScale(0.0f),
                                                           IsJumping(false)
{
}

void UHelltechMovementComponent::TryBufferJump()
{
	GetWorld()->GetTimerManager().SetTimer(JumpBufferTimerHandle, JumpBufferDuration, false);
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

	// No input, break.
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
		Velocity = Velocity - (Velocity - AccelerationDirection * VelocitySize) *
			FMath::Min(DeltaTime * Friction, 1.0f);

		float FinalAcceleration = CurrentMaxAcceleration;
		if (AccelerationCurve && CurrentMaxSpeed > 0.0f)
		{
			const float SpeedRatio = FMath::Clamp(Velocity.Size() / CurrentMaxSpeed, 0.0f, 1.0f);

			const float CurveMultiplier = AccelerationCurve->GetFloatValue(SpeedRatio);

			FinalAcceleration *= CurveMultiplier;
		}

		Velocity += AccelerationDirection * FinalAcceleration * DeltaTime;
	}

	Velocity = Velocity.GetClampedToMaxSize(CurrentMaxSpeed);
}

void UHelltechMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (!IsJumping && PreviousMovementMode == MOVE_Walking && MovementMode == MOVE_Falling)
	{
		GetWorld()->GetTimerManager().SetTimer(CoyoteTimeTimerHandle, CoyoteTimeDuration, false);
	}

	if (PreviousMovementMode == MOVE_Falling && MovementMode == MOVE_Walking)
	{
		// Reset needed due to it being changes in PhysFalling
		GravityScale = DefaultGravityScale;

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
	const bool InCoyoteTime = GetWorld()->GetTimerManager().IsTimerActive(CoyoteTimeTimerHandle);

	const bool CanJumpNormally = Super::CanAttemptJump();

	return CanJumpNormally || InCoyoteTime;
}

bool UHelltechMovementComponent::IsFalling() const
{
	if (GetWorld()->GetTimerManager().IsTimerActive(CoyoteTimeTimerHandle))
	{
		return false;
	}

	return Super::IsFalling();
}

void UHelltechMovementComponent::PhysFalling(float deltaTime, int32 Iterations)
{
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
	IsJumping = true;

	const bool DidJump = Super::DoJump(bReplayingMoves);

	if (DidJump)
	{
		GetWorld()->GetTimerManager().ClearTimer(CoyoteTimeTimerHandle);
	}

	IsJumping = false;

	return DidJump;
}
