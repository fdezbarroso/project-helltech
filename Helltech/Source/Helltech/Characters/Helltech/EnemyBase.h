#pragma once

#include "CoreMinimal.h"
#include "Game/EnemyAIController.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Chase UMETA(DisplayName = "Chase"),
	Attack UMETA(DisplayName = "Attack"),
	Dead UMETA(DisplayName = "Death")
};

UCLASS()
class HELLTECH_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	ACharacter* Player;
	
	// Current State of Enemy in FSM
	EEnemyState CurrentState;

	// For handling patrolling
	bool bHasTargetPatrol;
	FVector PatrolLocation;

	// Max radius for locating player
	UPROPERTY(EditAnywhere, Category="AI")
	float PlayerAwarenessRadius = 1000.0f;

	// Attack range
	UPROPERTY(EditAnywhere, Category="AI")
	float AttackRange = 150.0f;

	// Patrol range
	UPROPERTY(EditAnywhere, Category="AI")
	float PatrolRange = 1000.0f;

	// AIController
	AAIController* AIController;

	// Idle Timer
	float IdleTimer;
	UPROPERTY(EditAnywhere, Category="AI")
	float IdleDuration = 2.0f;
	
	// Max health of enemy
	UPROPERTY(EditAnywhere, Category = "Stats")
	float BaseHealth = 100.0f;

	// Current health of enemy
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float Health = 50.0f;

	// Damage enemy does
	UPROPERTY(EditAnywhere, Category = "Stats")
	float Damage = 50.0f;

	// Movement speed of enemy
	UPROPERTY(EditAnywhere, Category = "Stats")
	float Speed = 400.0f;

	// Makes enemy stronger
	UFUNCTION(Category = "Stats")
	virtual void IncreaseDifficulty(float HealthMultiply, float DamageMultiply);

	// To get damage from any source
	UFUNCTION(Category = "Stats")
	virtual float GetDamage(float Amount);

	// Make enemy die
	UFUNCTION(Category = "Stats")
	virtual void KillEnemy();

	UFUNCTION()
	virtual void OnZoneActivated();
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	FVector SpawnLocation;
	
	UFUNCTION(Category = "AI")
	virtual void HandlePatrol();

	UFUNCTION(Category = "AI")
	virtual void HandleIdle(float DeltaSeconds);

	UFUNCTION(Category = "AI")
	virtual void HandleChase();

	UFUNCTION(Category = "AI")
	virtual void HandleAttack();
	
	UFUNCTION()
	virtual void CheckPlayerDetection();


};
