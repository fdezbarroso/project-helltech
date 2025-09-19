#include "Characters/Helltech/EnemyBase.h"

#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Game/ZoneActivableComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	
	AIController = Cast<AAIController>(GetController());
	Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	CurrentState = EEnemyState::Idle;
	bHasTargetPatrol = false;
	IdleTimer = 0.0f;
	Health = BaseHealth;
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	OnZoneActivated();

	OriginalMaterial = GetMesh()->GetMaterial(0);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EEnemyState::Idle:
		HandleIdle(DeltaTime);
		break;
	case EEnemyState::Patrol:
		HandlePatrol();
		break;
	case EEnemyState::Attack:
		HandleAttack();
		break;
	case EEnemyState::Chase:
		HandleChase();
		break;
	case EEnemyState::Dead:
		break;
	}
}

void AEnemyBase::IncreaseDifficulty(float HealthMultiply, float DamageMultiply)
{
	BaseHealth *= FMath::Max(HealthMultiply, 0.0f);
	// Health = BaseHealth;
	Damage *= FMath::Max(DamageMultiply, 0.1f);
}

float AEnemyBase::GetDamage(float Amount)
{
	if (HitMaterial)
	{
		GetMesh()->SetMaterial(0, HitMaterial);
		GetWorldTimerManager().SetTimer(TimerHandle_ResetMaterial, this, &AEnemyBase::ResetMaterial, 0.4f, false);
	}
	
	Health -= Amount;
	if (Health <= 0 && CurrentState != EEnemyState::Dead)
	{
		KillEnemy();
	}
	return Health;
}

void AEnemyBase::ResetMaterial()
{
	if (OriginalMaterial)
	{
		GetMesh()->SetMaterial(0, OriginalMaterial);
	}
}

void AEnemyBase::KillEnemy()
{
	if (CurrentState == EEnemyState::Dead) return;
	
	CurrentState = EEnemyState::Dead;

	if (AIController) {
		AIController->StopMovement();
		AIController->UnPossess();
	} else
	{
		AController* Control = GetController();
		if (Control)
		{
			Control->UnPossess();
		}
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		SkelMesh->SetAllBodiesSimulatePhysics(true);
		SkelMesh->SetSimulatePhysics(true);
		SkelMesh->WakeAllRigidBodies();
		SkelMesh->WakeAllRigidBodies();
		SkelMesh->bBlendPhysics = false;
		SkelMesh->SetAllBodiesPhysicsBlendWeight(1.0f);
	}
	
	SetLifeSpan(7.0f);

	if (SkelMesh && SkelMesh->GetAnimInstance())
	{
		SkelMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SkelMesh->SetAnimInstanceClass(nullptr);
	}
}

void AEnemyBase::HandlePatrol()
{
	if (!bHasTargetPatrol)
	{
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation RandomLocation;
			if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), PatrolRange, RandomLocation))
			{
				PatrolLocation = RandomLocation.Location;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(AIController, PatrolLocation);
				bHasTargetPatrol = true;
			}
		}
	}

	if (bHasTargetPatrol && FVector::Dist(GetActorLocation(), PatrolLocation) <= 100.0f)
	{
		bHasTargetPatrol = false;
		CurrentState = EEnemyState::Idle;
	}

	CheckPlayerDetection();
}

void AEnemyBase::HandleIdle(float DeltaSeconds)
{
	IdleTimer += DeltaSeconds;

	if (IdleTimer >= IdleDuration)
	{
		IdleTimer = 0.0f;
		CurrentState = EEnemyState::Patrol;
		bHasTargetPatrol = false;
	}

	CheckPlayerDetection();
}

void AEnemyBase::HandleChase()
{
	if (!Player)
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Distance > PlayerAwarenessRadius)
	{
		CurrentState = EEnemyState::Idle;
		return;
	}

	if (Distance < AttackRange)
	{
		CurrentState = EEnemyState::Attack;
	}

	if (AIController)
	{
		AIController->MoveToActor(Player );
	}
}

void AEnemyBase::HandleAttack()
{
	if (!Player) {
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Distance > AttackRange) {
		CurrentState = EEnemyState::Chase;
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Attacking"));
}


void AEnemyBase::CheckPlayerDetection() {
	if (!Player) {
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Distance <= PlayerAwarenessRadius)
	{
		CurrentState = EEnemyState::Chase;
	}
}

void AEnemyBase::OnZoneActivated() {
	SetActorTickEnabled(true);

	if (AIController == nullptr)
	{
		AIController = Cast<AAIController>(GetController());
		if (!AIController)
		{
			SpawnDefaultController();
			AIController = Cast<AAIController>(GetController());
		}
	}

	CurrentState = EEnemyState::Idle;
	bHasTargetPatrol = false;
	IdleTimer = 0.0f;
}
