#include "HelltechCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "DataAssets/Characters/HelltechDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/HelltechMovementComponent.h"
#include "UI/DashProgressBarWidget.h"

// We use the ObjectInitializer to replace the default UCharacterMovementComponent with our own.
AHelltechCharacter::AHelltechCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UHelltechMovementComponent>(CharacterMovementComponentName))
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	LastMoveInput = FVector2D::ZeroVector;

	WantsToSprint = false;
}

void AHelltechCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerCamera = GetComponentByClass<UCameraComponent>();
	IsWidgetClassInViewport(GetWorld(), UDashProgressBarWidget::StaticClass());
}

void AHelltechCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsDashing)
	{
		DashElapsedTime += DeltaSeconds;
		float Alpha = DashElapsedTime / DashTime;

		if (Alpha >= 1.f)
		{
			bIsDashing = false;

			GetCharacterMovement()->Velocity = DashCurrentVelocity * FinalInertiaMultiplicator;
			
			GetCharacterMovement()->BrakingFrictionFactor = OriginalBrakingFrictionFactor;
			bTestGroundTouchedAfterDash = true;
			// Si está en el aire, mantener inercia en caida
			if (!GetCharacterMovement()->IsMovingOnGround())
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
			else
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
			if (DashDebug)
			{
				DebugPostDashUntilTouchGround = true;
				DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
			}
		}
		else
		{
			FVector Move = DashDirection * (DashDistance / DashTime) * DeltaSeconds;
			AddActorWorldOffset(Move, true);
		}
	}

	if (bTestGroundTouchedAfterDash && GetCharacterMovement()->IsMovingOnGround())
	{
		bTestGroundTouchedAfterDash = false;
		GetCharacterMovement()->AirControl = OriginalAirControl;
	}

	// DashProgressBar
	if (bCooldownStarted)
	{
		DashCooldownElapsedTime += DeltaSeconds;
		float Alpha = DashCooldownElapsedTime / DashCooldown;
		if (Alpha >= 1.f)
		{
			bCooldownStarted = false;
			DashCooldownElapsedTime = 0.f;
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(0);
			}
		}
		else
		{
			if (DashProgressBar)
			{
				DashProgressBar->SetDashPercentCooldown(1 - Alpha);
			}
		}
	}

#pragma region DEBUG_ZONE
	/* ---------------------- DEBUG ZONE ---------------------- */
	if (DebugPostDashUntilTouchGround && !GetCharacterMovement()->IsMovingOnGround())
	{
		ElapsedTimePostDashUntilTouchGround += DeltaSeconds;
		if (ElapsedTimePostDashUntilTouchGround > CooldownDebugUntilTouchGround)
		{
			ElapsedTimePostDashUntilTouchGround = 0;
			DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), InertiaDebugColor, false, 20);
			DebugLastPlayerPositionUntilTouchGround = GetActorLocation();
		}
	}
	else if (DebugPostDashUntilTouchGround)
	{
		ElapsedTimePostDashUntilTouchGround = 0;
		DebugPostDashUntilTouchGround = false;
		DrawDebugLine(GetWorld(), DebugLastPlayerPositionUntilTouchGround, GetActorLocation(), InertiaDebugColor, false, 20);
	}
#pragma endregion DEBUG_ZONE
}

void AHelltechCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetupAbilitySystemComponent(this, this);

	if (AbilitySystemComponent)
	{
		// Set listeners **before** initialization so they can react to the initial attribute values.
		MoveSpeedChangedDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UHelltechAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AHelltechCharacter::MoveSpeedChanged);

		AccelerationChangedDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UHelltechAttributeSet::GetAccelerationAttribute()).AddUObject(
			this, &AHelltechCharacter::AccelerationChanged);

		HealthChangedDelegate = AbilitySystemComponent->
		                        GetGameplayAttributeValueChangeDelegate(UHelltechAttributeSet::GetHealthAttribute()).
		                        AddUObject(this, &AHelltechCharacter::HealthChanged);

		if (HelltechDataAsset)
		{
			HelltechDataAsset->InitializeAttributes(AbilitySystemComponent);
		}

		InitializeCharacterAbilities();
	}
}

void AHelltechCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* PlayerSubsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputMappingContext)
			{
				PlayerSubsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this,
		                                   &AHelltechCharacter::EnhancedInputMove);

		EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this,
		                                   &AHelltechCharacter::EnhancedInputLook);

		EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this,
		                                   &AHelltechCharacter::EnhancedInputStartSprint);

		EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this,
		                                   &AHelltechCharacter::EnhancedInputStopSprint);

		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this,
		                                   &AHelltechCharacter::EnhancedInputJump);

		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this,
		                                   &AHelltechCharacter::EnhancedInputStopJump);
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveInputAction)
		{
			EnhancedInput->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AHelltechCharacter::DetectMovement);
			EnhancedInput->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &AHelltechCharacter::DetectMovement);
		}
	}
}

void AHelltechCharacter::DetectMovement(const FInputActionValue& Value)
{
	FVector2D result = Value.Get<FVector2D>();
	if (FMath::IsNearlyEqual(result.Y, 1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bUp = true;
	}
	else
	{
		MovementKeys.bUp = false;
	}
	if (FMath::IsNearlyEqual(result.Y, -1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bDown = true;
	}
	else
	{
		MovementKeys.bDown = false;
	}
	if (FMath::IsNearlyEqual(result.X, 1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bRight = true;
	}
	else
	{
		MovementKeys.bRight = false;
	}
	if (FMath::IsNearlyEqual(result.X, -1.f, ForwardMovementCameraTolerance / 100.f))
	{
		MovementKeys.bLeft = true;
	}
	else
	{
		MovementKeys.bLeft = false;
	}
}

void AHelltechCharacter::Dash()
{
	if (!bCanDash || bIsDashing) return;

	//Si tiene DashWithMovement y está en movimiento
	if (DashWithMovement && (GetCharacterMovement()->Velocity.X != 0 || GetCharacterMovement()->Velocity.Y != 0))
	{
		if (MoveInputAction)
		{
			DashDirection = GetActorForwardVector() * (MovementKeys.bUp ? 1 : 0 + MovementKeys.bDown ? -1 : 0) +
				GetActorRightVector() * (MovementKeys.bRight ? 1.f : 0.f + MovementKeys.bLeft ? -1.f : 0.f);
		}
		else
		{
			DashDirection = GetCharacterMovement()->Velocity.GetSafeNormal();
		}
		//Si está mirando hacia abajo y está en el suelo
		if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f)
		{
			//Hacia delante (Z)
			DashDirection.Z = 0;
		}
		else
		{
			//Si se está moviendo hacia delante
			if (!MovementKeys.bDown && !MovementKeys.bRight && !MovementKeys.bLeft)
			{
				//Hacia donde esté mirando (Z)
				DashDirection.Z = PlayerCamera->GetForwardVector().Z;
			}
			//Si se está moviendo hacia otra dirección (esquivar)
			else
			{
				DashDirection.Z = 0;
			}
		}
	}
	//Si está en el suelo y mirando hacia abajo y no se está moviendo
	else if (GetCharacterMovement()->IsMovingOnGround() && PlayerCamera->GetComponentRotation().Pitch < 0.f && GetCharacterMovement()->Velocity.IsNearlyZero())
	{
		//Dash hacia delante
		DashDirection = GetActorForwardVector().GetSafeNormal();
	}
	//Si no está en el suelo
	else
	{
		//Si tiene camara te mueves hacia donde apunte la camara, sino hacia delante
		DashDirection = PlayerCamera ? PlayerCamera->GetForwardVector().GetSafeNormal() : GetActorForwardVector().GetSafeNormal();
	}
	
	if (DashDebug)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + DashDirection*DashDistance, DashDebugColor, false, 20);
	}

	bIsDashing = true;
	bCanDash = false;
	bCooldownStarted = true;
	DashElapsedTime = 0.f;

	OriginalBrakingFrictionFactor = GetCharacterMovement()->BrakingFrictionFactor;
	OriginalAirControl = GetCharacterMovement()->AirControl;
	
	GetCharacterMovement()->BrakingFrictionFactor = 0;
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->AirControl = 1.0f;

	DashCurrentVelocity = DashDirection * (DashDistance / DashTime);

	// Cooldown
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		this,
		&AHelltechCharacter::ResetDash,
		DashCooldown,
		false
	);
}

void AHelltechCharacter::ResetDash()
{
	bCanDash = true;
}

bool AHelltechCharacter::IsWidgetClassInViewport(UWorld* World, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!World || !*WidgetClass)
	{
		return false;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, WidgetClass, false);

	if (FoundWidgets.Num() > 0)
	{
		DashProgressBar = Cast<UDashProgressBarWidget>(FoundWidgets[0]);
		return true;
	}
	
	return false;
}

void AHelltechCharacter::StopJumping()
{
	Super::StopJumping();

	// If we're still moving up, cut our vertical velocity to get a shorter jump.
	if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
	{
		if (HelltechMovement->Velocity.Z > 0.0f)
		{
			HelltechMovement->Velocity.Z *= HelltechMovement->JumpCutoffFactor;
		}
	}
}

void AHelltechCharacter::EnhancedInputMove(const FInputActionValue& InputValue)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D CurrentMoveVector = InputValue.Get<FVector2D>();

	LastMoveInput = CurrentMoveVector;

	// Stop sprinting if the player tries to move backwards.
	if (CurrentMoveVector.Y <= 0.0f && WantsToSprint)
	{
		EnhancedInputStopSprint(InputValue);
	}

	const FRotator Rotator = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, Rotator.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, CurrentMoveVector.Y);
	AddMovementInput(RightDirection, CurrentMoveVector.X);
}

void AHelltechCharacter::EnhancedInputLook(const FInputActionValue& InputValue)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D CurrentLookVector = InputValue.Get<FVector2D>();

	AddControllerYawInput(CurrentLookVector.X);
	AddControllerPitchInput(-CurrentLookVector.Y);
}

void AHelltechCharacter::EnhancedInputStartSprint(const FInputActionValue& InputValue)
{
	WantsToSprint = true;
	TryStartSprint();
}

void AHelltechCharacter::EnhancedInputStopSprint(const FInputActionValue& InputValue)
{
	WantsToSprint = false;

	if (CanSprint())
	{
		if (!AbilitySystemComponent || !SprintEffectHandle.IsValid())
		{
			return;
		}

		if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
		{
			HelltechMovement->SetIsSprinting(false);
		}

		// Stop sprinting by removing the gameplay effect.
		AbilitySystemComponent->RemoveActiveGameplayEffect(SprintEffectHandle);
		SprintEffectHandle.Invalidate();
	}
}

void AHelltechCharacter::EnhancedInputJump(const FInputActionValue& InputValue)
{
	// If we can't jump now, buffer it.
	if (CanJump())
	{
		Jump();
	}
	else
	{
		if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
		{
			HelltechMovement->TryBufferJump();
		}
	}
}

void AHelltechCharacter::EnhancedInputStopJump(const FInputActionValue& InputValue)
{
	StopJumping();
}

void AHelltechCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// When we land, check if we should still be sprinting.
	if (PrevMovementMode == MOVE_Falling && GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		if (!WantsToSprint)
		{
			if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
			{
				HelltechMovement->SetIsSprinting(false);
			}

			if (AbilitySystemComponent && SprintEffectHandle.IsValid())
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(SprintEffectHandle);
				SprintEffectHandle.Invalidate();
			}
		}
		else
		{
			TryStartSprint();
		}
	}
}

void AHelltechCharacter::MoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	const float MaxMoveSpeed = Data.NewValue;

	if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
	{
		HelltechMovement->MaxWalkSpeed = MaxMoveSpeed;

		// We check the Gameplay Effect handle to know if we are "sprinting", not the input flag.
		if (SprintEffectHandle.IsValid())
		{
			HelltechMovement->SetBaseSprintSpeed(MaxMoveSpeed);
		}
		else
		{
			HelltechMovement->SetBaseWalkSpeed(MaxMoveSpeed);
		}
	}
}

void AHelltechCharacter::AccelerationChanged(const FOnAttributeChangeData& Data)
{
	const float Acceleration = Data.NewValue;

	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->MaxAcceleration = Acceleration;
	}
}

void AHelltechCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	const float Health = Data.NewValue;

	if (Health <= 0.0f)
	{
		Die();
	}
}

bool AHelltechCharacter::CanSprint() const
{
	if (const UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
	{
		return HelltechMovement->IsMovingOnGround() || HelltechMovement->IsInCoyoteTime();
	}
	return false;
}

void AHelltechCharacter::TryStartSprint()
{
	// Gatekeeper for starting a sprint. Check all conditions before applying the effect.
	if (CanSprint() && LastMoveInput.Y > 0.0f && !SprintEffectHandle.IsValid())
	{
		if (!AbilitySystemComponent || !SprintEffect)
		{
			return;
		}

		if (UHelltechMovementComponent* HelltechMovement = Cast<UHelltechMovementComponent>(GetCharacterMovement()))
		{
			HelltechMovement->SetIsSprinting(true);
		}

		const FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			SprintEffect, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			SprintEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}
