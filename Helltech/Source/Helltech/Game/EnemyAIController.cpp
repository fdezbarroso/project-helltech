#include "Game/EnemyAIController.h"
#include "CoreMinimal.h"
#include "AIController.h"


void AEnemyAIController::MoveToTarget(AActor* Target, float MaxRadius)
{
	if (!Target) return;

    Super::MoveToActor(Target, MaxRadius, true, true, true, nullptr, true);
}

void AEnemyAIController::MoveToLocationAI(const FVector& Location, float MaxRadius)
{
	Super::MoveToLocation(Location, MaxRadius, true, true, true, false, nullptr, true);
}