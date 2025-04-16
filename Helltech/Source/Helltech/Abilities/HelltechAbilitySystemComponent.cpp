#include "HelltechAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

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
