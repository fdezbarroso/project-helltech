#include "Characters/Helltech/Masilla.h"

#include "NavigationSystem.h"
#include "VectorTypes.h"
#include "Components/CapsuleComponent.h"
#include "Game/EnemyAIController.h"
#include "Game/ZoneActivableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnitConversion.h"

AMasilla::AMasilla()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass();
}

void AMasilla::BeginPlay()
{
	Super::BeginPlay();

}

void AMasilla::HandleAttack() {
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

	Attack(Player);

	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownHandle, 
		this,
		&AMasilla::RestartCollision,
		AttackCooldown, 
		false
	);
}

void AMasilla::Attack(ACharacter* PlayerTarget)
{
	if (bDisableEnemyCollisions)
	{
		SetEnemyCollisionEnabled(false);
	}

	UGameplayStatics::ApplyDamage(PlayerTarget, Damage, AIController, this, nullptr);

	if (bDisableEnemyCollisions)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			FTimerDelegate::CreateUObject(this, &AMasilla::SetEnemyCollisionEnabled, true),
			0.2f,
			false
		);
	}
}

void AMasilla::SetEnemyCollisionEnabled(bool bEnabled)
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