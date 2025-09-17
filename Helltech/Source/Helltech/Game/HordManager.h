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
struct FEnemySpawnEntry
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FHordeWave
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemySpawnEntry> Enemies;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayBeforeWave = 1.5f;
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
	FTimerHandle WaitForAllDeadTimer;
	int32 RemainingToSpawn = 0;
	FHordeWave ActiveWave;

	int32 CurrentSpawnIndex = 0;

	int32 RemainingOfCurrentType = 0;
	
	// For avoiding starting horde more than once when triggering a zone
	UPROPERTY(EditAnywhere, Category="Hordes")
	bool bHordeRunning = false;

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
