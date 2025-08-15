#include "Player/HelltechMovementComponent.h"

#include "GameFramework/Character.h"

UHelltechMovementComponent::UHelltechMovementComponent() : AccelerationCurve(nullptr)
{
}

void UHelltechMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (!HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	Friction = FMath::Max(0.0f, Friction);
	const float MaxAccel = GetMaxAcceleration();
	const float MaxSpeed = GetMaxSpeed();

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

		float FinalAcceleration = MaxAccel;
		if (AccelerationCurve && MaxSpeed > 0.0f)
		{
			const float SpeedRatio = FMath::Clamp(Velocity.Size() / MaxSpeed, 0.0f, 1.0f);

			const float CurveMultiplier = AccelerationCurve->GetFloatValue(SpeedRatio);

			FinalAcceleration *= CurveMultiplier;
		}

		Velocity += AccelerationDirection * FinalAcceleration * DeltaTime;
	}

	Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
}
