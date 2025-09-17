#pragma once

#include "CoreMinimal.h"
#include "Characters/Helltech/Masilla.h"
#include "Tanque.generated.h"

UCLASS()
class HELLTECH_API ATanque : public AMasilla
{
	GENERATED_BODY()

public:
	ATanque();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Tanque")
	float KnocbackForce = 1500.0f;

	UPROPERTY(EditAnywhere, Category="Tanque")
	float KnockbackInZ = 0.5f;

	virtual void HandleAttack() override;

private:
	FTimerHandle AttackCooldownHandle;

	UFUNCTION(BlueprintCallable, Category = "Masilla")
	void AttackTank(ACharacter* Player);
	
	void SetEnemyCollisionEnabled(bool bEnabled);
	void RestartCollision()
	{
		SetEnemyCollisionEnabled(true);
	}
};
