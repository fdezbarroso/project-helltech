#include "HelltechAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

UHelltechAbilitySystemComponent::UHelltechAbilitySystemComponent(): CharacterAbilitiesGiven(false)
{
}

bool UHelltechAbilitySystemComponent::GetCharacterAbilitiesGiven() const
{
	return CharacterAbilitiesGiven;
}

void UHelltechAbilitySystemComponent::SetCharacterAbilitiesGiven(const bool AbilitiesGiven)
{
	CharacterAbilitiesGiven = AbilitiesGiven;
}

void UHelltechAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                              const FGameplayCueParameters& GameplayCueParameters) const
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UHelltechAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                          const FGameplayCueParameters& GameplayCueParameters) const
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UHelltechAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag,
                                                             const FGameplayCueParameters& GameplayCueParameters) const
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
		GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}
