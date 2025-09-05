#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class HELLTECH_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	// Max health of enemy
	UPROPERTY(EditAnywhere, Category = "Stats")
	float BaseHealth = 100.0f;

	// Current health of enemy
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float Health = 50.0f;

	// Damage enemy does
	UPROPERTY(EditAnywhere, Category = "Stats")
	float Damage = 5.0f;

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
	
protected:
	virtual void BeginPlay() override;
};
