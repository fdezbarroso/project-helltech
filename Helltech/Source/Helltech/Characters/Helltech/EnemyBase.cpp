#include "Characters/Helltech/EnemyBase.h"

#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	Health = BaseHealth;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AEnemyBase::IncreaseDifficulty(float HealthMultiply, float DamageMultiply)
{
	Health *= FMath::Max(HealthMultiply, 0.0f);
	Health = BaseHealth;
	Damage *= FMath::Max(DamageMultiply, 0.1f);
}

float AEnemyBase::GetDamage(float Amount)
{
	Health -= Amount;
	if (Health <= 0)
	{
		KillEnemy();
	}
	return Amount;
}

void AEnemyBase::KillEnemy()
{
	Destroy();
}