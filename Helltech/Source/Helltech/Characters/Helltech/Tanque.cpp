#include "Characters/Helltech/Tanque.h"

#include "GameFramework/CharacterMovementComponent.h"

ATanque::ATanque() {
	BaseHealth = 300.0f;
	Health = BaseHealth;

	Damage = 40.0f;
}

void ATanque::BeginPlay()
{
	Super::BeginPlay();
}

void ATanque::HandleAttack()
{
	Super::HandleAttack();
	
	Attack(Player);
}


void ATanque::Attack(ACharacter* PlayerTarget)
{
	if (PlayerTarget)
	{
		FVector KnockbackDirection = (PlayerTarget->GetActorForwardVector() - GetActorLocation()).GetSafeNormal();
		KnockbackDirection.Z = KnockbackInZ;
		
		PlayerTarget->LaunchCharacter(KnockbackDirection * KnocbackForce, true, true);
	}
}
