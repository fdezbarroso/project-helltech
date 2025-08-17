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

void AHelltechCharacter::MoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	const float MaxMoveSpeed = Data.NewValue;

	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->MaxWalkSpeed = MaxMoveSpeed;
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
