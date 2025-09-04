#pragma once

#include "CoreMinimal.h"
#include "SpawnPoint.h"
#include "Characters/Helltech/HelltechCharacter.h"
#include "GameFramework/Actor.h"
#include "HordManager.generated.h"

class AEnemySpawn;
class AZoneTrigger;
class AEnemyBase;

USTRUCT(BlueprintType)
struct FHordeWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayBeforeWave = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FZoneWaves
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHordeWave> Waves;
};

UCLASS()
class HELLTECH_API AHordManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AHordManager();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hordes")
	TMap<FName, FZoneWaves> WavesByZone;

	UPROPERTY(EditAnywhere, Category="Horde")
	float TimeBetweenWaves = 6.0f;

	UPROPERTY(EditAnywhere, Category="Horde")
	bool bWaitAllDead = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horde")
	int32 EnemiesAlive = 0;

	UFUNCTION()
	void OnZoneActivated(FName ZoneTag, bool bStartHordes);

private:
	TArray<ASpawnPoint*> EnemySpawnPoints;
	TArray<ASpawnPoint*> ActiveSpawnPoints;

	FName CurrentZone;
	int32 CurrentWave = -1;
	FTimerHandle WaveTimer;
	FTimerHandle SpawnTimer;
	int32 RemainingToSpawn = 0;
	FHordeWave ActiveWave;

	void RefreshSpawnPoints();
	void SetActiveZone(const FName& ZoneTag);
	void ActivateStaticEnemis(const FName& ZoneTag);
	void BeginZone();
	void StartNextWave();
	void SpawnOne();
	UFUNCTION()
	void OnEnemyDestroyed(AActor* DestroyedActor);
	ASpawnPoint* PickRandomSpawn() const;
};
