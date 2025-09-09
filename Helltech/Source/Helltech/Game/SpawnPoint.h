#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

UCLASS()
class HELLTECH_API ASpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnPoint();

	// Indicates which zone is
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FName SpawnZoneTag;

	// For random selection (1 by default)
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 Weight = 1;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UBillboardComponent* EditorIcon;
#endif
};
