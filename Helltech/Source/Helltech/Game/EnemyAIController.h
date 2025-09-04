#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"


UCLASS()
class HELLTECH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	UFUNCTION(Category="AI")
	void MoveToTarget(AActor* Target, float MaxRadius = 80.0f);

	UFUNCTION(Category="AI")
	void MoveToLocationAI(const FVector& Location, float MaxRadius = 80.0f);
};