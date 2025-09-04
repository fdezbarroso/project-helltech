#pragma once

#include "CoreMinimal.h"
#include "Characters/Helltech/EnemyBase.h"
#include "Masilla.generated.h"

UENUM(BlueprintType)
enum class EMasillaState : uint8
{
	Patrol UMETA(DisplayName = "Patrol"),
	Chase UMETA(DisplayName = "Chase"),
	Attack UMETA(DisplayName = "Attack"),
	Dead UMETA(DisplayName = "Death")
};

UCLASS()
class HELLTECH_API AMasilla : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMasilla();

	UPROPERTY(EditAnywhere, Category = "Masilla")
	float DetectionDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Masilla")
	float AttackDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Masilla")
	float AttackCooldown = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Masilla")
	bool bDisableEnemyCollisions = true;

	UPROPERTY(EditAnywhere, Category = "Masilla")
	float SightRadius = 800.0f;

	UFUNCTION(BlueprintCallable, Category = "Masilla")
	void SetState(EMasillaState NewState);

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle ThinkTimerHandle;
	FTimerHandle AttackCooldownHandle;
	EMasillaState CurrentState = EMasillaState::Patrol;
	FVector SpawnLocation;

	void Think();
	bool PlayerSighted(ACharacter*& Player, float& OutDistance) const;
	void GoToRandomPoint();
	void StartChase(ACharacter* Player);
	void TryAttack(ACharacter* Player);
	void Attack(ACharacter* Player);
	void SetEnemyCollisionEnabled(bool bEnabled);
};