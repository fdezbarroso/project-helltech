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

	UPROPERTY(EditAnywhere, Category = "Stats")
	float BaseHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float Health = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float Damage = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float Speed = 400.0f;

	UFUNCTION(Category = "Stats")
	virtual void IncreaseDifficulty(float HealthMultiply, float DamageMultiply);

	UFUNCTION(Category = "Stats")
	virtual float GetDamage(float Amount);

	UFUNCTION(Category = "Stats")
	virtual void KillEnemy();
	
protected:
	virtual void BeginPlay() override;
};
