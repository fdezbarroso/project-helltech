#pragma once

#include "CoreMinimal.h"
#include "Characters/Helltech/EnemyBase.h"
#include "Masilla.generated.h"

UCLASS()
class HELLTECH_API AMasilla : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMasilla();
	
	UPROPERTY(EditAnywhere, Category = "Masilla")
	float AttackCooldown = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Masilla")
	bool bDisableEnemyCollisions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Masilla")
	bool bGoingToAttack = false;

	virtual void HandleAttack() override;
	

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle AttackCooldownHandle;

	UFUNCTION(BlueprintCallable, Category = "Masilla")
	void Attack(ACharacter* Player);
	
	void SetEnemyCollisionEnabled(bool bEnabled);
	void RestartCollision()
	{
		SetEnemyCollisionEnabled(true);
	}
};