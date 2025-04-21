#include "HelltechCharacter.h"

#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"
#include "DataAssets/Characters/HelltechDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

AHelltechCharacter::AHelltechCharacter()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
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
