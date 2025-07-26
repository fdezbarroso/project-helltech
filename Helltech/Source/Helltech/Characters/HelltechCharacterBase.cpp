#include "HelltechCharacterBase.h"

#include "Helltech.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Abilities/HelltechAttributeSet.h"
#include "Abilities/HelltechGameplayAbility.h"

AHelltechCharacterBase::AHelltechCharacterBase(): AbilitySystemComponent(nullptr), AttributeSet(nullptr),
                                                  CharacterType(ECharacterType::None)
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UHelltechAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UHelltechAttributeSet>(TEXT("AttributeSet"));
}

void AHelltechCharacterBase::ApplyDamageToTarget(AHelltechCharacterBase* Target, const float DamageAmount)
{
	if (!Target || !Target->GetAbilitySystemComponent() || !DamageEffect)
	{
		return;
	}

	const FGameplayEffectSpecHandle DamageEffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		DamageEffect, GetCharacterLevel(), GetAbilitySystemComponent()->MakeEffectContext());

	if (DamageEffectSpecHandle.IsValid())
	{
		DamageEffectSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
		                                                     DamageAmount);

		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data,
		                                                             Target->GetAbilitySystemComponent());
	}
}

UHelltechAbilitySystemComponent* AHelltechCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UHelltechAttributeSet* AHelltechCharacterBase::GetAttributeSet() const
{
	return AttributeSet;
}

int AHelltechCharacterBase::GetAbilityLevel(const EHelltechAbilityID AbilityID) const
{
	if (!AbilitySystemComponent)
	{
		return -1;
	}

	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = AbilitySystemComponent->GetActivatableAbilities();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		if (Spec.Ability)
		{
			const UHelltechGameplayAbility* HelltechAbility = Cast<UHelltechGameplayAbility>(Spec.Ability);
			if (HelltechAbility && HelltechAbility->AbilityID == AbilityID)
			{
				return Spec.Level;
			}
		}
	}

	return -1;
}

int AHelltechCharacterBase::GetCharacterLevel() const
{
	if (!AttributeSet)
	{
		return 0;
	}

	return FMath::RoundToInt(AttributeSet->GetCharacterLevel());
}

float AHelltechCharacterBase::GetMoveSpeed() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return AttributeSet->GetMoveSpeed();
}

float AHelltechCharacterBase::GetAcceleration() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return AttributeSet->GetAcceleration();
}

float AHelltechCharacterBase::GetMaxHealth() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return AttributeSet->GetMaxHealth();
}

float AHelltechCharacterBase::GetHealth() const
{
	if (!AttributeSet)
	{
		return 0.0f;
	}

	return AttributeSet->GetHealth();
}

TSubclassOf<UGameplayEffect> AHelltechCharacterBase::GetDamageEffect() const
{
	return DamageEffect;
}

void AHelltechCharacterBase::SetupAbilitySystemComponent(AActor* OwnerActor, AActor* AvatarActor)
{
	AbilitySystemComponent->InitAbilityActorInfo(OwnerActor, AvatarActor);
}

void AHelltechCharacterBase::InitializeCharacterAbilities()
{
	if (!AbilitySystemComponent || AbilitySystemComponent->GetCharacterAbilitiesGiven())
	{
		return;
	}

	for (TSubclassOf<UHelltechGameplayAbility>& AbilityClass : CharacterAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		UHelltechGameplayAbility* AbilityDefaultObject = AbilityClass.GetDefaultObject();

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass,
		                                                         GetAbilityLevel(AbilityDefaultObject->AbilityID),
		                                                         static_cast<int32>(AbilityDefaultObject->AbilityID),
		                                                         this));
	}

	AbilitySystemComponent->SetCharacterAbilitiesGiven(true);
}

void AHelltechCharacterBase::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s died."), *GetActorNameOrLabel());
}
