#include "HelltechGameplayAbility.h"

#include "Helltech.h"

UHelltechGameplayAbility::UHelltechGameplayAbility(): AbilityID(EHelltechAbilityID::None)
{
}

const FGameplayTagContainer* UHelltechGameplayAbility::GetCooldownTags() const
{
	// Do not call Super::GetCooldownTags(), this should fully replace it.

	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();

	if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
	{
		MutableTags->AppendTags(*ParentTags);
	}

	MutableTags->AppendTags(CooldownTags);
	return MutableTags;
}

void UHelltechGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo) const
{
	// Do not call Super::ApplyCooldown(), this should fully replace it.

	if (const UGameplayEffect* CooldownGameplayEffect = GetCooldownGameplayEffect())
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
			CooldownGameplayEffect->GetClass(), GetAbilityLevel());

		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Cooldown")),
		                                               CooldownDuration.GetValueAtLevel(GetAbilityLevel()));

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
