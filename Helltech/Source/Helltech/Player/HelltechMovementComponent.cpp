#include "Player/HelltechMovementComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UHelltechMovementComponent::UHelltechMovementComponent(): AccelerationCurve(nullptr), OriginalBrakingFrictionFactor(0.0f),
                                                          OriginalAirControl(0.0f)
{
	JumpCutoffFactor = 0.5f;
	FallingGravityScale = 2.0f;
	BrakingDecelerationLanding = 1000.0f;
	CoyoteTimeDuration = 0.15f;
	JumpBufferDuration = 0.15f;
	BaseWalkSpeed = 0.0f;
	BaseSprintSpeed = 0.0f;
	DefaultGravityScale = 1.0f;
	
	bIsJumping = false;
	bIsSprinting = false;

	DashProgressBar = nullptr;
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
	bIsSprinting = NewIsSprinting;
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
	FindDashProgressBar();
}

void UHelltechMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void UHelltechMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDashing)
	{
		DashElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(DashElapsedTime / DashTime, 0.0f, 1.0f);

		if (Alpha >= 1.0f)
		{
			bIsDashing = false;
			Velocity = DashCurrentVelocity * FinalInertiaMultiplier;
			
			BrakingFrictionFactor = OriginalBrakingFrictionFactor;
			bTestGroundTouchedAfterDash = true;

			SetMovementMode(IsMovingOnGround() ? MOVE_Walking : MOVE_Falling);

			if (bDashDebug)
			{
				bDebugPostDashUntilTouchGround = true;
				DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
			}
		}
		else
		{
			FVector Move = DashDirection * (DashDistance / DashTime) * DeltaTime;
			if (CharacterOwner)
			{
				FHitResult Hit;
				CharacterOwner->AddActorWorldOffset(Move, true, &Hit);
			}
		}
	}

	if (bTestGroundTouchedAfterDash && IsMovingOnGround())
	{
		bTestGroundTouchedAfterDash = false;
		AirControl = OriginalAirControl;
	}

	if (bCooldownStarted)
	{
		DashCooldownElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(DashCooldownElapsedTime / DashCooldown, 0.0f, 1.0f);

		if (Alpha >= 1.0f)
		{
			bCooldownStarted = false;
			DashCooldownElapsedTime = 0.0f;
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(0.0f);
			}
		}
		else
		{
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(1.0f - Alpha);
			}
		}
	}
	
	if (bDebugPostDashUntilTouchGround)
	{
		if (!IsMovingOnGround())
		{
			ElapsedTimePostDashUntilTouchGround += DeltaTime;
			if (ElapsedTimePostDashUntilTouchGround > CooldownDebugUntilTouchGround)
			{
				ElapsedTimePostDashUntilTouchGround = 0.0f;
				DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), InertiaDebugColor, false, 20);
				DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
			}
		}
		else
		{
			ElapsedTimePostDashUntilTouchGround = 0.0f;
			bDebugPostDashUntilTouchGround = false;
			DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), InertiaDebugColor, false, 20);
		}
	}
}

void UHelltechMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	if (bIsDashing)
	{
		return;
	}

	if (!HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	Friction = FMath::Max(0.0f, Friction);
	const float CurrentMaxAcceleration = GetMaxAcceleration();
	const float CurrentMaxSpeed = GetMaxSpeed();

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
	else
	{
		const FVector AccelerationDirection = Acceleration.GetSafeNormal();
		const float VelocitySize = Velocity.Size();

		Velocity = Velocity - (Velocity - AccelerationDirection * VelocitySize) *
			FMath::Min(DeltaTime * Friction, 1.0f);

		float FinalAcceleration = CurrentMaxAcceleration;
		if (AccelerationCurve && CurrentMaxSpeed > 0.0f)
		{
			float SpeedRatio = 0.0f;

			if (bIsSprinting)
			{
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
			const FVector VelocityDirection = Velocity.GetSafeNormal();
			const float BrakeSpeed = FMath::Max(0.0f, BrakingDecelerationWalking * DeltaTime);
			Velocity = VelocityDirection * FMath::Max(CurrentMaxSpeed, CurrentVelocitySize - BrakeSpeed);
		}
	}
}

void UHelltechMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (!bIsJumping && PreviousMovementMode == MOVE_Walking && MovementMode == MOVE_Falling)
	{
		GetWorld()->GetTimerManager().SetTimer(CoyoteTimeTimerHandle, CoyoteTimeDuration, false);
	}

	if (PreviousMovementMode == MOVE_Falling && MovementMode == MOVE_Walking)
	{
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
	bIsJumping = true;
	const bool DidJump = Super::DoJump(bReplayingMoves);

	if (DidJump)
	{
		GetWorld()->GetTimerManager().ClearTimer(CoyoteTimeTimerHandle);
	}

	bIsJumping = false;
	return DidJump;
}

void UHelltechMovementComponent::PerformDash()
{
	if (!bCanDash || bIsDashing || !CharacterOwner)
	{
		return;
	}

	UCameraComponent* PlayerCamera = CharacterOwner->FindComponentByClass<UCameraComponent>();
	if (!PlayerCamera)
	{
		return;
	}

	const FVector InputVector = GetLastInputVector();

	if (bDashUsesMovementInput && !InputVector.IsNearlyZero())
	{
		DashDirection = InputVector.GetSafeNormal();
	}
	else
	{
		DashDirection = PlayerCamera->GetForwardVector();
	}
	
	if (IsMovingOnGround())
	{
		DashDirection.Z = 0.0f;
	}
	
	DashDirection.Normalize();

	if (bDashDebug)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DashDirection * DashDistance, DashDebugColor, false, 20);
	}

	bIsDashing = true;
	bCanDash = false;
	bCooldownStarted = true;
	DashElapsedTime = 0.0f;

	OriginalBrakingFrictionFactor = BrakingFrictionFactor;
	OriginalAirControl = AirControl;
	
	BrakingFrictionFactor = 0.0f;
	AirControl = 1.0f;
	SetMovementMode(MOVE_Falling);
	Velocity = FVector::ZeroVector;

	DashCurrentVelocity = DashDirection * (DashDistance / DashTime);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UHelltechMovementComponent::ResetDash, DashCooldown, false);
}

void UHelltechMovementComponent::ResetDash()
{
	bCanDash = true;
}

void UHelltechMovementComponent::FindDashProgressBar()
{
	if (!GetWorld())
	{
		return;
	}
	
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UDashProgressBarWidget::StaticClass(), false);

	if (FoundWidgets.Num() > 0)
	{
		DashProgressBar = Cast<UDashProgressBarWidget>(FoundWidgets[0]);
	}
}
