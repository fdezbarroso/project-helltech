#include "Characters/Helltech/Masilla.h"

#include "NavigationSystem.h"
#include "VectorTypes.h"
#include "Components/CapsuleComponent.h"
#include "Game/EnemyAIController.h"
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

	SpawnLocation = GetActorLocation();
	GetWorldTimerManager().SetTimer(ThinkTimerHandle, this, &AMasilla::Think, 0.2f, true);
	GoToRandomPoint();
}

void AMasilla::SetState(EMasillaState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
}

bool AMasilla::PlayerSighted(ACharacter*& Player, float& OutDistance) const
{
	Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return false;
	OutDistance = FVector::Dist(Player->GetActorLocation(), this->GetActorLocation());
	return OutDistance <= DetectionDistance;
}

void AMasilla::GoToRandomPoint()
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;
	FNavLocation NavLoc;

	if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, SightRadius, NavLoc))
	{
		if (AEnemyAIController* C = Cast<AEnemyAIController>(GetController()))
		{
			C->MoveToLocationAI(NavLoc.Location, 75.0f);
		}
	}
}

void AMasilla::StartChase(ACharacter* Player)
{
	if (AEnemyAIController* C = Cast<AEnemyAIController>(GetController()))
	{
		C->MoveToTarget(Player, AttackDistance - 10.0f);
	}
}

void AMasilla::TryAttack(ACharacter* Player)
{
	if (!Player) return;
	if (GetWorldTimerManager().IsTimerActive(AttackCooldownHandle)) return;

	float Dist = FVector::Dist(Player->GetActorLocation(), GetActorLocation());
	if (Dist <= AttackDistance)
	{
		SetState(EMasillaState::Attack);
		Attack(Player);
	}
}

void AMasilla::Attack(ACharacter* Player)
{
	if (bDisableEnemyCollisions)
	{
		SetEnemyCollisionEnabled(false);
	}

	UGameplayStatics::ApplyDamage(Player, Damage, GetController(), this, nullptr);

	GetWorldTimerManager().SetTimer(AttackCooldownHandle, [this]()
	{
		if (bDisableEnemyCollisions)
		{
			SetEnemyCollisionEnabled(true);
		}
		SetState(EMasillaState::Chase);
		}, AttackCooldown, false);
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

void AMasilla::Think()
{
	if (CurrentState == EMasillaState::Dead) return;

	ACharacter* Player = nullptr;
	float Dist = 0.0f;
	const bool bSee = PlayerSighted(Player, Dist);

	switch (CurrentState)
	{
	case EMasillaState::Patrol:
		if (bSee)
		{
			SetState(EMasillaState::Chase);
			StartChase(Player);
		}
		break;
	case EMasillaState::Chase:
		if (!bSee)
		{
			SetState(EMasillaState::Patrol);
			GoToRandomPoint();
		}
		else
		{
			TryAttack(Player);
		}
		break;
	case EMasillaState::Attack:
		break;
		
	default:
		break;
	}
}