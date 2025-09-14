#include "HelltechCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DataAssets/Characters/HelltechDataAsset.h"
#include "Player/HelltechMovementComponent.h"

AHelltechCharacter::AHelltechCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UHelltechMovementComponent>(CharacterMovementComponentName))
{
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	LastMoveInput = FVector2D::ZeroVector;
	WantsToSprint = false;

	HelltechMovementComponent = Cast<UHelltechMovementComponent>(GetCharacterMovement());
}

void AHelltechCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetupAbilitySystemComponent(this, this);

	if (AbilitySystemComponent)
	{
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
		if (MoveInputAction)
		{
			EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AHelltechCharacter::EnhancedInputMove);
		}
		if (LookInputAction)
		{
			EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AHelltechCharacter::EnhancedInputLook);
		}
		if (SprintInputAction)
		{
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Started, this, &AHelltechCharacter::EnhancedInputStartSprint);
			EnhancedInputComponent->BindAction(SprintInputAction, ETriggerEvent::Completed, this, &AHelltechCharacter::EnhancedInputStopSprint);
		}
		if (JumpInputAction)
		{
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Started, this, &AHelltechCharacter::EnhancedInputJump);
			EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Completed, this, &AHelltechCharacter::EnhancedInputStopJump);
		}
		if (DashInputAction)
		{
			EnhancedInputComponent->BindAction(DashInputAction, ETriggerEvent::Started, this, &AHelltechCharacter::EnhancedInputDash);
		}
	}
}

void AHelltechCharacter::StopJumping()
{
	Super::StopJumping();

	if (HelltechMovementComponent && HelltechMovementComponent->Velocity.Z > 0.0f)
	{
		HelltechMovementComponent->Velocity.Z *= HelltechMovementComponent->JumpCutoffFactor;
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

		if (HelltechMovementComponent)
		{
			HelltechMovementComponent->SetIsSprinting(false);
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
	else if (HelltechMovementComponent)
	{
		HelltechMovementComponent->TryBufferJump();
	}
}

void AHelltechCharacter::EnhancedInputStopJump(const FInputActionValue& InputValue)
{
	StopJumping();
}

void AHelltechCharacter::EnhancedInputDash(const FInputActionValue& InputValue)
{
	if (HelltechMovementComponent)
	{
		HelltechMovementComponent->PerformDash();
	}
}

void AHelltechCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (PrevMovementMode == MOVE_Falling && GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		if (!WantsToSprint)
		{
			if (HelltechMovementComponent)
			{
				HelltechMovementComponent->SetIsSprinting(false);
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
	if (HelltechMovementComponent)
	{
		HelltechMovementComponent->MaxWalkSpeed = Data.NewValue;
		if (SprintEffectHandle.IsValid())
		{
			HelltechMovementComponent->SetBaseSprintSpeed(Data.NewValue);
		}
		else
		{
			HelltechMovementComponent->SetBaseWalkSpeed(Data.NewValue);
		}
	}
}

void AHelltechCharacter::AccelerationChanged(const FOnAttributeChangeData& Data)
{
	if (UCharacterMovementComponent* CharMovement = GetCharacterMovement())
	{
		CharMovement->MaxAcceleration = Data.NewValue;
	}
}

void AHelltechCharacter::HealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f)
	{
		Die();
	}
}

bool AHelltechCharacter::CanSprint() const
{
	if (HelltechMovementComponent)
	{
		return HelltechMovementComponent->IsMovingOnGround() || HelltechMovementComponent->IsInCoyoteTime();
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
		
		if (HelltechMovementComponent)
		{
			HelltechMovementComponent->SetIsSprinting(true);
		}

		const FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(SprintEffect, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			SprintEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}
