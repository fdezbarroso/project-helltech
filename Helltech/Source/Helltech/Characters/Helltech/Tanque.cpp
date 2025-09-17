#include "Characters/Helltech/Tanque.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ATanque::ATanque() {
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	Health = BaseHealth;
}

void ATanque::BeginPlay()
{
	Super::BeginPlay();
}

void ATanque::HandleAttack()
{
	if (!Player || GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownHandle))
	{
		return;
	}
	
	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Distance > AttackRange)
	{
		CurrentState = EEnemyState::Chase;
		return;
	}
	
	bGoingToAttack = true;
	
	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownHandle, 
		this,
		&ATanque::RestartCollision,
		AttackCooldown, 
		false
	);
}


void ATanque::AttackTank(ACharacter* PlayerTarget)
{
	if (bDisableEnemyCollisions)
	{
		SetEnemyCollisionEnabled(false);
	}
	
	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Distance <= AttackRange) 
		UGameplayStatics::ApplyDamage(PlayerTarget, Damage, AIController, this, nullptr);
		
	FVector KnockbackDirection = (this->GetActorForwardVector()).GetSafeNormal();
	KnockbackDirection.Z = KnockbackInZ;
	PlayerTarget->LaunchCharacter(KnockbackDirection * KnocbackForce, true, true);

	if (bDisableEnemyCollisions)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			FTimerDelegate::CreateUObject(this, &ATanque::SetEnemyCollisionEnabled, true),
			0.2f,
			false
		);
	}
}

void ATanque::SetEnemyCollisionEnabled(bool bEnabled)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();

	if (!Capsule) return;

	if (bEnabled)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	} else
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}