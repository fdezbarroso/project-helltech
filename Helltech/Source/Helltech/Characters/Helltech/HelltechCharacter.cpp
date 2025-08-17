#include "HelltechCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "DataAssets/Characters/HelltechDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/HelltechMovementComponent.h"

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

void AHelltechCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetupAbilitySystemComponent(this, this);

	if (AbilitySystemComponent)
	{
		// Set listeners before initialization for attributes that need to set initial character values.
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
}

void AHelltechCharacter::StopJumping()
{
	Super::StopJumping();

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

	// Negate pitch because inverted Y-axis is for weird people :)
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

		AbilitySystemComponent->RemoveActiveGameplayEffect(SprintEffectHandle);
		SprintEffectHandle.Invalidate();
	}
}

void AHelltechCharacter::EnhancedInputJump(const FInputActionValue& InputValue)
{
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
