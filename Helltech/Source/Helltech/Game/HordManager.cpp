#include "Game/HordManager.h"

#include "AbilitySystemComponent.h"
#include "ZoneActivableComponent.h"
#include "ZoneTrigger.h"
#include "Characters//Helltech/EnemyBase.h"
#include "Abilities/HelltechAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

AHordManager::AHordManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHordManager::BeginPlay()
{
	Super::BeginPlay();

	RefreshSpawnPoints();
	
	// TArray<AActor*> Triggers;
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(),AZoneTrigger::StaticClass(), Triggers);
	//
	// for (AActor* A : Triggers)
	// {
	// 	if (AZoneTrigger* Trig = Cast<AZoneTrigger>(A))
	// 	{
	// 		Trig->OnZoneActivated.AddDynamic(this, &AHordManager::OnZoneActivated);
	// 	}
	// }
}

void AHordManager::RefreshSpawnPoints()
{
	ActiveSpawnPoints.Reset();
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ASpawnPoint::StaticClass(),Found);
	for (AActor* A : Found)
	{
		if (ASpawnPoint* sp = Cast<ASpawnPoint>(A))
		{
			ActiveSpawnPoints.Add(sp);
		}
	}
}

void AHordManager::OnZoneActivated(FName ZoneTag, bool bStartHordes)
{
	UE_LOG(LogTemp, Display, TEXT("Zone %s is activated on HordManager"), *ZoneTag.ToString());
	SetActiveZone(ZoneTag);
	ActivateStaticEnemis(ZoneTag);
	if (bStartHordes && !bHordeRunning && WavesByZone.Contains(ZoneTag))
	{
		bHordeRunning = true;
		BeginZone();
	}
}

void AHordManager::SetActiveZone(const FName& ZoneTag)
{
	CurrentZone = ZoneTag;
	ActiveSpawnPoints.Reset();

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ASpawnPoint::StaticClass(),Found);
	
	for (AActor* A : Found)
	{
		UE_LOG(LogTemp, Display, TEXT("Iterating over Spawn Points")); 
		if (ASpawnPoint* sp = Cast<ASpawnPoint>(A))
		{
			if (sp->SpawnZoneTag == CurrentZone)
			{
				UE_LOG(LogTemp, Warning, TEXT("Adding Spawn Point %s"), *sp->SpawnZoneTag.ToString());
				ActiveSpawnPoints.Add(sp);
			}
		}
	}
}

void AHordManager::ActivateStaticEnemis(const FName& ZoneTag)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),AEnemyBase::StaticClass(),Actors);
	for (AActor* A : Actors)
	{
		if (!A) continue;
		UZoneActivableComponent* Comp = A->FindComponentByClass<UZoneActivableComponent>();
		if (Comp)
		{
			Comp->ActivateForZone(ZoneTag);
		}
	}
}

void AHordManager::BeginZone()
{
	CurrentWave = -1;
	StartNextWave();
}

void AHordManager::StartNextWave()
{
	if (!WavesByZone.Contains(CurrentZone)) {
		bHordeRunning = false;
		return;
	}
	const TArray<FHordeWave>& Waves = WavesByZone[CurrentZone].Waves;
	CurrentWave++;

	UE_LOG(LogTemp, Display, TEXT("Starting Wave %d"), CurrentWave);

	if (!Waves.IsValidIndex(CurrentWave))
	{
		bHordeRunning = false;
		GetWorldTimerManager().ClearTimer(SpawnTimer);
		GetWorldTimerManager().ClearTimer(WaveTimer);
		GetWorldTimerManager().ClearTimer(WaitForAllDeadTimer);
		UE_LOG(LogTemp, Display, TEXT("Zone %s: all waves completed."), *CurrentZone.ToString());
		return;
	}

	ActiveWave = Waves[CurrentWave];
	CurrentSpawnIndex = 0;
	RemainingOfCurrentType = (ActiveWave.Enemies.Num() > 0) ? ActiveWave.Enemies[0].Count : 0;
	
	// UE_LOG(LogTemp, Display, TEXT("Zone %s -> Wave %d (Count=%d, Interval=%.2f)"),
	//    *CurrentZone.ToString(), CurrentWave, ActiveWave.Count, ActiveWave.SpawnInterval);

	GetWorldTimerManager().SetTimer(WaveTimer, [this]()
	{
		GetWorldTimerManager().SetTimer(SpawnTimer, this, &AHordManager::SpawnOne, ActiveWave.SpawnInterval, true, 0.f);
	}, ActiveWave.DelayBeforeWave, false);
}

void AHordManager::OnEnemyDestroyed(AActor* DestroyedActor)
{
	EnemiesAlive = FMath::Max(0, --EnemiesAlive);
	UE_LOG(LogTemp, Display, TEXT("Enemy destroyed on horde manager. Alive=%d"), EnemiesAlive);
	
	if (EnemiesAlive <= 0 && bWaitAllDead)
	{
		StartNextWave();
	}
}


void AHordManager::SpawnOne()
{
	if (!ActiveWave.Enemies.IsValidIndex(CurrentSpawnIndex)) {
		GetWorldTimerManager().ClearTimer(SpawnTimer);

		if (bWaitAllDead) {
			GetWorldTimerManager().SetTimer(WaitForAllDeadTimer, [this]()
			{
				if (EnemiesAlive <= 0)
				{
					GetWorldTimerManager().ClearTimer(WaitForAllDeadTimer);
					StartNextWave();
				}
			}, 0.5f, true);
		}
		else {
			GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordManager::StartNextWave, TimeBetweenWaves, false);
		}
		return;
	}

	int32 TotalEnemies = 0;
	for (const FEnemySpawnEntry& E : ActiveWave.Enemies) { TotalEnemies += E.Count; }

	UE_LOG(LogTemp, Display, TEXT("Zone %s -> Wave %d (Total=%d, Interval=%.2f)"),
		*CurrentZone.ToString(), CurrentWave, TotalEnemies, ActiveWave.SpawnInterval);

	const FEnemySpawnEntry& Entry = ActiveWave.Enemies[CurrentSpawnIndex];
	
	if (RemainingOfCurrentType <= 0)
	{
		CurrentSpawnIndex++;
		if (ActiveWave.Enemies.IsValidIndex(CurrentSpawnIndex))
		{
			RemainingOfCurrentType = ActiveWave.Enemies[CurrentSpawnIndex].Count;
		}
		else
		{
			GetWorldTimerManager().ClearTimer(SpawnTimer);

			if (bWaitAllDead)
			{
				GetWorldTimerManager().SetTimer(WaitForAllDeadTimer, [this]()
				{
					if (EnemiesAlive <= 0)
					{
						GetWorldTimerManager().ClearTimer(WaitForAllDeadTimer);
						StartNextWave();
					}
				}, 0.5f, true);
			}
			else
			{
				GetWorldTimerManager().SetTimer(WaveTimer, this, &AHordManager::StartNextWave, TimeBetweenWaves, false);
			}
		}
		return;
	}
	
	ASpawnPoint* SpawnP = PickRandomSpawn();
	if (!SpawnP ||!Entry.EnemyClass) {
		RemainingOfCurrentType = 0;
		return;
	}

	FVector Location = SpawnP->GetActorLocation();
	FRotator Rotation = FRotator::ZeroRotator;
	
	AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Entry.EnemyClass, Location, Rotation);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->IncreaseDifficulty(Entry.HealthMultiplier, Entry.DamageMultiplier);
		EnemiesAlive++;
		SpawnedEnemy->OnDestroyed.AddDynamic(this, &AHordManager::OnEnemyDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnActor failed for class %s"),
			   Entry.EnemyClass ? *Entry.EnemyClass->GetName() : TEXT("None"));	}
	RemainingOfCurrentType--;
}

ASpawnPoint* AHordManager::PickRandomSpawn() const
{
	if (ActiveSpawnPoints.Num() == 0) return nullptr;

	int32 Weigth = 0;
	for (ASpawnPoint* sp : ActiveSpawnPoints)
	{
		Weigth += FMath::Max(1, sp->Weight);
	}
	
	int32 Pick = FMath::RandRange(1, Weigth);
	int32 Accum = 0;
	
	for (ASpawnPoint* SP : ActiveSpawnPoints)
	{
		Accum += FMath::Max(1, SP->Weight);
		if (Pick <= Accum)
		{
			return SP;
		}
	}
	return ActiveSpawnPoints.Last();
}